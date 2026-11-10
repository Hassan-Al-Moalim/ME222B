/*
  ME222 Lab 7 - Step 4: drive with the lidar

  One ESP32 does everything: lidar, gyro, motors.

  What the car does:
    1. Press START. The car stands still for 1 s to calibrate the gyro.
    2. It drives forward and holds its heading with the gyro (Lab 4).
    3. When something is in its path, it turns 45 degrees away from it
       (to the open side) and keeps driving.
    4. When the lidar shows the original direction is clear again, it turns
       back and carries on, parallel to where it started.
    5. If there is no way through, it stops and waits (BLOCKED).
    6. It stops by itself after RUN_TIME_MS.

  START / STOP: press the BOOT button on the ESP32, or send g / s in the
  Serial Monitor.

  You change only sections 1 and 2. Serial Monitor: 115200 baud.
*/

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// ===================== 1. YOUR CAR (copy from earlier steps) =====================

// Motors: Lab 3 wiring, one PWM and one DIR pin per side
const int L_PWM = 25, L_DIR = 26;
const int R_PWM = 27, R_DIR = 14;
const int L_POLARITY = 1, R_POLARITY = 1;   // -1 reverses a whole side

// IMU
const int I2C_SDA = 21, I2C_SCL = 22;
const float YAW_SIGN = 1.0f;                // -1 if turning the car left makes heading negative

// Lidar: the values you found in Step 3
const int LIDAR_RX_PIN = 16, LIDAR_TX_PIN = 17, LIDAR_PWM_PIN = 4;
const int LIDAR_MOTOR_DUTY = 255;
const float ANGLE_OFFSET_DEG = 0.0f;
const bool  ANGLE_REVERSED   = false;
const uint16_t D_MIN_MM = 150;
const float CAR_WIDTH_MM = 200;             // your car's width (Step 3)

const int BUTTON_PIN = 0;                   // the BOOT button


// ===================== 2. TUNING =====================

const int   P_DRIVE = 120;                  // forward PWM, 0-255
const int   P_TURN  = 150;                  // PWM while turning on the spot
const float SIDE_MARGIN_MM  = 50;           // extra room on each side
const uint16_t D_AVOID_MM   = 600;          // start avoiding when something in the path is this close
const uint16_t D_STOP_MM    = 300;          // stop at once when something in the path is this close
const float TURN_DEG        = 45;           // how far to turn away from an obstacle
const int   N_CONFIRM       = 2;            // full lidar turns in a row before acting
const float KH = 60, KR = 15, U_MAX = 40;   // heading hold from Lab 4 (PWM/rad, PWM/(rad/s), PWM)
const uint32_t RUN_TIME_MS  = 20000;        // the run ends by itself after this

// Car states (used by section 7)
enum State { READY, CALIBRATING, DRIVING, TURNING, BLOCKED, FINISHED, STOPPED };
const char *STATE_NAME[] = {"READY", "CALIBRATING", "DRIVING", "TURNING", "BLOCKED", "FINISHED", "STOPPED"};

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

// ===================== 4. WHAT IS IN THE PATH (given) =====================

const uint16_t NO_HIT = 65535;
const float HALF_PATH_MM = CAR_WIDTH_MM / 2 + SIDE_MARGIN_MM;
float cosTable[360], sinTable[360];

// Distance to the nearest thing inside a lane as wide as the car (plus
// margins) that points toward bearing dirDeg. NO_HIT if the lane is empty.
// Bearings: 0 = straight ahead, 90 = right, 270 = left.
uint16_t pathDistance(float dirDeg) {
  int c = ((int)lroundf(dirDeg) % 360 + 360) % 360;
  uint16_t best = NO_HIT;
  for (int b = 0; b < 360; ++b) {
    if (!scan[b]) continue;
    int rel = (b - c + 360) % 360;
    float along = scan[b] * cosTable[rel];
    float side  = scan[b] * sinTable[rel];
    if (along > 0 && fabsf(side) < HALF_PATH_MM && along < best) best = (uint16_t)along;
  }
  return best;
}

