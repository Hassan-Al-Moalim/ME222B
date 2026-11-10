/*
  ME222 Lab 7 - RPLiDAR A1M8 starter

  What this sketch already does:
    - spins the lidar motor and talks to the lidar on a second serial port
    - checks the lidar's health and starts a scan
    - decodes every measurement and stores one full turn in scan[]
    - prints one summary line about once a second

  What you add (sections marked TODO):
    - Part B: ANGLE_OFFSET_DEG and ANGLE_REVERSED so that 0 deg is the car's front
    - Part D: sectorMin()
    - Part E: the CLEAR / AVOID LEFT / AVOID RIGHT / BLOCKED decision

  Serial Monitor: 115200 baud. Send the letter d to print the last full scan as
  CSV (bearing in degrees, distance in mm).

  No library is needed. Board: ESP32 Dev Module.
*/

#include <Arduino.h>

// ===================== 1. SETTINGS =====================

// The three lidar signals, as wired in the lab manual. No change needed.
const int LIDAR_RX_PIN  = 16;   // ESP32 input:  wired to the lidar's TX
const int LIDAR_TX_PIN  = 17;   // ESP32 output: wired to the lidar's RX
const int LIDAR_PWM_PIN = 4;    // ESP32 output: wired to the lidar's MOTOCTL

const int LIDAR_MOTOR_DUTY = 255;        // 0 to 255. 255 = full speed

// Part B. Bearing 0 must be straight ahead of the car, 90 on its right.
const float ANGLE_OFFSET_DEG = 0.0f;     // raw angle at which an object straight ahead appears
const bool  ANGLE_REVERSED   = false;    // true if an object on the right shows near 270

const uint16_t D_MIN_MM = 150;           // ignore anything closer: lidar blind zone and the car's own parts
const int PRINT_EVERY = 5;               // print one line every this many scans

// ===================== 2. LIDAR DRIVER (given, no changes needed) =====================

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

// ===================== 3. YOUR CODE =====================

// Closest valid point in the whole scan. Returns its distance in mm (0 if the
// scan is empty) and writes its bearing to bearingDeg.
uint16_t nearestPoint(int &bearingDeg) {
  uint16_t best = 0;
  bearingDeg = -1;
  for (int b = 0; b < 360; ++b) {
    if (scan[b] && (best == 0 || scan[b] < best)) { best = scan[b]; bearingDeg = b; }
  }
  return best;
}

// TODO Part D: closest valid distance in mm between bearings
// (centerDeg - halfWidthDeg) and (centerDeg + halfWidthDeg). Return 0 if the
// sector holds no valid reading. Remember that a sector around 0 runs from
// 345 through 359 and on to 15.
uint16_t sectorMin(int centerDeg, int halfWidthDeg) {
  (void)centerDeg; (void)halfWidthDeg;
  return 0;
}

// Runs once per full turn, right after scan[] is refreshed.
void processScan() {
  bool show = (scanCount % PRINT_EVERY) == 0;

  int nearestDeg;
  uint16_t nearestMm = nearestPoint(nearestDeg);

  // TODO Part D: front, right and left clearances from sectorMin()

  // TODO Part E: decide CLEAR / AVOID LEFT / AVOID RIGHT / BLOCKED.
  //              Confirm over several turns before you change the decision.

  if (show) {
    Serial.printf("scan %lu | %.1f turns/s | %d points, %d bearings | nearest %u mm at %d deg | sync errors %lu\n",
                  (unsigned long)scanCount, scanHz, scanPoints, scanBins,
                  (unsigned)nearestMm, nearestDeg, (unsigned long)syncErrors);
  }
}

// Send d in the Serial Monitor to print the last full scan.
void dumpScan() {
  Serial.println("bearing_deg,distance_mm");
  for (int b = 0; b < 360; ++b) {
    if (scan[b]) Serial.printf("%d,%u\n", b, (unsigned)scan[b]);
  }
  Serial.println("end");
}

// ===================== 4. SETUP AND LOOP =====================

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("ME222 Lab 7 - RPLiDAR A1M8");

  pinMode(LIDAR_PWM_PIN, OUTPUT);
  analogWrite(LIDAR_PWM_PIN, LIDAR_MOTOR_DUTY);

  // The lidar sends about 10 000 bytes per second. The default 256-byte
  // receive buffer would overflow in 26 ms, so make it larger. This call must
  // come before begin().
  Lidar.setRxBufferSize(8192);
  Lidar.begin(115200, SERIAL_8N1, LIDAR_RX_PIN, LIDAR_TX_PIN);

  delay(1500);                                   // let the motor reach speed

  uint8_t model, fwMajor, fwMinor, hardware;
  if (lidarGetInfo(model, fwMajor, fwMinor, hardware)) {
    Serial.printf("lidar model %u, firmware %u.%02u, hardware %u\n", model, fwMajor, fwMinor, hardware);
  } else {
    Serial.println("NO ANSWER from the lidar. Check the 5 V wires, and that lidar TX goes to GPIO 16 and lidar RX to GPIO 17.");
  }

  uint8_t status = 0;
  uint16_t errorCode = 0;
  if (lidarGetHealth(status, errorCode)) {
    Serial.printf("health status %u (0 good, 1 warning, 2 error), error code %u\n", status, errorCode);
    if (status == 2) {
      Serial.println("lidar reports an error: resetting it");
      lidarSend(CMD_RESET);
      delay(1000);
      lidarFlush();
    }
  }

  Serial.println(lidarStartScan() ? "scan started" : "scan did NOT start");
}

void loop() {
  lidarPoll();

  if (scanReady) {
    scanReady = false;
    processScan();
  }

  if (Serial.available()) {
    int c = Serial.read();
    if (c == 'd' || c == 'D') dumpScan();
  }

  // No data for a second: is the motor turning? Try to start again.
  if (millis() - lastNodeMs > 1000) {
    Serial.println("no data from the lidar: is the head spinning? restarting the scan");
    lidarStartScan();
    lastNodeMs = millis();
  }

  // Do not put delay() anywhere in loop(). See the note in setup().
}
