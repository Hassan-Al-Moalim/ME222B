/*
  Standard ESP32 DevKit (ESP32-WROOM-32) + MPU6050 + nRF Toolbox UART.
  Arduino IDE: ESP32 Dev Module, Espressif ESP32 board package 3.x.
  IMU uses the working direct-register test; no sensor libraries required.
  BLE libraries are included with Espressif's board package.

  PWM/DIR driver required: PWM=0 must stop drive in either direction.
  M1 PWM=GPIO25, DIR=GPIO26; M2 PWM=GPIO27, DIR=GPIO14.
  IMU SDA=GPIO21, SCL=GPIO22; auto-detect address 0x68 or 0x69.
  Accepts WHO_AM_I 0x68 and the observed 0x70.
  Use common GND, appropriate separate motor power, and 10k pulldowns on PWM.
  NOT direct IN1/IN2/PWM wiring for TB6612 or L298.
  These pin assignments are for the original ESP32, not ESP32-C3/S3.

  Buttons: F/B/L/R movement; S stop; 1/2/3 speed; I toggle IMU stream.
  Movement taps run at most 3 seconds. Speed/IMU commands do not renew it.
  IMU reports acceleration m/s^2, gyro rad/s, temperature degrees C.
*/
#include <Arduino.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <atomic>

const int M1_PWM = 25, M1_DIR = 26;
const int M2_PWM = 27, M2_DIR = 14;
const int M1_POLARITY = 1, M2_POLARITY = 1;
const int MPU_SDA = 21, MPU_SCL = 22;
uint8_t imuAddress = 0, imuIdentity = 0;
const uint32_t RUN_TIMEOUT_MS = 3000;
const char *SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
const char *RX_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
const char *TX_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

BLECharacteristic *txChar = nullptr;
BLE2902 *txCCCD = nullptr;
QueueHandle_t commandQueue;
std::atomic<bool> connected(false), stopRequested(false), restartRequested(false);
std::atomic<uint32_t> connectionEpoch(0);
struct MotorCommand { char value; uint32_t epoch; };
bool imuReady = false, streamIMU = false;
int speedPWM = 150, leftDirection = 0, rightDirection = 0;
uint32_t lastMovementMs = 0, lastIMUms = 0;

bool readRegs(uint8_t reg, uint8_t *data, uint8_t count) {
  Wire.beginTransmission(imuAddress);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(imuAddress, count) != count) return false;
  for (uint8_t i = 0; i < count; ++i) data[i] = Wire.read();
  return true;
}

bool writeReg(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(imuAddress);
  Wire.write(reg); Wire.write(value);
  return Wire.endTransmission() == 0;
}

int16_t signed16(const uint8_t *p) {
  return (int16_t)((uint16_t)p[0] << 8 | p[1]);
}

bool initIMU() {
  for (uint8_t candidate = 0x68; candidate <= 0x69; ++candidate) {
    Wire.beginTransmission(candidate);
    if (Wire.endTransmission() == 0) { imuAddress = candidate; break; }
  }
  if (!imuAddress) { Serial.println("No IMU at 0x68/0x69. Check wiring."); return false; }
  if (!readRegs(0x75, &imuIdentity, 1)) { Serial.println("Identity read failed"); return false; }
  Serial.printf("I2C=0x%02X WHO_AM_I=0x%02X\n", imuAddress, imuIdentity);
  if (imuIdentity != 0x68 && imuIdentity != 0x70) {
    Serial.println("Unexpected IMU identity; stopping before configuration.");
    return false;
  }
  if (!writeReg(0x6B, 0x80)) { Serial.println("Reset write failed"); return false; }
  delay(100);
  // Wake; select PLL clock; enable accel/gyro; use +/-2g and +/-250 deg/s.
  bool ok = writeReg(0x6B, 0x01);
  ok &= writeReg(0x6C, 0x00);
  ok &= writeReg(0x1A, 0x03); // Gyro digital low-pass filter.
  ok &= writeReg(0x19, 0x09); // 100 Hz with 1 kHz internal sample rate.
  ok &= writeReg(0x1B, 0x00);
  ok &= writeReg(0x1C, 0x00);
  if (imuIdentity == 0x70) ok &= writeReg(0x1D, 0x03); // MPU6500 accel filter.
  delay(100);
  uint8_t gyroConfig = 0, accelConfig = 0, power = 0;
  ok &= readRegs(0x1B, &gyroConfig, 1);
  ok &= readRegs(0x1C, &accelConfig, 1);
  ok &= readRegs(0x6B, &power, 1);
  bool configured = ok && gyroConfig == 0 && accelConfig == 0 && (power & 0x60) == 0;
  Serial.println(configured ? "IMU ready: use I to stream readings." : "Configuration failed.");
  return configured;
}

