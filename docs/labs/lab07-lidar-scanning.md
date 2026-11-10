---
title: Lab 7 — LiDAR Scanning
---

# Lab 7 — LiDAR Scanning

<div class="lab-meta" markdown>
<div><span class="k">Date</span><span class="v">Wednesday 11 November</span></div>
<div><span class="k">Depends on</span><span class="v">Labs 5 and 6</span></div>
<div><span class="k">Milestone</span><span class="v">Car reads a 360° scan and picks the avoidance side without turning</span></div>
</div>

**Goal:** bring up the RPLiDAR A1M8 on the ESP32, find where it points, and use
one scan to decide what the car should do about an obstacle. In Lab 5 the car
had to stop and turn left and right to look. A scanning lidar looks in every
direction while the car stands still.

The lidar spins a laser rangefinder and sends about 2000 points per second,
5–10 full turns per second, over a serial link at 115200 baud. Each point is an
angle and a distance. It measures from about 0.15 m to 12 m.

**Bring:** the car with a charged battery, USB cable, a matte light-coloured
box at least 20 cm wide, and a tape measure.

## Connections

The lidar plugs into the **RPLIDAR** connector on the bottom edge of the
[carrier board](../resources/pcb.md) with its 7-pin cable. It is powered from
the board's 5 V rail, so the battery must be switched on. Leave the TF-Luna and
the IMU connected; the lidar has its own connector and its own serial port.

!!! warning "Handling"
    Plug and unplug the cable only with the battery **off**. Do not hold or stop
    the spinning head, and keep wires clear of it.

Three signals go to the ESP32. Their GPIO numbers are given in the lab.

<div class="worksheet" data-worksheet="lab07-pins" markdown>

| Board net | What it carries | Sketch constant | ESP32 GPIO |
| --- | --- | --- | --- |
| `RPL_TX` | Data out of the lidar | `LIDAR_RX_PIN` | `______` |
| `RPL_RX` | Commands into the lidar | `LIDAR_TX_PIN` | `______` |
| `RPL_PWM` | Motor on / speed | `LIDAR_PWM_PIN` | `______` |

</div>

The board names are from the lidar's side: `RPL_TX` goes to the ESP32's
**receive** pin. If the lidar never answers, swap the two pin numbers in the
sketch.

## Starter file

[`ME222_Lab7_RPLidar_Starter.ino`](lab07/ME222_Lab7_RPLidar_Starter.ino) talks to
the lidar and decodes the stream. No library is needed. It gives you one array,
refreshed once per full turn:

```cpp
uint16_t scan[360];   // scan[b] = closest distance in mm at bearing b, or 0 if no valid reading
```

**Bearings:** 0° is straight ahead of the car, 90° its right side, 180° behind,
270° its left side. This is the opposite sign to your gyro, where a left turn
is positive.

`processScan()` runs once per turn. Your code goes there.

!!! danger "No `delay()` in `loop()`"
    The lidar sends 10 000 bytes per second, which nearly fills the serial
    link. If the loop stops reading, points are lost. Print one line per
    second, not one line per point.

## Part A · Bring-up

1. Battery off. Plug in the lidar. Set the three pin constants and upload.
2. Battery on. The head should start spinning within two seconds.
3. Open the Serial Monitor at **115200** baud:

```text title="Serial Monitor"
lidar model 24, firmware 1.29, hardware 7
health status 0 (0 good, 1 warning, 2 error), error code 0
scan started
scan 5 | 6.3 turns/s | 318 points, 240 bearings | nearest 412 mm at 87 deg | sync errors 0
```

Expect 5–10 turns/s, points per turn close to 2000 ÷ turns/s, and *sync errors*
staying near 0. Move your hand around the lidar at 30 cm: the *nearest* bearing
should follow it.

<div class="worksheet" data-worksheet="lab07-bringup" markdown>

| Health status | Turns per second | Points per turn |
| --- | --- | --- |
| `______` | `______` | `______` |

</div>

## Part B · Find the front

The lidar's own 0° depends on how it is mounted.

1. Put the box 40 cm **straight ahead** of the car. Read the *nearest* bearing,
   set `ANGLE_OFFSET_DEG` to that number, and upload. The box should now read
   within a few degrees of 0 (357 or 2 is fine).
2. Move the box to the car's **right**. It should read near 90. If it reads
   near 270, set `ANGLE_REVERSED = true`.
3. With nothing nearby, send `d` in the Serial Monitor to list the whole scan.
   Short distances that never move are parts of your own car. Raise `D_MIN_MM`
   until they are gone.

<div class="worksheet" data-worksheet="lab07-front" markdown>

| `ANGLE_OFFSET_DEG` | `ANGLE_REVERSED` | Box in front reads | Box on the right reads | `D_MIN_MM` |
| --- | --- | --- | --- | --- |
| `______` | `______` | `______` | `______` | `______` |

</div>

## Part C · Check the distance

The lidar measures from the **centre of its head**, not from the front of the
car. Measure $d_0$, the distance from the lidar centre to the car's front edge.
Then place the box straight ahead and compare the lidar with the tape, both
measured from the lidar centre. Expect errors of 1–2 cm or less.

<div class="worksheet" data-worksheet="lab07-distance" markdown>

| Tape distance (cm) | Lidar reading (mm) | Error (mm) |
| --- | --- | --- |
| 30 | `____` | `____` |
| 50 | `____` | `____` |
| 100 | `____` | `____` |
| under 15 | `____` | — |
| $d_0$ | `____` | — |