// Where the obstacle straight ahead sits: average sideways offset (mm) of
// the points in the lane ahead that are closer than D_AVOID_MM.
// Positive = more of it on the right, negative = more on the left.
float obstacleSide() {
  float sum = 0; int n = 0;
  for (int b = 0; b < 360; ++b) {
    if (!scan[b]) continue;
    float along = scan[b] * cosTable[b], side = scan[b] * sinTable[b];
    if (along > 0 && along < D_AVOID_MM && fabsf(side) < HALF_PATH_MM) { sum += side; ++n; }
  }
  return n ? sum / n : 0;
}

// Send d in the Serial Monitor to print the last full scan.
void dumpScan() {
  Serial.println("bearing_deg,distance_mm");
  for (int b = 0; b < 360; ++b) if (scan[b]) Serial.printf("%d,%u\n", b, (unsigned)scan[b]);
  Serial.println("end");
}

// ===================== 5. GYRO (MPU6050, same as Lab 3) =====================

uint8_t imuAddr = 0;

bool imuRead(uint8_t reg, uint8_t *d, uint8_t n) {
  Wire.beginTransmission(imuAddr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(imuAddr, n) != n) return false;
  for (uint8_t i = 0; i < n; ++i) d[i] = Wire.read();
  return true;
}

bool imuWrite(uint8_t reg, uint8_t v) {
  Wire.beginTransmission(imuAddr);
  Wire.write(reg); Wire.write(v);
  return Wire.endTransmission() == 0;
}

bool imuBegin() {
  for (uint8_t a = 0x68; a <= 0x69; ++a) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) { imuAddr = a; break; }
  }
  if (!imuAddr) return false;
  uint8_t who = 0;
  if (!imuRead(0x75, &who, 1) || (who != 0x68 && who != 0x70)) return false;
  imuWrite(0x6B, 0x80); delay(100);              // reset
  bool ok = imuWrite(0x6B, 0x01);                // wake, PLL clock
  ok &= imuWrite(0x1A, 0x03);                    // gyro low-pass filter
  ok &= imuWrite(0x1B, 0x00);                    // +/-250 deg/s
  delay(50);
  return ok;
}

// Yaw rate in rad/s, positive = turning left. Returns false on a read error.
bool readYawRate(float &w) {
  uint8_t d[2];
  if (!imuRead(0x47, d, 2)) return false;
  int16_t raw = (int16_t)((d[0] << 8) | d[1]);
  w = YAW_SIGN * raw / 131.0f * (PI / 180.0f);
  return true;
}

// ===================== 6. MOTORS =====================

int leftOut = 0, rightOut = 0;          // last command, -255..255
int leftDirNow = 0, rightDirNow = 0;    // direction pins as set
uint32_t dirChangedMs = 0;

// left, right: -255 (full reverse) to +255 (full forward)
void setMotors(int left, int right) {
  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);
  int ld = left > 0 ? 1 : (left < 0 ? -1 : leftDirNow);
  int rd = right > 0 ? 1 : (right < 0 ? -1 : rightDirNow);
  if (ld != leftDirNow || rd != rightDirNow) {     // short pause before reversing
    analogWrite(L_PWM, 0); analogWrite(R_PWM, 0);
    digitalWrite(L_DIR, ld * L_POLARITY > 0 ? HIGH : LOW);
    digitalWrite(R_DIR, rd * R_POLARITY > 0 ? HIGH : LOW);
    leftDirNow = ld; rightDirNow = rd;
    dirChangedMs = millis();
  }
  if (millis() - dirChangedMs < 20) { left = 0; right = 0; }
  analogWrite(L_PWM, abs(left));
  analogWrite(R_PWM, abs(right));
  leftOut = left; rightOut = right;
}

void stopMotors() { setMotors(0, 0); }

// ===================== 7. MISSION (given) =====================


