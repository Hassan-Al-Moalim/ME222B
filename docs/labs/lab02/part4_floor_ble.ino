// ===== ME 222 Lab 2 - PART 4: Complete sketch (BLE + floor run + sweep + dead zone) =====
// ESP32 + Arduino core 3.x. Commands and data go over BLE (Nordic UART Service).
// USB Serial also works (115200 baud) for debugging.
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
const float WHEEL_D = 0.065;                        // <-- wheel diameter in metres
const int PWM_FREQ = 20000, PWM_RES = 8;            // 20 kHz, 0-255

// ---------- PWM compatibility ----------
// ESP32 Arduino core 3.x addresses LEDC by pin: ledcAttach(pin, freq, res) and
// ledcWrite(pin, duty). Core 2.x addresses it by channel: ledcSetup(ch, ...),
// ledcAttachPin(pin, ch), ledcWrite(ch, duty). Calling the 3.x names on a 2.x
// install fails to compile, so the sketch does not run at all. This picks the
// right pair at compile time; one channel per motor.
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  static inline void pwmAttach(int pin, int ch) { (void)ch; ledcAttach(pin, PWM_FREQ, PWM_RES); }
  static inline void pwmWrite(int pin, int ch, int duty) { (void)ch; ledcWrite(pin, duty); }
#else
  static inline void pwmAttach(int pin, int ch) { ledcSetup(ch, PWM_FREQ, PWM_RES); ledcAttachPin(pin, ch); }
  static inline void pwmWrite(int pin, int ch, int duty) { (void)pin; ledcWrite(ch, duty); }
#endif


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
  pwmWrite(EN_PIN[m], m, abs(pwm));
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

// Wait that can be interrupted by STOP. Returns false if stopped.
bool waitMs(unsigned long ms) {
  unsigned long t0 = millis();
  while (millis() - t0 < ms) { if (stopReq) return false; delay(2); }
  return true;
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

// ---------- MEASUREMENT ----------
float measureRPM(int m, unsigned long windowMs) {  // average RPM over windowMs
  long c0 = readCount(m); unsigned long t0 = millis();
  waitMs(windowMs);
  long c1 = readCount(m); float dt = (millis() - t0) / 1000.0;
  if (dt <= 0) return 0;
  return ((c1 - c0) / CPR[m]) * (60.0 / dt);
}
float rpmToMps(float rpm) { return rpm * PI * WHEEL_D / 60.0; }

// ---------- PART 2: PWM sweep ----------
void sweep(int m) {
  if (m < 0 || m >= NUM_MOTORS) { blePrintf("bad motor\n"); return; }
  blePrintf("motor,dir,pwm,rpm,speed_mps,counts\n");
  for (int dir = 0; dir < 2 && !stopReq; dir++) {
    for (int k = 0; k <= 17; k++) {
      int pwm = (dir == 0) ? k * 15 : 255 - k * 15;
      setMotor(m, pwm);
      if (!waitMs(500)) break;                      // settle
      float rpm = measureRPM(m, 1500);              // average over 1.5 s
      if (stopReq) break;
      blePrintf("%d,%s,%d,%.1f,%.3f,%ld\n", m, dir ? "down" : "up",
           pwm, rpm, rpmToMps(rpm), readCount(m));
    }
  }
  stopAll();
  blePrintf(stopReq ? "sweep ABORTED\n" : "sweep done\n");
}

// ---------- PART 2: dead zone ----------
void deadZone(int m) {
  if (m < 0 || m >= NUM_MOTORS) { blePrintf("bad motor\n"); return; }
  int start = -1, stopP = -1;
  for (int pwm = 0; pwm <= 200 && !stopReq; pwm += 2) {      // ramp up
    setMotor(m, pwm); waitMs(200);
    if (measureRPM(m, 100) > 5) { start = pwm; break; }
  }
  if (!stopReq) { setMotor(m, 150); waitMs(800); }
  for (int pwm = 150; pwm >= 0 && !stopReq; pwm -= 2) {      // ramp down
    setMotor(m, pwm); waitMs(200);
    if (measureRPM(m, 100) < 5) { stopP = pwm; break; }
  }
  stopAll();
  blePrintf("motor %d: PWM_start=%d PWM_stop=%d\n", m, start, stopP);
}

// ---------- PART 4: floor run (3 s, left side PWM L, right side PWM R) ----------
void floorRun(int L, int R) {
  resetCounts();
  blePrintf("t_ms,rpm0,rpm1,rpm2,rpm3\n");
  long prev[NUM_MOTORS] = {0, 0, 0, 0};
  setMotor(0, L); setMotor(2, L); setMotor(1, R); setMotor(3, R);
  unsigned long t0 = millis(), tPrev = t0;
  while (millis() - t0 < 3000) {
    if (!waitMs(100)) break;
    unsigned long now = millis(); float dt = (now - tPrev) / 1000.0; tPrev = now;
    float rpm[NUM_MOTORS];
    for (int i = 0; i < NUM_MOTORS; i++) {
      long c = readCount(i);
      rpm[i] = ((c - prev[i]) / CPR[i]) * (60.0 / dt);
      prev[i] = c;
    }
    blePrintf("%lu,%.1f,%.1f,%.1f,%.1f\n", now - t0, rpm[0], rpm[1], rpm[2], rpm[3]);
  }
  stopAll();
  for (int i = 0; i < NUM_MOTORS; i++)
    blePrintf("motor %d distance = %.3f m\n", i, readCount(i) / CPR[i] * PI * WHEEL_D);
}

// ---------- COMMANDS ----------
//  c          print encoder counts        z        reset counts
//  m<L>,<R>   manual drive, e.g. m120,-80 (left side, right side; -255..255)
//  s<m>       PWM sweep on motor m        f<m>     dead zone of motor m
//  r<pwm>     3 s floor run, same PWM     r<L>,<R> floor run, corrected PWMs
//  x          STOP (always works)
void handle(String cmd) {
  char c = cmd.charAt(0);
  int arg = cmd.substring(1).toInt();
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
    case 's': sweep(arg); break;
    case 'f': deadZone(arg); break;
    case 'r': {                                     // r150 or r150,138
      int k = cmd.indexOf(',');
      int L = (k < 0) ? arg : cmd.substring(1, k).toInt();
      int R = (k < 0) ? arg : cmd.substring(k + 1).toInt();
      floorRun(L, R); break;
    }
    case 'x': stopAll(); blePrintf("STOP\n"); break;
    default:  blePrintf("unknown: %s\n", cmd.c_str());
  }
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_MOTORS; i++) {
    pinMode(IN1_PIN[i], OUTPUT); pinMode(IN2_PIN[i], OUTPUT);
    pwmAttach(EN_PIN[i], i);
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