</div>

## Part D · Sectors

One point is not a safe basis for a decision. Write
`sectorMin(centerDeg, halfWidthDeg)`: the smallest **non-zero** `scan[]` value
between `centerDeg − halfWidthDeg` and `centerDeg + halfWidthDeg`, or 0 if the
sector has no valid reading.

!!! warning "The front sector crosses 359 → 0"
    A sector centred on 0 with half-width 15 covers 345…359 and 0…15. Wrap every
    index into 0…359.

Use three sectors: **front** centred on 0°, **right** on 45° and **left** on
315°, the sides with a half-width of 30°. Choose the front half-width so that
anything inside it would be hit:

$$ \alpha = \arctan\!\left(\frac{W/2 + m}{D}\right) $$

W is the car's width, m a side margin, and D the distance at which you want to
react. Example: W = 20 cm, m = 5 cm, D = 40 cm gives α ≈ 21°.

Print the three values on your once-per-second line and test:

<div class="worksheet" data-worksheet="lab07-sectors" markdown>

| Box position | Front (mm) | Right (mm) | Left (mm) |
| --- | --- | --- | --- |
| No box | `____` | `____` | `____` |
| 40 cm straight ahead | `____` | `____` | `____` |
| 40 cm ahead, shifted right | `____` | `____` | `____` |
| 40 cm ahead, shifted left | `____` | `____` | `____` |

</div>

## Part E · Decide

Write the decision from this outline. No code is provided.

```text
EVERY FULL TURN
    IF front is beyond D_avoid:                        candidate = CLEAR
    ELSE IF left and right are both closer than D_side: candidate = BLOCKED
    ELSE IF left has more room than right:              candidate = AVOID LEFT
    ELSE:                                               candidate = AVOID RIGHT

    IF candidate has been the same for N turns in a row:
        decision = candidate
```

Decide and record:

- **What a 0 means.** Open space, or a surface the lidar cannot see?
- **N.** One sample is now a full turn. How long do N turns take, and how far
  does the car travel in that time?
- **$D_{avoid}$.** It is measured from the lidar, so it includes $d_0$.

<div class="worksheet" data-worksheet="lab07-decision-params" markdown>

| α (front half-width) | $D_{avoid}$ | $D_{side}$ | N | A reading of 0 is treated as |
| --- | --- | --- | --- | --- |
| `____ °` | `____ mm` | `____ mm` | `____` | `____` |

</div>

Keep the car still, place the box, and record the decision:

<div class="worksheet" data-worksheet="lab07-tests" markdown>

| Test | Layout | Expected | Decision |
| --- | --- | --- | --- |
| 1 | Nothing within 1 m | CLEAR | |
| 2 | Box ahead, more room on the left | AVOID LEFT | |
| 3 | Box ahead, more room on the right | AVOID RIGHT | |
| 4 | Box ahead, boxes on both sides | BLOCKED | |
| 5 | Hand waved once through the front sector | No change | |

</div>

## Part F · On the car, if time allows

Replace the turn-and-scan step of your Lab 5 and Lab 6 avoidance with the lidar
decision. The obstacle trigger becomes *decision ≠ CLEAR*, the side comes from
the decision, and BLOCKED stops the car. Keep every stop rule from Lab 6.

**One ESP32.** The lidar uses the second serial port, so it runs beside the
motors, IMU and Wi-Fi. It needs three GPIOs that nothing else uses; with the
Lab 3 and Lab 4 wiring, 16 (data in), 17 (commands out) and 4 (motor) are free.
Copy the starter's settings and lidar driver into a copy of your Lab 6 sketch,
call `lidarPoll()` on **every** pass of `loop()`, and let the state machine read
the decision from a global variable.

!!! warning "Every wait must keep reading the lidar"
    A `delay()` or a `while` loop that waits for a turn to finish stops
    `lidarPoll()`, and the scan goes stale. Call `lidarPoll()` inside every
    waiting loop, or wait for one new full turn before trusting the decision.

**Two ESP32s.** The lidar ESP32 decides and the motion ESP32 drives. Share the
decision over two GPIO lines: 00 CLEAR, 01 AVOID LEFT, 10 AVOID RIGHT,
11 BLOCKED.

## Questions

1. Neighbouring points are about 1° apart. How wide is the gap between them on
   a wall 2 m away?
2. The lidar scans in one horizontal plane. Name two obstacles it would miss.
3. Compare the reaction time of your Lab 5 confirmation with today's. What does
   the slower one give you in return?

**Deliverable:** the completed tables and your sketch.

## Troubleshooting

??? failure "`NO ANSWER from the lidar`"
    Head not spinning: no 5 V (battery switch, cable) or wrong `LIDAR_PWM_PIN`.
    Head spinning: swap the RX and TX pin numbers, or the GPIO numbers are
    wrong.

??? failure "*sync errors* keeps climbing"
    Bytes are being lost. Look for a `delay()` or a long print in your code.

??? failure "The nearest point never moves and is very close"
    The lidar sees part of the car. Raise `D_MIN_MM` or move the wire.

??? failure "A sector reads 0 although something is there"
    Black, glossy or transparent surfaces return little light, and anything
    closer than about 15 cm is in the blind zone.

## Starter code

??? example "ME222_Lab7_RPLidar_Starter.ino"
    ```cpp
    --8<-- "docs/labs/lab07/ME222_Lab7_RPLidar_Starter.ino"
    ```