State state = READY;
const char *reason = "";
float heading = 0;          // degrees since START, positive = left
float target = 0;           // heading the car is trying to hold, degrees
float yawRate = 0;          // rad/s
float gyroBias = 0;
uint32_t runStartMs = 0, stateMs = 0, lastScanMs = 0, lastControlUs = 0;
int aheadCount = 0, homeCount = 0, clearCount = 0;
uint16_t ahead = NO_HIT, leftPath = NO_HIT, rightPath = NO_HIT, homePath = NO_HIT;
bool imuOk = false;

float calSum = 0, calMin = 0, calMax = 0;
int calN = 0;

bool running() { return state == CALIBRATING || state == DRIVING || state == TURNING || state == BLOCKED; }

void setState(State s, const char *why) {
  state = s; reason = why; stateMs = millis();
  aheadCount = homeCount = clearCount = 0;
  if (s != DRIVING && s != TURNING) stopMotors();
  Serial.printf("[%s] %s\n", STATE_NAME[s], why);
}

void startRun() {
  if (running()) return;
  if (!imuOk) { Serial.println("cannot start: no IMU"); return; }
  calSum = 0; calN = 0;
  setState(CALIBRATING, "keep the car still");
}

void stopRun(const char *why) {
  if (running()) setState(STOPPED, why);
}

// Turn toward one side, or stop if neither side is open.
void chooseSide() {
  bool leftOpen  = leftPath  >= D_AVOID_MM;
  bool rightOpen = rightPath >= D_AVOID_MM;
  if (!leftOpen && !rightOpen) { setState(BLOCKED, "no way through"); return; }
  bool goLeft;
  if (leftOpen && rightOpen) goLeft = obstacleSide() >= 0;   // turn away from the obstacle
  else goLeft = leftOpen;                                    // only one side is open
  float step = goLeft ? TURN_DEG : -TURN_DEG;                // left is positive
  if (fabsf(target + step) > 90.0f) { setState(BLOCKED, "would turn more than 90 deg"); return; }
  target += step;
  setState(TURNING, step > 0 ? "obstacle: turning left" : "obstacle: turning right");
}

// Runs once per full lidar turn.
void processScan() {
  lastScanMs = millis();
  ahead     = pathDistance(0);
  leftPath  = pathDistance(360 - TURN_DEG);
  rightPath = pathDistance(TURN_DEG);
  homePath  = pathDistance(heading);      // the starting direction, seen from the car

  if (state == DRIVING) {
    if (ahead < D_STOP_MM) { setState(BLOCKED, "too close"); return; }
    aheadCount = (ahead < D_AVOID_MM) ? aheadCount + 1 : 0;
    if (aheadCount >= N_CONFIRM) { chooseSide(); return; }
    if (target != 0) {                      // driving at an angle: is the way back open?
      homeCount = (homePath >= D_AVOID_MM) ? homeCount + 1 : 0;
      if (homeCount >= N_CONFIRM) { target = 0; setState(TURNING, "path clear: turning back"); }
    }
  } else if (state == BLOCKED) {
    clearCount = (ahead >= D_AVOID_MM) ? clearCount + 1 : 0;
    if (clearCount >= N_CONFIRM) { setState(DRIVING, "path clear"); return; }
    if (millis() - stateMs > 1000 && ahead >= D_STOP_MM) chooseSide();
  }
}

