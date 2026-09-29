// ===== ME 222 Lab 2 - PART 1: Encoder test =====
// Goal: check that every encoder senses its own motor, in the right direction,
// and measure counts per revolution (CPR). USB Serial Monitor, 115200 baud, Newline.
//
// Commands:  c = print counts      z = reset counts
//            p = live view on/off (counts + RPM of each wheel every 250 ms)
//            m<L>,<R> = drive left/right side, e.g. m100,100     x = STOP
#include <Arduino.h>

// ---------- CONFIGURATION (edit these) ----------
const int NUM_MOTORS = 4;                           // 0=FL, 1=FR, 2=RL, 3=RR
const int EN_PIN[NUM_MOTORS]  = {14, 27, 13, 21};   // PWM enable pins
const int IN1_PIN[NUM_MOTORS] = {26, 25, 19, 18};
const int IN2_PIN[NUM_MOTORS] = {33, 32, 23, 5};
const int ENC_A[NUM_MOTORS]   = {34, 35, 36, 39};   // input-only: external pull-ups
const int ENC_B[NUM_MOTORS]   = {4, 16, 17, 15};
float CPR[NUM_MOTORS] = {700, 700, 700, 700};       // <-- replace with measured CPR
const int PWM_FREQ = 20000, PWM_RES = 8;            // 20 kHz, 0-255

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

// ---------- LIVE VIEW ----------
bool live = false;
unsigned long tLast = 0;
long cLast[NUM_MOTORS] = {0, 0, 0, 0};

void liveView() {
  unsigned long now = millis();
  if (now - tLast < 250) return;
  float dt = (now - tLast) / 1000.0;
  tLast = now;
  Serial.print("counts:");
  for (int i = 0; i < NUM_MOTORS; i++) Serial.printf(" %7ld", readCount(i));
  Serial.print("   rpm:");
  for (int i = 0; i < NUM_MOTORS; i++) {
    long c = readCount(i);
    Serial.printf(" %6.1f", ((c - cLast[i]) / CPR[i]) * (60.0 / dt));
    cLast[i] = c;
  }
  Serial.println();
}

// ---------- COMMANDS ----------
void handle(String cmd) {
  char c = cmd.charAt(0);
  switch (c) {
    case 'c': Serial.printf("counts: %ld %ld %ld %ld\n", readCount(0), readCount(1),
                            readCount(2), readCount(3)); break;
    case 'z': resetCounts(); for (int i = 0; i < NUM_MOTORS; i++) cLast[i] = 0;
              Serial.println("counts reset"); break;
    case 'p': live = !live; tLast = millis();
              for (int i = 0; i < NUM_MOTORS; i++) cLast[i] = readCount(i);
              Serial.println(live ? "live view ON" : "live view OFF"); break;
    case 'm': {
      int k = cmd.indexOf(',');
      if (k < 0) { Serial.println("use m<L>,<R>"); break; }
      int L = cmd.substring(1, k).toInt(), R = cmd.substring(k + 1).toInt();
      setMotor(0, L); setMotor(2, L); setMotor(1, R); setMotor(3, R);
      Serial.printf("drive L=%d R=%d\n", L, R); break;
    }
    case 'x': stopAll(); Serial.println("STOP"); break;
    default:  Serial.printf("unknown: %s\n", cmd.c_str());
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
  Serial.println("Part 1 - encoder test. Commands: c z p m<L>,<R> x");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n'); cmd.trim();
    if (cmd.length()) handle(cmd);
  }
  if (live) liveView();
}
