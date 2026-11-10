---
title: Lab 7 — LiDAR Scanning
---

# Lab 7 — LiDAR Scanning

<div class="lab-meta" markdown>
<div><span class="k">Date</span><span class="v">Wednesday 11 November</span></div>
<div><span class="k">Depends on</span><span class="v">Labs 5 and 6</span></div>
<div><span class="k">Milestone</span><span class="v">Car reads a 360° scan and picks the avoidance side without turning</span></div>
</div>

**Goal:** bring up the RPLiDAR A1M8 on the ESP32, find out where it points and
how accurate it is, and use one scan to decide what the car should do about an
obstacle. In Lab 5 the car had to stop and turn left and right to look. With a
scanning lidar it can look in every direction while standing still.

## Hardware

| Component | Purpose |
| --- | --- |
| ESP32 on the carrier board | Reads the lidar and makes the decision |
| RPLiDAR A1M8 with its 7-pin cable | Measures distance in every direction around the car |
| Battery, switched on | Supplies the board's 5 V rail, which powers the lidar |
| USB cable | Upload and Serial Monitor |
| Box (matte, light colour, at least 20 cm wide), tape measure | Test target |

## How the sensor differs from the front ToF

The TF-Luna and VL53L0X measure along one fixed line. The RPLiDAR spins a laser
rangefinder and reports a stream of points. Each point has an **angle**, a
**distance** and a **quality** value.

| | Front ToF (Labs 5 and 6) | RPLiDAR A1M8 |
| --- | --- | --- |
| Directions measured | 1 | 360°, about one point per degree |
| Range | about 0.2 m to 8 m (TF-Luna) | about 0.15 m to 12 m |
| Rate | one distance every 10–30 ms | about 2000 points per second, 5–10 full turns per second |
| Interface | I2C | Serial (UART), 115200 baud |
| To look left and right | Turn the whole car | Read a different angle |

!!! warning "Handle the lidar by its base"
    Do not hold or stop the spinning head, and keep wires and fingers clear of
    it and of the drive belt. Plug and unplug the 7-pin cable only with the
    battery switched **off**; the 1.25 mm connector is small and its pins bend
    easily. The laser is Class 1 and is safe to look at.

## Before the lab

Read this page, then work out these two numbers. You need them in Part A.

1. Each lidar point is 5 bytes. At 2000 points per second, how many bytes per
   second does the lidar send?
2. A serial link at 115200 baud carries 11 520 bytes per second. The ESP32's
   serial receive buffer holds 256 bytes unless you enlarge it. If your `loop()`
   stops reading for 50 ms, how many bytes arrive, and what happens to them?

## Connections

The lidar plugs into the **RPLIDAR** connector on the bottom edge of the
[carrier board](../resources/pcb.md).

| Board pin (left → right) | Board net | Lidar pin | Function |
| --- | --- | --- | --- |
| 1 | GND | GND | Ground |
| 2 | `RPL_RX` | RX | Commands **into** the lidar (ESP32 transmits here) |
| 3 | `RPL_TX` | TX | Data **out of** the lidar (ESP32 receives here) |
| 4 | +5V | VCC_5 | Supply for the rangefinder |
| 5 | GND | GND_MOTO | Motor ground |
| 6 | `RPL_PWM` | MOTOCTL | Motor speed: high = spin, low = stop |
| 7 | +5V | 5V_MOTO | Supply for the motor |

!!! warning "RX and TX are named from the lidar's side"
    `RPL_TX` carries data the lidar **sends**, so it goes to the ESP32's
    **receive** pin. If the lidar never answers, the two pin numbers in your
    sketch are the first thing to swap.

The GPIO numbers for the three signals, and which ESP32 they go to, are given in
the lab. Record them before you upload anything.

<div class="worksheet" data-worksheet="lab07-pins" markdown>

| Signal | Sketch constant | ESP32 GPIO |
| --- | --- | --- |
| `RPL_TX` (lidar data out) | `LIDAR_RX_PIN` | `______` |
| `RPL_RX` (lidar commands in) | `LIDAR_TX_PIN` | `______` |
| `RPL_PWM` (motor) | `LIDAR_PWM_PIN` | `______` |

</div>

## Starter file

| File | Use |
| --- | --- |
| [`ME222_Lab7_RPLidar_Starter.ino`](lab07/ME222_Lab7_RPLidar_Starter.ino) | Talks to the lidar and stores one full turn in `scan[]`. You add the processing. |

