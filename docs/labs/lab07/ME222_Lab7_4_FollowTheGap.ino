/*
  ME222 Lab 7 - Step 6 (extra): Follow the Gap

  Every lidar turn the car:
    1. reads the distances in front of it, from 90 deg left to 90 deg right
    2. makes every obstacle wider by half the car's width (TODO 1),
       so any gap that is left is one the car fits through
    3. finds the widest gap (TODO 2)
    4. steers toward the middle of that gap (TODO 3)

  The sketch compiles and runs before you do the TODOs: the car then just
  drives straight. Do them one at a time and test after each.

  You copy in sections 1 and 6 from your drive sketch.
  START / STOP: BOOT button, or g / s in the Serial Monitor (115200 baud).
  t = motor test, d = print the whole scan.
*/

#include <Arduino.h>
#include <math.h>

// ===================== 1. YOUR CAR (copy from your drive sketch) =====================

const int LIDAR_RX_PIN = 16, LIDAR_TX_PIN = 17, LIDAR_PWM_PIN = 4;
const int LIDAR_MOTOR_DUTY = 255;
const float ANGLE_OFFSET_DEG = 0.0f;
const bool  ANGLE_REVERSED   = false;
const uint16_t D_MIN_MM = 150;
const float CAR_WIDTH_MM = 200;

const int BUTTON_PIN = 0;                   // the BOOT button

// ===================== 2. TUNING =====================

const int   P_DRIVE = 110;                  // forward PWM, 0..255
const float K_STEER = 1.5f;                 // PWM difference per degree the gap is off-centre
const int   STEER_MAX = 90;                 // largest PWM difference
const uint16_t MAX_RANGE_MM = 1500;         // anything farther counts as open
const uint16_t OPEN_MM      = 600;          // a direction is open if nothing is closer than this
const float SAFETY_MM       = 60;           // extra space each side of the car
const uint16_t D_STOP_MM    = 200;          // something this close straight ahead: stop
const uint32_t RUN_TIME_MS  = 60000;

// ===================== 3. LIDAR DRIVER (given, do not change) =====================

HardwareSerial Lidar(2);

// One full turn, already converted to car bearings.
// scan[b] = closest distance in mm seen at bearing b degrees, or 0 if there was
// no valid reading at that bearing in this turn.
//   b = 0 front, 90 right, 180 back, 270 left (clockwise seen from above)
uint16_t scan[360];

bool     scanReady    = false;           // true once per turn, when scan[] has just been refreshed
uint32_t scanCount    = 0;               // number of full turns so far
float    scanHz       = 0.0f;            // turns per second, from the last turn
int      scanPoints   = 0;               // measurements received in the last turn (valid or not)
int      scanBins     = 0;               // bearings in scan[] that hold a valid distance
uint32_t syncErrors   = 0;               // bytes thrown away while looking for a valid measurement

static uint16_t work[360];               // the turn being collected
static int      workPoints = 0;
static bool     haveTurn   = false;
static uint8_t  node[5];
static uint8_t  nodeFill   = 0;
static uint32_t lastNodeMs = 0;
static uint32_t lastTurnMs = 0;

const uint8_t CMD_STOP = 0x25, CMD_RESET = 0x40, CMD_SCAN = 0x20;
const uint8_t CMD_GET_INFO = 0x50, CMD_GET_HEALTH = 0x52;

void lidarSend(uint8_t cmd) {
  uint8_t packet[2] = {0xA5, cmd};
  Lidar.write(packet, 2);
}

void lidarFlush() {
  while (Lidar.available()) Lidar.read();
}

bool lidarRead(uint8_t *buf, size_t n, uint32_t timeoutMs) {
  uint32_t t0 = millis();
  size_t got = 0;
  while (got < n) {
    if (Lidar.available()) buf[got++] = (uint8_t)Lidar.read();
    else if (millis() - t0 > timeoutMs) return false;
  }
  return true;
}