void control(float dt) {
  float w;
  if (!readYawRate(w)) { stopRun("IMU read failed"); return; }

  if (state == CALIBRATING) {
    if (calN == 0) { calMin = calMax = w; }
    calSum += w; ++calN;
    calMin = min(calMin, w); calMax = max(calMax, w);
    if (calN >= 100) {                                // 1 s of samples
      if (calMax - calMin > 0.05f) { calSum = 0; calN = 0; Serial.println("car moved: calibrating again"); return; }
      gyroBias = calSum / calN;
      heading = 0; target = 0;
      runStartMs = millis();
      setState(DRIVING, "go");
    }
    return;
  }

  yawRate = w - gyroBias;
  heading += yawRate * dt * (180.0f / PI);
  if (!running()) return;

  if (millis() - runStartMs > RUN_TIME_MS) { setState(FINISHED, "run time over"); return; }
  if (millis() - lastScanMs > 600)         { setState(STOPPED, "no lidar data"); return; }

  float e = (heading - target) * (PI / 180.0f);       // heading error, rad
  if (state == DRIVING) {
    float u = constrain(KH * e + KR * yawRate, -U_MAX, U_MAX);
    setMotors((int)(P_DRIVE + u), (int)(P_DRIVE - u));
  } else if (state == TURNING) {
    if (fabsf(heading - target) < 5.0f)    { setState(DRIVING, "turn done"); return; }
    if (millis() - stateMs > 4000)         { setState(STOPPED, "turn took too long"); return; }
    if (heading < target) setMotors(-P_TURN, P_TURN);  // turn left
    else                  setMotors(P_TURN, -P_TURN);  // turn right
  }
}


// ===================== 8. SETUP AND LOOP =====================

void printStatus() {
  static uint32_t last = 0;
  if (millis() - last < 1000) return;
  last = millis();
  Serial.printf("%-11s heading %6.1f target %6.1f | ahead %5u left %5u right %5u (mm, 65535 = clear) | %.1f turns/s\n",
                STATE_NAME[state], heading, target, ahead, leftPath, rightPath, scanHz);
}

void readButton() {
  static bool last = HIGH;
  static uint32_t lastChangeMs = 0;
  bool now = digitalRead(BUTTON_PIN);
  if (now != last && millis() - lastChangeMs > 50) {
    lastChangeMs = millis();
    last = now;
    if (now == LOW) { if (running()) stopRun("button"); else startRun(); }
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nME222 Lab 7 - Step 4: drive with the lidar");

  pinMode(L_PWM, OUTPUT); pinMode(L_DIR, OUTPUT);
  pinMode(R_PWM, OUTPUT); pinMode(R_DIR, OUTPUT);
  digitalWrite(L_DIR, LOW); digitalWrite(R_DIR, LOW);
  stopMotors();
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  for (int i = 0; i < 360; ++i) { cosTable[i] = cosf(i * PI / 180); sinTable[i] = sinf(i * PI / 180); }

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);
  imuOk = imuBegin();
  Serial.println(imuOk ? "IMU ready" : "NO IMU: check SDA 21, SCL 22. The car will not start.");

  pinMode(LIDAR_PWM_PIN, OUTPUT);
  analogWrite(LIDAR_PWM_PIN, LIDAR_MOTOR_DUTY);
  Lidar.setRxBufferSize(8192);
  Lidar.begin(115200, SERIAL_8N1, LIDAR_RX_PIN, LIDAR_TX_PIN);
  delay(1500);
  uint8_t status = 0; uint16_t code = 0;
  if (lidarGetHealth(status, code)) Serial.printf("lidar health %u (0 = good)\n", status);
  else Serial.println("NO ANSWER from the lidar: check the wiring (Step 1)");
  Serial.println(lidarStartScan() ? "lidar scanning" : "lidar scan did NOT start");


  lastScanMs = millis();
  lastControlUs = micros();
  Serial.println("ready: press BOOT (or send g) to start, again (or s) to stop");
}

void loop() {
  lidarPoll();                                   // every pass, never skip it
  if (scanReady) { scanReady = false; processScan(); }

  uint32_t nowUs = micros();
  if (nowUs - lastControlUs >= 10000) {          // control loop every 10 ms
    float dt = (nowUs - lastControlUs) * 1e-6f;
    lastControlUs = nowUs;
    if (imuOk) control(dt);
  }

  readButton();
  if (Serial.available()) {
    int c = Serial.read();
    if (c == 'g' || c == 'G') startRun();
    if (c == 's' || c == 'S') stopRun("serial");
    if (c == 'd' || c == 'D') dumpScan();
  }

  if (!running() && millis() - lastNodeMs > 1000) {   // lidar silent: restart it
    lidarStartScan();
    lastNodeMs = millis();
  }

  printStatus();
}