void stopMotors() {
  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);
  leftDirection = rightDirection = 0;
}

void sendText(const char *text) {
  Serial.print(text);
  if (!connected.load() || !txCCCD || !txCCCD->getNotifications()) return;
  // Use <=20-byte notifications even with the default BLE MTU of 23.
  size_t len = strlen(text);
  for (size_t offset = 0; offset < len && connected.load(); offset += 20) {
    size_t count = len - offset;
    if (count > 20) count = 20;
    txChar->setValue((uint8_t *)(text + offset), count);
    txChar->notify();
  }
}

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *) override { connected.store(true); }
  void onDisconnect(BLEServer *) override {
    connected.store(false);
    connectionEpoch.fetch_add(1);
    stopRequested.store(true);
    restartRequested.store(true);
  }
};

class RxCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override {
    auto value = characteristic->getValue();
    for (size_t i = 0; i < value.length(); ++i) {
      char c = value[i];
      if (c == '\r' || c == '\n' || c == ' ' || c == '\t') continue;
      if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
      if (c == 'S') {
        // Stop promptly instead of waiting behind queued movement commands.
        xQueueReset(commandQueue);
        stopRequested.store(true);
        continue;
      }
      MotorCommand command = {c, connectionEpoch.load()};
      if (xQueueSend(commandQueue, &command, 0) != pdTRUE) {
        xQueueReset(commandQueue);
        stopRequested.store(true);
        break;
      }
    }
  }
};

void applyMotion(int left, int right) {
  analogWrite(M1_PWM, 0); analogWrite(M2_PWM, 0);
  delay(20); // Off interval before direction changes, not a mechanical brake.
  if (!connected.load() || stopRequested.load()) { stopMotors(); return; }
  digitalWrite(M1_DIR, left * M1_POLARITY > 0 ? HIGH : LOW);
  digitalWrite(M2_DIR, right * M2_POLARITY > 0 ? HIGH : LOW);
  leftDirection = left; rightDirection = right;
  analogWrite(M1_PWM, left ? speedPWM : 0);
  analogWrite(M2_PWM, right ? speedPWM : 0);
}

void handleCommand(char c) {
  switch (c) {
    case 'F': applyMotion( 1,  1); lastMovementMs = millis(); break;
    case 'B': applyMotion(-1, -1); lastMovementMs = millis(); break;
    case 'L': applyMotion(-1,  1); lastMovementMs = millis(); break;
    case 'R': applyMotion( 1, -1); lastMovementMs = millis(); break;
    case '1': case '2': case '3':
      speedPWM = c == '1' ? 90 : (c == '2' ? 150 : 230);
      if (leftDirection || rightDirection) applyMotion(leftDirection, rightDirection);
      break;
    case 'I':
      if (!imuReady) { sendText("ERR: IMU unavailable\n"); return; }
      streamIMU = !streamIMU;
      sendText(streamIMU ? "IMU ON\n" : "IMU OFF\n");
      return;
    default: stopMotors(); sendText("ERR: unknown cmd\n"); return;
  }
  char reply[10];
  snprintf(reply, sizeof(reply), "OK %c\n", c);
  sendText(reply);
}