// Every reply starts with a 7-byte descriptor: A5 5A, four length/mode bytes,
// and one data-type byte. Skip anything before it.
bool lidarDescriptor(uint8_t dataType, uint32_t timeoutMs) {
  uint32_t t0 = millis();
  uint8_t prev = 0;
  while (millis() - t0 < timeoutMs) {
    if (!Lidar.available()) continue;
    uint8_t b = (uint8_t)Lidar.read();
    if (prev == 0xA5 && b == 0x5A) {
      uint8_t rest[5];
      if (!lidarRead(rest, 5, timeoutMs)) return false;
      return rest[4] == dataType;
    }
    prev = b;
  }
  return false;
}

// status: 0 good, 1 warning, 2 error. Returns false if the lidar did not answer.
bool lidarGetHealth(uint8_t &status, uint16_t &errorCode) {
  lidarFlush();
  lidarSend(CMD_GET_HEALTH);
  uint8_t d[3];
  if (!lidarDescriptor(0x06, 500) || !lidarRead(d, 3, 500)) return false;
  status = d[0];
  errorCode = (uint16_t)(d[1] | (d[2] << 8));
  return true;
}

bool lidarGetInfo(uint8_t &model, uint8_t &fwMajor, uint8_t &fwMinor, uint8_t &hardware) {
  lidarFlush();
  lidarSend(CMD_GET_INFO);
  uint8_t d[20];
  if (!lidarDescriptor(0x04, 500) || !lidarRead(d, 20, 500)) return false;
  model = d[0]; fwMinor = d[1]; fwMajor = d[2]; hardware = d[3];
  return true;
}

bool lidarStartScan() {
  lidarSend(CMD_STOP);
  delay(20);
  lidarFlush();
  nodeFill = 0;
  workPoints = 0;
  haveTurn = false;
  memset(work, 0, sizeof(work));
  lidarSend(CMD_SCAN);
  if (!lidarDescriptor(0x81, 1000)) return false;
  lastNodeMs = millis();
  return true;
}

static void finishTurn() {
  uint32_t now = millis();
  if (haveTurn) {
    memcpy(scan, work, sizeof(scan));
    scanPoints = workPoints;
    scanBins = 0;
    for (int i = 0; i < 360; ++i) if (scan[i]) ++scanBins;
    uint32_t period = now - lastTurnMs;
    scanHz = period ? 1000.0f / (float)period : 0.0f;
    ++scanCount;
    scanReady = true;
  }
  haveTurn = true;
  lastTurnMs = now;
  workPoints = 0;
  memset(work, 0, sizeof(work));
}

// One measurement is 5 bytes:
//   byte 0: quality (6 bits) | not-S | S        S = 1 on the first point of a new turn
//   byte 1: angle low 7 bits | C                C is always 1
//   byte 2: angle high 8 bits                   angle = value / 64 degrees
//   byte 3, 4: distance, low byte first         distance = value / 4 mm, 0 = no echo
static void handleNode() {
  bool    startOfTurn = node[0] & 0x01;
  uint8_t quality     = node[0] >> 2;
  float   rawDeg      = (float)((node[1] >> 1) | (node[2] << 7)) / 64.0f;
  float   distMm      = (float)(node[3] | (node[4] << 8)) / 4.0f;

  lastNodeMs = millis();
  if (startOfTurn) finishTurn();
  ++workPoints;
  if (quality == 0 || distMm < (float)D_MIN_MM || distMm > 65000.0f) return;

  // raw lidar angle -> car bearing
  float bearing = rawDeg - ANGLE_OFFSET_DEG;
  if (ANGLE_REVERSED) bearing = -bearing;
  while (bearing < 0.0f)    bearing += 360.0f;
  while (bearing >= 360.0f) bearing -= 360.0f;
  int i = (int)(bearing + 0.5f) % 360;

  uint16_t d = (uint16_t)(distMm + 0.5f);
  if (work[i] == 0 || d < work[i]) work[i] = d;      // keep the closest in each degree
}

// Call this on every pass of loop(). It never waits.
void lidarPoll() {
  while (Lidar.available()) {
    node[nodeFill++] = (uint8_t)Lidar.read();
    if (nodeFill == 1) {
      bool s = node[0] & 0x01, notS = node[0] & 0x02;
      if (s == notS) { nodeFill = 0; ++syncErrors; }           // S and not-S must differ
    } else if (nodeFill == 2) {
      if (!(node[1] & 0x01)) { nodeFill = 0; ++syncErrors; }   // C must be 1
    } else if (nodeFill == 5) {
      nodeFill = 0;
      handleNode();
    }
  }
}

