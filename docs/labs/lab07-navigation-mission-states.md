---
title: Lab 7 — ThingsBoard + Autonomous Obstacle Avoidance
---

# Lab 7 — ThingsBoard + Autonomous Obstacle Avoidance

<div class="lab-meta" markdown>
<div><span class="k">Date</span><span class="v">Wednesday 4 November</span></div>
<div><span class="k">Depends on</span><span class="v">Labs 4 and 5</span></div>
<div><span class="k">Milestone</span><span class="v">Dashboard-started 3 m mission that avoids an obstacle</span></div>
</div>

## Before the lab — ESP32 + ThingsBoard 15-minute tutorial

Complete this short exercise before working on the car. The only objective is
to prove that your ESP32 can join the lab Wi-Fi, connect to the Raspberry Pi
ThingsBoard server over MQTT, and send one live telemetry value.

Official reference: [ThingsBoard Arduino/ESP32 Client SDK](https://thingsboard.io/docs/paas/reference/arduino-client-sdk/)

**Step 1 — Connect to the lab network.** Connect your laptop and ESP32 to SSID
`TP-Link_DA7D`. The Wi-Fi password will be provided in the lab.

**Step 2 — Find the Raspberry Pi address.** The Raspberry Pi IP will be provided.

**Step 3 — Log in and obtain your device token.** Open
`http://<RASPBERRY_PI_IP>:8080`, log in with your Team account, define your
`ME222_Car_#` device, and copy its access token. The token is the ESP32 MQTT
credential; it is not your Team password.

**Step 4 — Install the ThingsBoard Arduino library.** In Arduino IDE: Tools /
Sketch → Manage Libraries, search for *ThingsBoard*, and install the ThingsBoard
library. The official SDK supports ESP32 over MQTT.

**Step 5 — Run a simple telemetry test.** Use the official ESP32 example as a
starting point. Change only the lab-specific settings:

```cpp title="Lab-specific settings"
const char* WIFI_SSID        = "TP-Link_DA7D";
const char* WIFI_PASSWORD    = "<LAB PASSWORD>";      // given in the lab
const char* THINGSBOARD_IP   = "<RASPBERRY_PI_IP>";   // given in the lab
const int   THINGSBOARD_PORT = 1883;
const char* ACCESS_TOKEN     = "<YOUR DEVICE TOKEN>"; // from your ME222_Car_# device
```

For the pre-lab test, send one telemetry value every second, for example
`distance = 0.0` or `testValue` = a changing number. Confirm that it appears
under the device's **Latest Telemetry** in ThingsBoard.

## Lab network

| Item | Setting |
| --- | --- |
| Wi-Fi SSID | `TP-Link_DA7D` |
| Wi-Fi password | Provided in the lab |
| ThingsBoard web | `http://<RASPBERRY_PI_IP>:8080` |
| MQTT broker | `<RASPBERRY_PI_IP>` |
| MQTT port | `1883` |

Team 1–Team 5 accounts are prepared. The initial classroom password is provided
in the lab.

## Part A — Connect to ThingsBoard

Connect the laptop, Raspberry Pi and car to the same lab Wi-Fi. On the student
laptop open `http://<RASPBERRY_PI_IP>:8080` and log in with the assigned Team
account.

## Part B — Create or verify the car device

Each team needs one ThingsBoard device: `ME222_Car_1` … `ME222_Car_5`. Open its
credentials and copy the device access token.

!!! warning "The login password is not the ESP32 credential"
    The ESP32 uses the **device access token**, not the student login password.

## Part C — ESP32 → ThingsBoard telemetry

| MQTT item | Value |
| --- | --- |
| Broker | Raspberry Pi IP |
| Port | `1883` |
| Username | Device access token |
| Password | empty |
| Telemetry topic | `v1/devices/me/telemetry` |

Minimum first telemetry:

```json title="Topic: v1/devices/me/telemetry"
{
  "distance": 0.00,
  "obstacleDistance": 75,
  "heading": 0.0,
  "state": "READY"
}
```

### Starter ESP32 MQTT structure

```cpp title="Starter MQTT structure"
#include <WiFi.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "TP-Link_DA7D";
const char* WIFI_PASS = "LAB_WIFI_PASSWORD";
const char* TB_HOST   = "RASPBERRY_PI_IP";
const int   TB_PORT   = 1883;
const char* TOKEN     = "YOUR_DEVICE_ACCESS_TOKEN";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void publishTelemetry(float d, float obs, float heading, const char* state) {
  char msg[180];
  snprintf(msg, sizeof(msg),
    "{\"distance\":%.2f,\"obstacleDistance\":%.1f,\"heading\":%.1f,\"state\":\"%s\"}",
    d, obs, heading, state);
  mqtt.publish("v1/devices/me/telemetry", msg);
}
```

!!! checkpoint "Checkpoint 1"
    - ESP32 connected to Wi-Fi.
    - ThingsBoard device is active.
    - `distance`, `obstacleDistance`, `heading` and `state` appear as latest telemetry.

## Part D — Build a professional ThingsBoard car dashboard

In this lab the dashboard is part of the engineering task. Do not work only from
Latest Telemetry. Build a clear HMI that lets a user understand the car state
immediately and issue the high-level mission command.

### Target dashboard

```text
┌──────────────────────────────────────────────────┐
│          ME222 AUTONOMOUS CAR — TEAM #           │
├───────────────┬───────────────┬──────────────────┤
│ DISTANCE      │ OBSTACLE      │ HEADING          │
│    1.84 m     │    47 cm      │    +3.2°         │
├───────────────┴───────────────┴──────────────────┤
│                 STATUS: MOVING                   │
├──────────────────────────────────────────────────┤
│              TARGET DISTANCE: 3.0 m              │
│                                                  │
│             [ ▶ START MISSION ]                  │
│                  [ ■ STOP ]                      │
├──────────────────────────────────────────────────┤
│ Distance / obstacle history versus time          │
│ 3 m ┤                         ╭────              │
│ 2 m ┤                ╭────────╯                  │
│ 1 m ┤       ╭────────╯                           │
│ 0 m ┼───────╯                                    │
└──────────────────────────────────────────────────┘
```

### D1 — Create the dashboard

1. From the ThingsBoard left menu, open **Dashboards**.
2. Select **Add dashboard (+)** and create a dashboard named *ME222 Car — Team #*.
3. Open the new dashboard and enter **Edit** mode.
4. Keep the layout simple: telemetry cards at the top, mission controls in the center, and the history graph at the bottom.

### D2 — Add the car as a dashboard data source

1. In Edit mode, create or select an entity alias for your team's `ME222_Car_#` device.
2. Select **Device** as the entity type and choose only your team's car.
3. Use this same car alias for all telemetry widgets so the whole dashboard follows one device.

### D3 — Build the live telemetry cards

Add four clear cards or gauges using the telemetry already sent by the ESP32:

| Card | Telemetry key | Unit | Example |
| --- | --- | --- | --- |
| Distance | `distance` | m | 1.84 m |
| Obstacle | `obstacleDistance` | cm | 47 cm |
| Heading | `heading` | ° | +3.2° |
| Car State | `state` | — | MOVING |

For each card: Add widget → choose a card/value widget → select the car alias →
choose the telemetry key → set the unit and number of decimals → give the widget
a clear title.

Recommended formatting: distance 2 decimals; obstacle distance 0–1 decimal;
heading 1 decimal; state as text. The dashboard must be readable from a few
meters away during the car test.

### D4 — Add the history graph

1. Add a **Time series / Line chart** widget.
2. Select the car alias.
3. Add `distance` as the first series.
4. Add `obstacleDistance` as a second series if the scales stay understandable; otherwise use a second small graph.
5. Set the time window so the complete test run is visible.
6. During the final run, the graph should show motion progress and the obstacle event.

### D5 — Add the mission controls

The dashboard must provide high-level mission controls only. **Do not** create
Forward / Back / Left / Right driving buttons.

- Create a prominent **START MISSION** control. It requests a 3.0 m autonomous mission.
- Create a separate **STOP** control. STOP is an emergency/safety command and must remain available during every run.

Label the controls clearly enough that another student can operate the system
without reading the source code.

### MQTT command design used in this lab

Use ThingsBoard **shared attributes** as the command interface. The dashboard
changes command values; the ESP32 receives the attribute update over MQTT.

| Button | Shared attributes written |
| --- | --- |
| START MISSION | `command = "START"`, `targetDistance = 3.0` |
| STOP (emergency) | `command = "STOP"` |

The ESP32 subscribes to attribute updates on `v1/devices/me/attributes`.

The exact widget configuration can differ slightly with the ThingsBoard version.
The engineering requirement is that the dashboard writes the shared
command/target values and the ESP32 reacts to the resulting MQTT attribute
update.

### D6 — ESP32 command behavior

```text title="On a shared-attribute update"
if command == "START":
    targetDistance = received targetDistance
    reset encoder distance
    reset heading reference
    state = MOVING
    missionActive = true

if command == "STOP":
    stop motors immediately
    missionActive = false
    state = STOPPED
```

### D7 — Dashboard checkpoint

- [ ] Dashboard title identifies the team/car.
- [ ] Distance card updates live.
- [ ] Obstacle-distance card updates live.
- [ ] Heading card updates live.
- [ ] State changes are visible.
- [ ] History graph records the test.
- [ ] START MISSION sends the high-level command.
- [ ] STOP safely stops the car.
- [ ] No manual steering buttons are present.

!!! note "Dashboard design requirement"
    A good dashboard is part of the assessment. It should have a clear visual
    hierarchy, meaningful units, large readable values, and only the controls
    needed for the mission. You may customize the appearance, but all required
    information and controls must remain obvious.

## Part E — Send the mission using MQTT shared attributes

START changes the ThingsBoard shared command attributes. The ESP32 receives the
update over MQTT. Do not build Forward/Left/Right manual driving controls.

| Command | Shared attributes |
| --- | --- |
| Start | `command = "START"`, `targetDistance = 3.0` |
| Emergency stop | `command = "STOP"` |

ESP32 subscribes to shared-attribute updates on `v1/devices/me/attributes`.

### MQTT shared-attribute callback logic

```text title="Callback logic"
When an RPC arrives:
1. Parse method and parameters.
2. If method = move:
   - store targetDistance
   - reset distance and heading reference
   - set missionActive = true
   - set state = MOVING
3. If method = stop:
   - stop motors immediately
   - missionActive = false
   - set state = STOPPED
```

## Part F — Autonomous control

After START, the dashboard must not steer the car. The ESP32 executes the
mission locally.

### Required state machine

```text title="Mission states"
READY
  │ START
  ▼
MOVING ── obstacle too close ──► STOP / AVOID
  │                                │
  │                                ▼
  │                             REALIGN
  │                                │
  └────────── resume ◄─────────────┘
  │
  └── travelled distance ≥ target ──► FINISHED
```

| Sensor | Measurement | Purpose |
| --- | --- | --- |
| Encoders | Wheel counts / distance | Travelled distance and left/right motion |
| IMU gyro | Yaw rate / integrated heading | Heading correction and recovery |
| Front ToF | Obstacle distance | Safe stop and avoidance trigger |

### Straight-line heading correction

$$ u = \operatorname{clip}(K_h\theta + K_r\omega,\ -u_{max},\ +u_{max}) $$

$$ P_L = \operatorname{clip}(P_0 + u, 0, 255) \qquad P_R = \operatorname{clip}(P_0 - u, 0, 255) $$

θ is heading error and ω is yaw rate. Tune at low speed before adding obstacle
avoidance.

### Obstacle behavior

Detect early enough to stop without contact, execute an avoidance maneuver,
realign, then continue the mission. The avoidance strategy is a team design
choice.

Start testing around 30–50 cm depending on speed and measured stopping
distance. The final threshold must be justified experimentally.

### Control-loop pseudocode

```text title="Control loop, every 10–20 ms on the ESP32"
service MQTT
read encoders
read gyro and update heading
read front ToF

if STOP received:
    motors OFF

if missionActive:
    if obstacleDistance < safeThreshold:
        run avoidance state machine
    else:
        apply heading correction

    update travelled distance

    if travelled distance >= targetDistance:
        motors OFF
        state = FINISHED
        missionActive = false

periodically publish telemetry
```

!!! warning "Engineering rule"
    Do not run the motor feedback loop through ThingsBoard. MQTT/Wi-Fi is for
    mission commands and monitoring; the fast loop stays on the ESP32.

## Part G — 3-meter autonomous challenge

- Instructor places an obstacle along the nominal path.
- Set target to 3.0 m and press START once.
- No manual steering after START.
- No contact with the obstacle.
- Car must continue autonomously and stop at mission completion.
- STOP is always allowed for safety, but ends the scored attempt.

### Required telemetry during the run

```json title="Topic: v1/devices/me/telemetry"
{
  "distance": 1.84,
  "obstacleDistance": 47,
  "heading": 3.2,
  "leftPWM": 142,
  "rightPWM": 135,
  "state": "MOVING"
}
```