No library is needed. The starter already decodes the serial stream. Its output
is one array:

```cpp
uint16_t scan[360];   // scan[b] = closest distance in mm at bearing b, or 0 if no valid reading
```

**Bearing convention used in this lab:** 0° is straight ahead of the car, 90° is
its right side, 180° is behind, 270° is its left side. Bearings increase
**clockwise** seen from above.

!!! warning "Opposite sign to your gyro"
    In Labs 3, 4 and 6 a left turn is **positive** yaw. Lidar bearings grow to
    the **right**. Keep the two apart in your code, and convert in one place
    only if you ever combine them.

`processScan()` runs once per full turn, right after `scan[]` is refreshed. All
of your code goes there or in functions it calls.

!!! danger "No `delay()` in `loop()`"
    This is the answer to the pre-lab question. The lidar fills most of the
    serial link. If the loop stops reading, the buffer overflows and points are
    lost. Use the `millis()` timing pattern from [Setup](../resources/setup.md#useful-snippets),
    and print one line per second, not one line per point.

## Part A · Bring-up

1. Battery **off**. Plug in the lidar cable. Check the head turns freely by hand.
2. Set the three pin constants at the top of the starter and upload it.
3. Battery **on**. The head should start spinning within two seconds.
4. Open the Serial Monitor at **115200** baud. You should see something like:

```text title="Serial Monitor"
ME222 Lab 7 - RPLiDAR A1M8
lidar model 24, firmware 1.29, hardware 7
health status 0 (0 good, 1 warning, 2 error), error code 0
scan started
scan 5 | 6.3 turns/s | 318 points, 240 bearings | nearest 412 mm at 87 deg | sync errors 0
scan 10 | 6.3 turns/s | 319 points, 238 bearings | nearest 409 mm at 87 deg | sync errors 0
```

What the numbers should look like:

- **turns/s** between 5 and 10, and steady.
- **points** per turn close to 2000 ÷ turns/s. At 6 turns/s that is about 330.
- **sync errors** stays at 0, or grows by only a few. A number that climbs
  every line means bytes are being lost.

<div class="worksheet" data-worksheet="lab07-bringup" markdown>

| Item | Your value |
| --- | --- |
| Model / firmware | `______` |
| Health status | `______` |
| Turns per second | `______` |
| Points per turn | `______` |
| Bearings with a valid reading | `______` |
| Angle between neighbouring points (360 ÷ points) | `______ °` |

</div>

!!! checkpoint "Checkpoint 1"
    - Health status 0 and a steady scan rate.
    - Move your hand around the lidar at about 30 cm: the *nearest* bearing
      follows it.

## Part B · Find the front

The lidar's own 0° depends on how it is mounted. Your code needs 0° to be the
car's front.

1. Leave `ANGLE_OFFSET_DEG = 0`. Clear the area for 1 m around the car.
2. Put the box 40 cm **straight ahead** of the car. Read the *nearest* bearing.
   That number is your offset. Set `ANGLE_OFFSET_DEG` to it and upload again.
   The box should now read within a few degrees of 0 (so 357 or 2 is fine).
3. Move the box to the car's **right** side. It should read near 90. If it reads
   near 270, set `ANGLE_REVERSED = true` and check the front again.
4. With nothing nearby, send `d` in the Serial Monitor. Look for short distances
   that never move. Those are parts of your own car: posts, wires, the other
   boards. Raise `D_MIN_MM` until they are gone, or note their bearings.

<div class="worksheet" data-worksheet="lab07-front" markdown>

| Item | Your value |
| --- | --- |
| `ANGLE_OFFSET_DEG` | `______` |
| `ANGLE_REVERSED` | `______` |
| Box in front reads | `______ °` |
| Box on the right reads | `______ °` |
| Box on the left reads | `______ °` |
| Bearings blocked by the car itself | `______` |
| `D_MIN_MM` | `______` |

</div>

## Part C · Check the distance

The lidar measures from the **centre of its spinning head**, not from the front
of the car.

1. Measure $d_0$, the distance from the centre of the lidar to the car's front
   edge.
2. Place the box straight ahead with its face square to the car. Measure from
   the lidar centre to the box with the tape. Record the lidar reading at
   bearing 0 (or the *nearest* value).
3. Repeat at each distance in the table.
4. Move the box closer than 15 cm from the lidar. Record what the reading does.

$$ e = d_{lidar} - d_{tape} \qquad \text{clearance in front of the car} = d_{lidar} - d_0 $$

<div class="worksheet" data-worksheet="lab07-distance" markdown>

| Tape distance (cm) | Lidar reading (mm) | Error e (mm) |
| --- | --- | --- |
| 20 | `____` | `____` |
| 30 | `____` | `____` |
| 50 | `____` | `____` |
| 100 | `____` | `____` |
| 150 | `____` | `____` |
| under 15 | `____` | — |

</div>

Expect errors of about 1–2 cm or less at these distances. A larger error
usually means the tape was not measured from the lidar centre, or the box was at
an angle.

<div class="worksheet" data-worksheet="lab07-d0" markdown>

| Item | Your value |
| --- | --- |
| $d_0$, lidar centre to front edge | `______ mm` |
| Closest distance that still gives a reading | `______ mm` |

</div>

## Part D · Sectors

One point is not a safe basis for a decision. Group the bearings into sectors
and use the closest valid reading in each.

```text
                 front
              345°  0°  15°
          \     |   |   |     /
   left     \   |  car  |   /     right
  285°–345°   \ |       | /      15°–75°
```

Write `sectorMin(centerDeg, halfWidthDeg)`. It returns the smallest non-zero
`scan[]` value between `centerDeg − halfWidthDeg` and `centerDeg + halfWidthDeg`,
or 0 if the sector has no valid reading.

!!! warning "The front sector crosses 359 → 0"
    A sector centred on 0 with half-width 15 covers 345…359 and 0…15. A plain
    `for (b = -15; b <= 15; b++)` reads outside the array. Wrap every index into
    0…359.

Choose the front sector's half-width from the geometry of your car. The front
sector must be wide enough that anything inside it would actually be hit:

$$ \alpha = \arctan\!\left(\frac{W/2 + m}{D}\right) $$

W is the car's width, m is a side margin, and D is the distance at which you
want to react, measured from the lidar. Example: W = 20 cm, m = 5 cm, D = 40 cm
gives α ≈ 21°.

Suggested starting sectors. Change them if you can justify it.

| Sector | Centre | Half-width |
| --- | --- | --- |
| Front | 0° | α from your calculation |
| Right | 45° | 30° |
| Left | 315° | 30° |

Print the three values on your once-per-second line. Then test:

<div class="worksheet" data-worksheet="lab07-sectors" markdown>

| Box position | Front (mm) | Right (mm) | Left (mm) |
| --- | --- | --- | --- |
| No box | `____` | `____` | `____` |
| 40 cm straight ahead | `____` | `____` | `____` |
| 40 cm ahead, shifted to the right | `____` | `____` | `____` |
| 40 cm ahead, shifted to the left | `____` | `____` | `____` |
| Beside the right wheels | `____` | `____` | `____` |

</div>

<div class="worksheet" data-worksheet="lab07-sector-params" markdown>

| Parameter | Value | Reason |
| --- | --- | --- |
| W, car width | `____ cm` | |
| m, side margin | `____ cm` | |
| D, reaction distance | `____ cm` | |
| α, front half-width | `____ °` | |

</div>

!!! checkpoint "Checkpoint 2"
    The three sector values change as expected when the TA moves the box, and
    the front value does not jump when the box crosses the 359 → 0 line.

## Part E · Decide

Use the three sector values to produce one of four decisions. No code is
provided; write it from this outline.

```text
EVERY FULL TURN
    front = closest reading in the front sector
    right = closest reading in the right sector
    left  = closest reading in the left sector

    IF front is beyond D_avoid:
        candidate = CLEAR
    ELSE IF left and right are both closer than D_side:
        candidate = BLOCKED
    ELSE IF left has more room than right:
        candidate = AVOID LEFT
    ELSE:
        candidate = AVOID RIGHT

    IF candidate has been the same for N turns in a row:
        decision = candidate
```

Things you must decide and write down:

- **What a 0 means.** `sectorMin()` returns 0 when the sector had no valid
  reading. Is that open space, or a surface the lidar cannot see? Choose how
  your code treats it and be ready to defend the choice.
- **N.** In Lab 5 you confirmed over N samples about 30 ms apart. Here one
  sample is a full turn. Work out how long N turns take, and how far the car
  travels in that time at your Lab 6 speed.
- **D_avoid.** It is measured from the lidar, so it must include $d_0$.

Print the decision whenever it changes, and on your once-per-second line.

<div class="worksheet" data-worksheet="lab07-decision-params" markdown>

| Parameter | Value | Reason |
| --- | --- | --- |
| $D_{avoid}$ | `____ mm` | |
| $D_{side}$ | `____ mm` | |
| N | `____ turns` | |
| Time for N turns | `____ ms` | |
| A sector reading of 0 is treated as | `____` | |

</div>

### Tests

Keep the car still. Place the box, wait, and record the decision.

<div class="worksheet" data-worksheet="lab07-tests" markdown>

| Test | Layout | Expected | Front / Right / Left (mm) | Decision | Correct? |
| --- | --- | --- | --- | --- | --- |
| 1 | Nothing within 1 m | CLEAR | | | |
| 2 | Box ahead, more room on the left | AVOID LEFT | | | |
| 3 | Box ahead, more room on the right | AVOID RIGHT | | | |
| 4 | Box ahead, boxes on both sides | BLOCKED | | | |
| 5 | Box beside the car only | CLEAR | | | |
| 6 | Hand waved once through the front sector | No change | | | |

</div>

!!! checkpoint "Checkpoint 3"
    Tests 1 to 6 demonstrated to a TA. The decision does not flicker when the
    box sits right at the threshold.

## Part F · On the car, if time allows

Replace the turn-and-scan step of your Lab 5 and Lab 6 avoidance with the lidar
decision.

1. Wheels raised first. Confirm the lidar still reports steadily while the
   motors run.
2. In your mission state machine, the obstacle trigger becomes
   *decision ≠ CLEAR*. The side comes from the decision, so the SCAN LEFT and
   SCAN RIGHT states are no longer needed.
3. BLOCKED must stop the car.
4. Add `front`, `left`, `right` and the decision to your ThingsBoard telemetry.
5. Keep every stop rule from Lab 6: STOP from the dashboard, link loss, and a
   stop if the lidar stops sending data.

How you join the lidar to the mission depends on how many ESP32s your car uses.

### If your car uses one ESP32

One ESP32 can run the motors, encoders, IMU, Wi-Fi and the lidar together. The
lidar uses the second hardware serial port, so the USB Serial Monitor keeps
working.

**Pins.** The three lidar signals need three GPIOs that nothing else uses. With
the Lab 3 and Lab 4 wiring, the starter's defaults are free:

| Already used | GPIO |
| --- | --- |
| Left motors PWM / DIR | 25 / 26 |
| Right motors PWM / DIR | 27 / 14 |
| I2C: IMU and front ToF | 21 / 22 |
| Encoders | your Lab 4 pins |
| **Lidar data in (`LIDAR_RX_PIN`)** | **16** |
| **Lidar commands out (`LIDAR_TX_PIN`)** | **17** |
| **Lidar motor (`LIDAR_PWM_PIN`)** | **4** |

Check the three numbers against your own pin table before you use them. Do not
use GPIO 1 or 3 (USB serial), 6–11 (flash), or 0, 2, 12, 15 (boot pins).
GPIO 34–39 are inputs only, so they can serve as `LIDAR_RX_PIN` but not as the
other two.

**Power.** Both lidar supply pins take 5 V from the board's 5 V rail. Never
power the lidar from the ESP32's 3.3 V pin. All grounds are common.

**Code.** Work in a copy of your Lab 6 sketch.

1. Copy sections 1 and 2 of the starter (settings and lidar driver) into it,
   together with your `sectorMin()` and decision code.
2. Copy the lidar lines of the starter's `setup()` into your `setup()`.
3. Call `lidarPoll()` on **every** pass of `loop()`, and run `processScan()`
   when `scanReady` is true. Store the decision in a global variable.
4. Your mission state machine reads that variable. It never talks to the lidar
   itself.
5. Reduce the starter's printing to state changes only.

!!! warning "Every wait in your code must keep reading the lidar"
    Any `delay()`, and any `while` loop that waits for a turn to finish or for
    Wi-Fi to reconnect, stops `lidarPoll()`. The lidar buffer then overflows.
    Nothing crashes, because the decoder finds its place again, but the scan is
    stale: wait for one new full turn before you trust the decision.
    The proper fix is to call `lidarPoll()` inside every waiting loop, or to
    replace the wait with a state and a `millis()` check.

Check it with the wheels raised: *points per turn* and *turns/s* must stay the
same with the motors running and MQTT connected as they were in Part A.

### If your car uses two ESP32s

The lidar ESP32 makes the decision and the motion ESP32 drives. They only need
to share the decision, which has four values, so two wires are enough.

| Decision | Line A | Line B |
| --- | --- | --- |
| CLEAR | 0 | 0 |
| AVOID LEFT | 0 | 1 |
| AVOID RIGHT | 1 | 0 |
| BLOCKED | 1 | 1 |

1. Pick two free GPIOs on each board and join them A to A and B to B. The
   boards already share ground on the carrier.
2. Lidar ESP32: outputs. Write both lines whenever the decision changes.
3. Motion ESP32: inputs with `INPUT_PULLDOWN`, so a missing wire reads as
   CLEAR on the bench. Read both lines on every control-loop pass, and accept a
   new value only after it has been the same on two passes in a row.
4. Decide what the motion ESP32 does if the lidar ESP32 resets or stops. A
   third "alive" line that the lidar board toggles every turn is one way.

!!! warning "The fast loop still has to run"
    Reading the lidar, the 10–20 ms heading controller and MQTT must not block
    each other. If the scan rate or *points per turn* drops when the motors
    start, something in your loop is waiting.

## Questions

Answer briefly in your notebook.

1. Your answers to the two pre-lab questions, and how the starter avoids the
   problem.
2. The angle between neighbouring points is about 1°. How wide is the gap
   between two neighbouring points on a wall 2 m away? What is the narrowest
   object you can count on seeing at that distance?
3. The lidar scans in one horizontal plane. Name two obstacles on the lab floor
   that it would miss, and say which sensor from earlier labs would catch them.
4. Compare the reaction time of your Lab 5 confirmation (N samples from the
   front ToF) with today's (N full turns). Which is faster, and what does the
   slower one give you in return?
5. Would a faster or slower motor speed (`LIDAR_MOTOR_DUTY`) be better for a
   moving car? What do you lose either way?

## Deliverable

- Completed tables on this page.
- Your sketch with `sectorMin()` and the decision logic.
- Checkpoints 1 to 3 signed off.

## Troubleshooting

??? failure "`NO ANSWER from the lidar` and the head is not spinning"
    No 5 V. Is the battery switch on? Is the 7-pin cable fully seated at both
    ends? Is `LIDAR_PWM_PIN` the right GPIO?

??? failure "`NO ANSWER from the lidar` but the head spins"
    1. RX and TX swapped. Exchange the two pin numbers in the sketch.
    2. Wrong GPIO numbers. Check them against the board table.
    3. Do not use GPIO 1 or 3; those are the USB serial port.

??? failure "`scan did NOT start`, or `no data from the lidar` repeats"
    The lidar answers commands only once it is powered, and sends points only
    while the head turns. Check the motor is spinning. Press **EN** to restart
    the sketch after switching the battery on.

??? failure "*sync errors* keeps climbing, or *points per turn* is far below 2000 ÷ turns/s"
    Bytes are being lost. Look for a `delay()`, a long `Serial.print` inside
    `processScan()`, or a slow sensor read in `loop()`. Check that
    `Lidar.setRxBufferSize()` is still called before `Lidar.begin()`.

??? failure "The nearest point never moves and is very close"
    The lidar sees part of the car. Raise `D_MIN_MM`, or move the wire or post
    out of the scan plane.

??? failure "A box in front reads near 180, or left and right are swapped"
    `ANGLE_OFFSET_DEG` or `ANGLE_REVERSED` is wrong. Redo Part B.

??? failure "A sector reads 0 although something is clearly there"
    Black, glossy or transparent surfaces return little light, and anything
    closer than about 15 cm is inside the blind zone. Try a matte, light box.

??? failure "The ESP32 resets when the lidar starts"
    The motor draws a surge of current at start-up. The lidar must be powered
    from the board's 5 V rail with the battery on, not from the USB port alone.

## Starter code

??? example "ME222_Lab7_RPLidar_Starter.ino"
    ```cpp
    --8<-- "docs/labs/lab07/ME222_Lab7_RPLidar_Starter.ino"
    ```