// ===================== 6. MOTORS (fill in from your Lab 1 car) =====================
// Your motor pins. Do not use GPIO 0, 4, 16, 17, 21, 22 (BOOT, lidar, IMU).

// LEFT driver
#define L_FWD_IN1   ___
#define L_FWD_IN2   ___
#define L_FWD_PWM   ___
#define L_BWD_IN1   ___
#define L_BWD_IN2   ___
#define L_BWD_PWM   ___

// RIGHT driver
#define R_FWD_IN1   ___
#define R_FWD_IN2   ___
#define R_FWD_PWM   ___
#define R_BWD_IN1   ___
#define R_BWD_IN2   ___
#define R_BWD_PWM   ___

// Runs one motor. speed: -255 (full reverse) to +255 (full forward). Given.
void motor(int in1, int in2, int pwm, int speed) {
  digitalWrite(in1, speed > 0 ? HIGH : LOW);
  digitalWrite(in2, speed < 0 ? HIGH : LOW);
  analogWrite(pwm, abs(speed));
}

void motorsBegin() {
  // TODO: pinMode(..., OUTPUT) for all 12 pins above
}

int leftOut = 0, rightOut = 0;

void setMotors(int left, int right) {
  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);
  leftOut = left; rightOut = right;

  // TODO: one line per motor. Left motors get left, right motors get right.
  // A wheel turns the wrong way? Swap IN1 and IN2 in its line.
  motor(L_FWD_IN1, L_FWD_IN2, L_FWD_PWM, left);
  // ...
}

void stopMotors() { setMotors(0, 0); }


// ===================== 7. FOLLOW THE GAP =====================

// r[k] is the distance at bearing k - 90 degrees:
//   r[0] = 90 deg left, r[90] = straight ahead, r[180] = 90 deg right
const int N = 181;
float r[N];

bool running = false;
uint32_t runStartMs = 0, lastScanMs = 0;
int gapStart = 90, gapEnd = 90;
float target = 0;                           // bearing of the gap's middle, + = right
const char *status = "READY";

// Step 1 (given): copy the front half of the scan into r[].
// No reading, or farther than MAX_RANGE_MM, counts as open.
void readRanges() {
  for (int k = 0; k < N; ++k) {
    int b = (k - 90 + 360) % 360;
    uint16_t d = scan[b];
    r[k] = (d == 0 || d > MAX_RANGE_MM) ? MAX_RANGE_MM : d;
  }
}

// TODO 1: make every obstacle wider by half the car's width.
//
// A point at distance d blocks the car if it is closer than
// h = CAR_WIDTH_MM / 2 + SAFETY_MM to the car's path. Seen from the lidar,
// that covers an angle of  a = atan(h / d)  on each side of the point.
// For every k, set r[j] = min(r[j], r[k]) for all j from k - a to k + a.
//
// Hints: work on a copy (out[]) so points you just widened do not widen again.
// atan() gives radians; degrees = radians * 180 / PI. Keep j between 0 and N - 1.
void widenObstacles() {
  float out[N];
  for (int k = 0; k < N; ++k) out[k] = r[k];
  // your code here
  for (int k = 0; k < N; ++k) r[k] = out[k];
}

// TODO 2: find the widest gap: the longest run of k next to each other with
// r[k] >= OPEN_MM. Put its first and last k in gapStart and gapEnd.
// Return false if there is no open direction at all.
//
// Hint: walk k from 0 to N - 1, remember where the current run started,
// and when a run ends compare its length with the best one so far.
bool findWidestGap() {
  gapStart = 90; gapEnd = 90;     // replace: for now "the gap" is straight ahead
  return true;
}

// TODO 3: steer toward the middle of the gap.
// target = bearing of the middle of the gap (degrees, + = right, - = left).
// Steering: steer = K_STEER * target, limited to +-STEER_MAX.
// To turn right the left wheels must go faster than the right wheels.
void steer() {
  target = 0;                     // replace
  setMotors(P_DRIVE, P_DRIVE);    // replace
}

