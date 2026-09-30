// ===== ME 222 Lab 2 - PART 2: PWM sweep and dead zone =====
// Car on the stand, USB Serial Monitor, 115200 baud, Newline.
//
// Commands:  s<m> = PWM sweep of motor m (0=FL 1=FR 2=RL 3=RR), CSV output
//            f<m> = dead zone of motor m (PWM_start, PWM_stop)
//            c = counts   z = reset   m<L>,<R> = drive   x = STOP (also during a sweep)
#include <Arduino.h>

// ---------- CONFIGURATION (edit these) ----------
const int NUM_MOTORS = 4;
const int EN_PIN[NUM_MOTORS]  = {14, 27, 13, 21};
const int IN1_PIN[NUM_MOTORS] = {26, 25, 19, 18};
const int IN2_PIN[NUM_MOTORS] = {33, 32, 23, 5};
const int ENC_A[NUM_MOTORS]   = {34, 35, 36, 39};
const int ENC_B[NUM_MOTORS]   = {4, 16, 17, 15};
float CPR[NUM_MOTORS] = {700, 700, 700, 700};       // <-- measured CPR (Part 1)
const float WHEEL_D = 0.065;                        // <-- wheel diameter in metres
const int PWM_FREQ = 20000, PWM_RES = 8;

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


// ---------- ENCODERS ----------
volatile long counts[NUM_MOTORS] = {0, 0, 0, 0};
void IRAM_ATTR encISR(void* arg) {
  int m = (int)(intptr_t)arg;
  if (digitalRead(ENC_B[m])) counts[m]--; else counts[m]++;
}
long readCount(int m) { noInterrupts(); long c = counts[m]; interrupts(); return c; }
void resetCounts() {
  noInterrupts(); for (int i = 0; i < NUM_MOTORS; i++) counts[i] = 0; interrupts();
}

// ---------- MOTORS ----------
void setMotor(int m, int pwm) {
  pwm = constrain(pwm, -255, 255);
  digitalWrite(IN1_PIN[m], pwm > 0);
  digitalWrite(IN2_PIN[m], pwm < 0);
  pwmWrite(EN_PIN[m], m, abs(pwm));
}
void stopAll() { for (int i = 0; i < NUM_MOTORS; i++) setMotor(i, 0); }

// ---------- STOP DURING LONG TESTS ----------
bool stopReq = false;
void pollStop() {                                   // 'x' typed during a test
  while (Serial.available()) {
    if (Serial.read() == 'x') { stopReq = true; stopAll(); }
  }
}
bool waitMs(unsigned long ms) {                     // false if stopped
  unsigned long t0 = millis();
  while (millis() - t0 < ms) { pollStop(); if (stopReq) return false; delay(2); }
  return true;
}

// ---------- MEASUREMENT ----------
float measureRPM(int m, unsigned long windowMs) {
  long c0 = readCount(m); unsigned long t0 = millis();
  waitMs(windowMs);
  long c1 = readCount(m); float dt = (millis() - t0) / 1000.0;
  if (dt <= 0) return 0;
  return ((c1 - c0) / CPR[m]) * (60.0 / dt);
}
float rpmToMps(float rpm) { return rpm * PI * WHEEL_D / 60.0; }

// ---------- SWEEP: 0 -> 255 -> 0 in steps of 15 ----------
void sweep(int m) {
  if (m < 0 || m >= NUM_MOTORS) { Serial.println("bad motor"); return; }
  Serial.println("motor,dir,pwm,rpm,speed_mps,counts");
  for (int dir = 0; dir < 2 && !stopReq; dir++) {
    for (int k = 0; k <= 17; k++) {
      int pwm = (dir == 0) ? k * 15 : 255 - k * 15;
      setMotor(m, pwm);
      if (!waitMs(500)) break;                      // settle 0.5 s
      float rpm = measureRPM(m, 1500);              // average over 1.5 s
      if (stopReq) break;
      Serial.printf("%d,%s,%d,%.1f,%.3f,%ld\n", m, dir ? "down" : "up",
                    pwm, rpm, rpmToMps(rpm), readCount(m));
    }
  }
  stopAll();
  Serial.println(stopReq ? "sweep ABORTED" : "sweep done");
}

// ---------- DEAD ZONE ----------
void deadZone(int m) {
  if (m < 0 || m >= NUM_MOTORS) { Serial.println("bad motor"); return; }
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
  Serial.printf("motor %d: PWM_start=%d PWM_stop=%d\n", m, start, stopP);
}

// ---------- COMMANDS ----------
void handle(String cmd) {
  char c = cmd.charAt(0);
  int arg = cmd.substring(1).toInt();
  switch (c) {
    case 'c': Serial.printf("counts: %ld %ld %ld %ld\n", readCount(0), readCount(1),
                            readCount(2), readCount(3)); break;
    case 'z': resetCounts(); Serial.println("counts reset"); break;
    case 'm': {
      int k = cmd.indexOf(',');
      if (k < 0) { Serial.println("use m<left>,<right> with numbers, e.g. m100,100"); break; }
      int L = cmd.substring(1, k).toInt(), R = cmd.substring(k + 1).toInt();
      setMotor(0, L); setMotor(2, L); setMotor(1, R); setMotor(3, R);
      Serial.printf("drive L=%d R=%d\n", L, R); break;
    }
    case 's': sweep(arg); break;
    case 'f': deadZone(arg); break;
    case 'x': stopAll(); Serial.println("STOP"); break;
    default:  Serial.printf("unknown: %s\n", cmd.c_str());
  }
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_MOTORS; i++) {
    pinMode(IN1_PIN[i], OUTPUT); pinMode(IN2_PIN[i], OUTPUT);
    pwmAttach(EN_PIN[i], i);
    pinMode(ENC_A[i], INPUT); pinMode(ENC_B[i], INPUT);
    attachInterruptArg(digitalPinToInterrupt(ENC_A[i]), encISR,
                       (void*)(intptr_t)i, RISING);
  }
  stopAll();
  Serial.println("Part 2 - sweep. Commands: s<m> f<m> c z m<L>,<R> x");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n'); cmd.trim();
    if (cmd.length()) { stopReq = false; handle(cmd); }
  }
}
