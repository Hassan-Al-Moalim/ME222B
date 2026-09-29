// ===== ME 222 Lab 2 - PART 3: BLE wireless link =====
// Same motor/encoder code as Part 1, now controlled over BLE (Nordic UART Service).
// Phone: nRF Toolbox -> UART. USB Serial (115200) still works for debugging.
// Commands: c = counts   z = reset   m<L>,<R> = drive   x = STOP
// FAILSAFE: motors stop when the BLE link is lost.
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdarg.h>

// ---------- CONFIGURATION (edit these) ----------
const char* BLE_NAME = "ME222-T1";                  // <-- your team name
const int NUM_MOTORS = 4;                           // 0=FL, 1=FR, 2=RL, 3=RR
const int EN_PIN[NUM_MOTORS]  = {14, 27, 13, 21};   // PWM enable pins
const int IN1_PIN[NUM_MOTORS] = {26, 25, 19, 18};
const int IN2_PIN[NUM_MOTORS] = {33, 32, 23, 5};
const int ENC_A[NUM_MOTORS]   = {34, 35, 36, 39};   // input-only: external pull-ups
const int ENC_B[NUM_MOTORS]   = {4, 16, 17, 15};
float CPR[NUM_MOTORS] = {700, 700, 700, 700};       // <-- measured CPR (Part 1)
const int PWM_FREQ = 20000, PWM_RES = 8;            // 20 kHz, 0-255

// ---------- BLE UART (Nordic UART Service) ----------
#define NUS_SERVICE "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define NUS_RX      "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"   // phone -> ESP32
#define NUS_TX      "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"   // ESP32 -> phone
BLECharacteristic* txChar = nullptr;
volatile bool bleConnected = false;
volatile bool stopReq = false;          // set by 'x' or by disconnect
char cmdBuf[32];
volatile bool cmdReady = false;

// ---------- ENCODERS ----------
volatile long counts[NUM_MOTORS] = {0, 0, 0, 0};

void IRAM_ATTR encISR(void* arg) {
  int m = (int)(intptr_t)arg;
  // 1x decoding: on rising edge of A, B tells the direction
  if (digitalRead(ENC_B[m])) counts[m]--; else counts[m]++;
}
long readCount(int m) {
  noInterrupts(); long c = counts[m]; interrupts();
  return c;
}
void resetCounts() {
  noInterrupts(); for (int i = 0; i < NUM_MOTORS; i++) counts[i] = 0; interrupts();
}

// ---------- MOTORS ----------
void setMotor(int m, int pwm) {                     // pwm: -255..255
  pwm = constrain(pwm, -255, 255);
  digitalWrite(IN1_PIN[m], pwm > 0);
  digitalWrite(IN2_PIN[m], pwm < 0);
  ledcWrite(EN_PIN[m], abs(pwm));
}
void stopAll() { for (int i = 0; i < NUM_MOTORS; i++) setMotor(i, 0); }

// ---------- OUTPUT: USB Serial + BLE in 20-byte chunks ----------
void blePrintf(const char* fmt, ...) {
  char buf[160];
  va_list args; va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  Serial.print(buf);
  if (bleConnected && txChar) {
    size_t n = strlen(buf);
    for (size_t i = 0; i < n; i += 20) {
      size_t len = (n - i < 20) ? (n - i) : 20;
      txChar->setValue((uint8_t*)buf + i, len);
      txChar->notify();
      delay(4);                                     // do not flood the BLE stack
    }
  }
}

// ---------- BLE CALLBACKS ----------
class ServerCB : public BLEServerCallbacks {
  void onConnect(BLEServer* s) override { bleConnected = true; }
  void onDisconnect(BLEServer* s) override {
    bleConnected = false;
    stopReq = true; stopAll();                      // FAILSAFE: link lost -> stop
    s->startAdvertising();                          // allow reconnection
  }
};
class RxCB : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) override {
    String v = c->getValue();                       // core 3.x returns String
    v.trim();
    if (v.length() == 0) return;
    if (v[0] == 'x') { stopReq = true; stopAll(); } // STOP acts immediately
    strncpy(cmdBuf, v.c_str(), sizeof(cmdBuf) - 1);
    cmdBuf[sizeof(cmdBuf) - 1] = 0;
    cmdReady = true;                                // loop() runs the command
  }
};

void setupBLE() {
  BLEDevice::init(BLE_NAME);
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCB());
  BLEService* svc = server->createService(NUS_SERVICE);
  txChar = svc->createCharacteristic(NUS_TX, BLECharacteristic::PROPERTY_NOTIFY);
  txChar->addDescriptor(new BLE2902());
  BLECharacteristic* rx = svc->createCharacteristic(NUS_RX,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  rx->setCallbacks(new RxCB());
  svc->start();
  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(NUS_SERVICE);
  adv->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.printf("BLE ready: %s\n", BLE_NAME);
}

// ---------- COMMANDS ----------
void handle(String cmd) {
  char c = cmd.charAt(0);
  switch (c) {
    case 'c': blePrintf("counts: %ld %ld %ld %ld\n", readCount(0), readCount(1),
                        readCount(2), readCount(3)); break;
    case 'z': resetCounts(); blePrintf("counts reset\n"); break;
    case 'm': {
      int k = cmd.indexOf(',');
      if (k < 0) { blePrintf("use m<L>,<R>\n"); break; }
      int L = cmd.substring(1, k).toInt(), R = cmd.substring(k + 1).toInt();
      setMotor(0, L); setMotor(2, L); setMotor(1, R); setMotor(3, R);
      blePrintf("drive L=%d R=%d\n", L, R); break;
    }
    case 'x': stopAll(); blePrintf("STOP\n"); break;
    default:  blePrintf("unknown: %s\n", cmd.c_str());
  }
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_MOTORS; i++) {
    pinMode(IN1_PIN[i], OUTPUT); pinMode(IN2_PIN[i], OUTPUT);
    ledcAttach(EN_PIN[i], PWM_FREQ, PWM_RES);
    pinMode(ENC_A[i], INPUT); pinMode(ENC_B[i], INPUT);  // INPUT_PULLUP where supported
    attachInterruptArg(digitalPinToInterrupt(ENC_A[i]), encISR,
                       (void*)(intptr_t)i, RISING);
  }
  stopAll();
  setupBLE();
}

void loop() {
  if (cmdReady) {                                   // command from BLE
    String cmd = String(cmdBuf); cmdReady = false;
    stopReq = false; handle(cmd);
  }
  if (Serial.available()) {                         // command from USB (debug)
    String cmd = Serial.readStringUntil('\n'); cmd.trim();
    if (cmd.length()) { stopReq = false; handle(cmd); }
  }
}