// Nearest thing straight ahead (within 10 deg), before widening.
uint16_t aheadMm() {
  uint16_t best = MAX_RANGE_MM;
  for (int b = -10; b <= 10; ++b) {
    uint16_t d = scan[(b + 360) % 360];
    if (d && d < best) best = d;
  }
  return best;
}

void processScan() {
  lastScanMs = millis();
  readRanges();
  uint16_t ahead = aheadMm();
  widenObstacles();
  bool found = findWidestGap();
  if (!running) return;
  if (ahead < D_STOP_MM) { stopMotors(); status = "STOP: too close"; return; }
  if (!found)            { stopMotors(); status = "STOP: no gap"; return; }
  status = "DRIVING";
  steer();
}

void startRun() {
  running = true; runStartMs = millis(); lastScanMs = millis();
  status = "DRIVING";
  Serial.println("go");
}

void stopRun(const char *why) {
  running = false; stopMotors();
  status = "READY";
  Serial.printf("stop: %s\n", why);
}

void dumpScan() {
  Serial.println("bearing_deg,distance_mm");
  for (int b = 0; b < 360; ++b) if (scan[b]) Serial.printf("%d,%u\n", b, (unsigned)scan[b]);
  Serial.println("end");
}

// ===================== 8. SETUP AND LOOP =====================

void motorHold(int left, int right, uint32_t ms) {
  uint32_t t0 = millis();
  while (millis() - t0 < ms) { setMotors(left, right); lidarPoll(); delay(1); }
}

void motorTest() {
  Serial.println("motor test: LEFT wheels forward");
  motorHold(P_DRIVE, 0, 1000);
  Serial.println("motor test: RIGHT wheels forward");
  motorHold(0, P_DRIVE, 1000);
  Serial.println("motor test: turn LEFT on the spot");
  motorHold(-P_DRIVE, P_DRIVE, 1000);
  stopMotors();
  Serial.println("motor test done");
}

void readButton() {
  static bool last = HIGH;
  static uint32_t lastChangeMs = 0;
  bool now = digitalRead(BUTTON_PIN);
  if (now != last && millis() - lastChangeMs > 50) {
    lastChangeMs = millis();
    last = now;
    if (now == LOW) { if (running) stopRun("button"); else startRun(); }
  }
}

void printStatus() {
  static uint32_t last = 0;
  if (millis() - last < 500) return;
  last = millis();
  Serial.printf("%-16s gap %+4d..%+4d deg  target %+6.1f deg  ahead %5u mm  PWM L %4d R %4d  %.1f turns/s\n",
                status, gapStart - 90, gapEnd - 90, target, aheadMm(), leftOut, rightOut, scanHz);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nME222 Lab 7 - Follow the Gap");
  motorsBegin();
  stopMotors();
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(LIDAR_PWM_PIN, OUTPUT);
  analogWrite(LIDAR_PWM_PIN, LIDAR_MOTOR_DUTY);
  Lidar.setRxBufferSize(8192);
  Lidar.begin(115200, SERIAL_8N1, LIDAR_RX_PIN, LIDAR_TX_PIN);
  delay(1500);
  Serial.println(lidarStartScan() ? "lidar scanning" : "lidar scan did NOT start: check the wiring");
  Serial.println("ready: press BOOT (or send g) to start, again (or s) to stop");
}

void loop() {
  lidarPoll();                                   // every pass, never skip it
  if (scanReady) { scanReady = false; processScan(); }

  if (running && millis() - lastScanMs > 600)        stopRun("no lidar data");
  if (running && millis() - runStartMs > RUN_TIME_MS) stopRun("run time over");

  readButton();
  if (Serial.available()) {
    int c = Serial.read();
    if (c == 'g' || c == 'G') startRun();
    if (c == 's' || c == 'S') stopRun("serial");
    if (c == 'd' || c == 'D') dumpScan();
    if ((c == 't' || c == 'T') && !running) motorTest();
  }

  if (!running && millis() - lastNodeMs > 1000) {   // lidar silent: restart it
    lidarStartScan();
    lastNodeMs = millis();
  }
  printStatus();
}