void sendIMU() {
  uint8_t bytes[14];
  if (!readRegs(0x3B, bytes, sizeof(bytes))) {
    streamIMU = false;
    sendText("ERR: IMU read\n");
    return;
  }
  // Preserve the BLE units of the original sketch.
  const float accelerationScale = 9.80665f / 16384.0f;
  const float gyroScale = (3.14159265359f / 180.0f) / 131.0f;
  float ax = signed16(bytes) * accelerationScale;
  float ay = signed16(bytes + 2) * accelerationScale;
  float az = signed16(bytes + 4) * accelerationScale;
  int16_t rawTemperature = signed16(bytes + 6);
  float gx = signed16(bytes + 8) * gyroScale;
  float gy = signed16(bytes + 10) * gyroScale;
  float gz = signed16(bytes + 12) * gyroScale;
  float temp = imuIdentity == 0x70 ? rawTemperature / 333.87f + 21.0f
                                   : rawTemperature / 340.0f + 36.53f;
  char line[32];
  snprintf(line, sizeof(line), "AX %.2f\n", ax); sendText(line);
  snprintf(line, sizeof(line), "AY %.2f\n", ay); sendText(line);
  snprintf(line, sizeof(line), "AZ %.2f\n", az); sendText(line);
  snprintf(line, sizeof(line), "GX %.2f\n", gx); sendText(line);
  snprintf(line, sizeof(line), "GY %.2f\n", gy); sendText(line);
  snprintf(line, sizeof(line), "GZ %.2f\n", gz); sendText(line);
  snprintf(line, sizeof(line), "TEMP %.2f\n", temp); sendText(line);
}

void setup() {
  pinMode(M1_PWM, OUTPUT); digitalWrite(M1_PWM, LOW);
  pinMode(M2_PWM, OUTPUT); digitalWrite(M2_PWM, LOW);
  pinMode(M1_DIR, OUTPUT); digitalWrite(M1_DIR, LOW);
  pinMode(M2_DIR, OUTPUT); digitalWrite(M2_DIR, LOW);
  // ESP32 Arduino analogWrite defaults to an 8-bit duty value (0..255).
  stopMotors();
  Serial.begin(115200);
  commandQueue = xQueueCreate(32, sizeof(MotorCommand));
  if (!commandQueue) { Serial.println("Command queue failed"); while (true) delay(100); }

  Wire.begin(MPU_SDA, MPU_SCL);
  Wire.setClock(100000);
  Wire.setTimeOut(50);
  imuReady = initIMU();
  if (!imuReady) Serial.println("IMU unavailable; motor buttons remain available");

  BLEDevice::init("ESP32-Motors");
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());
  BLEService *service = server->createService(SERVICE_UUID);
  txChar = service->createCharacteristic(TX_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  txCCCD = new BLE2902();
  txChar->addDescriptor(txCCCD);
  BLECharacteristic *rxChar = service->createCharacteristic(
    RX_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  rxChar->setCallbacks(new RxCallbacks());
  service->start();
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("Connect nRF Toolbox to ESP32-Motors");
}

void loop() {
  if (stopRequested.exchange(false)) {
    stopMotors();
    xQueueReset(commandQueue);
    sendText("STOP\n");
  }
  if (restartRequested.exchange(false)) BLEDevice::startAdvertising();
  if (!connected.load()) {
    stopMotors();
    streamIMU = false;
    xQueueReset(commandQueue);
    delay(5);
    return;
  }
  if ((leftDirection || rightDirection) &&
      (uint32_t)(millis() - lastMovementMs) >= RUN_TIMEOUT_MS) {
    stopMotors();
    sendText("STOP: timeout\n");
  }
  MotorCommand command;
  // One command per loop keeps timeout and disconnect checks responsive.
  if (xQueueReceive(commandQueue, &command, 0) == pdTRUE &&
      command.epoch == connectionEpoch.load() && connected.load() &&
      !stopRequested.load()) handleCommand(command.value);

  if (imuReady && streamIMU && connected.load() &&
      (uint32_t)(millis() - lastIMUms) >= 1000) {
    lastIMUms = millis();
    sendIMU();
  }
  delay(5);
}
