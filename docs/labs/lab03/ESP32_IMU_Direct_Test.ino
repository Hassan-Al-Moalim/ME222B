// ESP32 + MPU6050/MPU6500-compatible register test.
// No sensor library: only Arduino's built-in Wire library.
// SDA=21, SCL=22; Serial Monitor=115200 baud.
// Detects 0x68 or 0x69; supports observed WHO_AM_I 0x68 or 0x70.
#include <Wire.h>

uint8_t address = 0, identity = 0;
bool ready = false;

bool readRegs(uint8_t reg, uint8_t *data, uint8_t count) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(address, count) != count) return false;
  for (uint8_t i = 0; i < count; ++i) data[i] = Wire.read();
  return true;
}

bool writeReg(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg); Wire.write(value);
  return Wire.endTransmission() == 0;
}

int16_t signed16(const uint8_t *p) {
  return (int16_t)((uint16_t)p[0] << 8 | p[1]);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\nDIRECT IMU TEST");
  Wire.begin(21, 22);
  Wire.setClock(100000);
  Wire.setTimeOut(50);
  for (uint8_t candidate = 0x68; candidate <= 0x69; ++candidate) {
    Wire.beginTransmission(candidate);
    if (Wire.endTransmission() == 0) { address = candidate; break; }
  }
  if (!address) { Serial.println("No IMU at 0x68/0x69. Check wiring."); return; }
  if (!readRegs(0x75, &identity, 1)) { Serial.println("Identity read failed"); return; }
  Serial.printf("I2C=0x%02X WHO_AM_I=0x%02X\n", address, identity);
  if (identity != 0x68 && identity != 0x70) {
    Serial.println("Unexpected identity; stopping before configuration.");
    return;
  }
  if (!writeReg(0x6B, 0x80)) { Serial.println("Reset write failed"); return; }
  delay(100);
  // Wake; select PLL clock; enable accel/gyro; use +/-2g and +/-250 deg/s.
  bool ok = writeReg(0x6B, 0x01);
  ok &= writeReg(0x6C, 0x00);
  ok &= writeReg(0x1A, 0x03); // Gyro digital low-pass filter.
  ok &= writeReg(0x19, 0x09); // 100 Hz with 1 kHz internal sample rate.
  ok &= writeReg(0x1B, 0x00);
  ok &= writeReg(0x1C, 0x00);
  if (identity == 0x70) ok &= writeReg(0x1D, 0x03); // MPU6500 accel filter.
  delay(100);
  uint8_t gyroConfig = 0, accelConfig = 0, power = 0;
  ok &= readRegs(0x1B, &gyroConfig, 1);
  ok &= readRegs(0x1C, &accelConfig, 1);
  ok &= readRegs(0x6B, &power, 1);
  ready = ok && gyroConfig == 0 && accelConfig == 0 && (power & 0x60) == 0;
  Serial.println(ready ? "Ready: move and rotate the module." : "Configuration failed.");
}

void loop() {
  if (!ready) { delay(1000); return; }
  uint8_t bytes[14];
  if (!readRegs(0x3B, bytes, sizeof(bytes))) {
    Serial.println("I2C read failed"); delay(500); return;
  }
  float ax = signed16(bytes) / 16384.0f;
  float ay = signed16(bytes + 2) / 16384.0f;
  float az = signed16(bytes + 4) / 16384.0f;
  int16_t rawTemperature = signed16(bytes + 6);
  float gx = signed16(bytes + 8) / 131.0f;
  float gy = signed16(bytes + 10) / 131.0f;
  float gz = signed16(bytes + 12) / 131.0f;
  // Temperature conversion differs for the two reported identities.
  float temp = identity == 0x70 ? rawTemperature / 333.87f + 21.0f
                                : rawTemperature / 340.0f + 36.53f;
  Serial.printf("ACC g: X=%.3f Y=%.3f Z=%.3f\n", ax, ay, az);
  Serial.printf("GYRO deg/s: X=%.2f Y=%.2f Z=%.2f | TEMP=%.2f C\n\n", gx, gy, gz, temp);
  delay(500);
}
