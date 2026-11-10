/*
  ME222 Lab 7 - Steps 2 and 3: lidar test (the car does not move)

  Prints, about once a second:
    - scan rate and the nearest point
    - ahead / left / right: distance to the nearest thing in a lane as wide
      as the car, straight ahead and 45 degrees to each side
    - what the car would do: CLEAR, AVOID LEFT, AVOID RIGHT or BLOCKED

  Send d in the Serial Monitor to print the whole scan (bearing, distance).
  Serial Monitor: 115200 baud. No library needed. Board: ESP32 Dev Module.
*/

#include <Arduino.h>
#include <math.h>

// ===================== 1. SETTINGS =====================

// Lidar wiring (Step 1). No change needed.
const int LIDAR_RX_PIN  = 16;   // lidar TX
const int LIDAR_TX_PIN  = 17;   // lidar RX
const int LIDAR_PWM_PIN = 4;    // lidar MOTOCTL
const int LIDAR_MOTOR_DUTY = 255;

// Step 3: make bearing 0 the front of the car and 90 its right side.
const float ANGLE_OFFSET_DEG = 0.0f;     // the bearing a box straight ahead shows before you change this
const bool  ANGLE_REVERSED   = false;    // true if a box on the right shows near 270
const uint16_t D_MIN_MM = 150;           // ignore anything closer (blind zone, parts of your car)

// The lane the car needs. Use the same values in the drive sketch.
const float CAR_WIDTH_MM   = 200;        // measure your car
const float SIDE_MARGIN_MM = 50;
const uint16_t D_AVOID_MM  = 600;        // something in the lane closer than this = obstacle
const float TURN_DEG       = 45;

const int PRINT_EVERY = 5;               // print every 5th lidar turn

// ===================== 2. LIDAR DRIVER (given, do not change) =====================

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

// ===================== 3. WHAT IS IN THE PATH =====================

const uint16_t NO_HIT = 65535;
const float HALF_PATH_MM = CAR_WIDTH_MM / 2 + SIDE_MARGIN_MM;
float cosTable[360], sinTable[360];

// Distance to the nearest thing inside a lane as wide as the car (plus
// margins) that points toward bearing dirDeg. NO_HIT if the lane is empty.
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

uint16_t nearestPoint(int &bearingDeg) {
  uint16_t best = 0;
  bearingDeg = -1;
  for (int b = 0; b < 360; ++b)
    if (scan[b] && (best == 0 || scan[b] < best)) { best = scan[b]; bearingDeg = b; }
  return best;
}

void printMm(const char *name, uint16_t mm) {
  if (mm == NO_HIT) Serial.printf("%s  clear | ", name);
  else              Serial.printf("%s %5u | ", name, (unsigned)mm);
}

void processScan() {
  if (scanCount % PRINT_EVERY) return;
  int nb;
  uint16_t nearest = nearestPoint(nb);
  uint16_t ahead = pathDistance(0);
  uint16_t left  = pathDistance(360 - TURN_DEG);
  uint16_t right = pathDistance(TURN_DEG);

  const char *would;
  if (ahead >= D_AVOID_MM)                             would = "CLEAR";
  else if (left < D_AVOID_MM && right < D_AVOID_MM)    would = "BLOCKED";
  else if (left >= D_AVOID_MM && right >= D_AVOID_MM)  would = obstacleSide() >= 0 ? "AVOID LEFT" : "AVOID RIGHT";
  else if (left >= D_AVOID_MM)                         would = "AVOID LEFT";
  else                                                 would = "AVOID RIGHT";

  Serial.printf("%.1f turns/s %3d pts | nearest %5u mm at %3d | ", scanHz, scanPoints, (unsigned)nearest, nb);
  printMm("ahead", ahead); printMm("left", left); printMm("right", right);
  Serial.printf("%s | sync errors %lu\n", would, (unsigned long)syncErrors);
}

void dumpScan() {
  Serial.println("bearing_deg,distance_mm");
  for (int b = 0; b < 360; ++b) if (scan[b]) Serial.printf("%d,%u\n", b, (unsigned)scan[b]);
  Serial.println("end");
}

// ===================== 4. SETUP AND LOOP =====================

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nME222 Lab 7 - lidar test");
  for (int i = 0; i < 360; ++i) { cosTable[i] = cosf(i * PI / 180); sinTable[i] = sinf(i * PI / 180); }

  pinMode(LIDAR_PWM_PIN, OUTPUT);
  analogWrite(LIDAR_PWM_PIN, LIDAR_MOTOR_DUTY);
  Lidar.setRxBufferSize(8192);           // must come before begin()
  Lidar.begin(115200, SERIAL_8N1, LIDAR_RX_PIN, LIDAR_TX_PIN);
  delay(1500);                           // let the motor reach speed

  uint8_t model, fwMajor, fwMinor, hardware;
  if (lidarGetInfo(model, fwMajor, fwMinor, hardware))
    Serial.printf("lidar model %u, firmware %u.%02u, hardware %u\n", model, fwMajor, fwMinor, hardware);
  else
    Serial.println("NO ANSWER from the lidar. Check the 5 V wires, and that lidar TX goes to GPIO 16 and lidar RX to GPIO 17.");

  uint8_t status = 0; uint16_t code = 0;
  if (lidarGetHealth(status, code)) {
    Serial.printf("health %u (0 good, 1 warning, 2 error)\n", status);
    if (status == 2) { lidarSend(CMD_RESET); delay(1000); lidarFlush(); }
  }
  Serial.println(lidarStartScan() ? "scan started" : "scan did NOT start");
}

void loop() {
  lidarPoll();                           // every pass: no delay() anywhere in loop()
  if (scanReady) { scanReady = false; processScan(); }

  if (Serial.available()) {
    int c = Serial.read();
    if (c == 'd' || c == 'D') dumpScan();
  }

  if (millis() - lastNodeMs > 1000) {    // no data for a second: restart the scan
    Serial.println("no data from the lidar: is the head spinning? restarting the scan");
    lidarStartScan();
    lastNodeMs = millis();
  }
}
