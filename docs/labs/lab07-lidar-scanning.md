---
title: Lab 7 — Driving with the LiDAR
---

# Lab 7 — Driving with the LiDAR

<div class="lab-meta" markdown>
<div><span class="k">Date</span><span class="v">Wednesday 11 November</span></div>
<div><span class="k">Depends on</span><span class="v">Labs 3 to 6</span></div>
<div><span class="k">Milestone</span><span class="v">Car drives past an obstacle using the lidar, started from ThingsBoard</span></div>
</div>

**Goal:** mount the RPLiDAR on your car, check what it sees, then let the car
drive by itself and steer around obstacles. Finally, start and stop it from
the ThingsBoard dashboard on your laptop.

The lidar spins and measures distance all around the car, about 6 times a
second. In Lab 5 the car had to stop and turn to look left and right. Now it
sees both sides at once.

**You do not write the code from scratch.** You get three sketches. You set
your car's values, test, and tune.

| Step | Sketch | What it does |
| --- | --- | --- |
| 2–3 | [`ME222_Lab7_1_LidarTest.ino`](lab07/ME222_Lab7_1_LidarTest.ino) | Shows what the lidar sees. The car does not move. |
| 4 | [`ME222_Lab7_2_Drive.ino`](lab07/ME222_Lab7_2_Drive.ino) | The car drives and avoids obstacles. |
| 5 | [`ME222_Lab7_3_Drive_ThingsBoard.ino`](lab07/ME222_Lab7_3_Drive_ThingsBoard.ino) | Same, started and watched from ThingsBoard. |

**Before the lab:** in the Arduino IDE Library Manager, install
**PubSubClient** by Nick O'Leary (needed in Step 5). Have your Lab 6
`ME222_Car_#` device token ready.

**Bring:** the car with a charged battery, USB cable, a tape measure, and a
matte, light-coloured box at least 20 cm wide.

## Step 1 · Wire the lidar

Battery **off** while you wire. Your car keeps its one ESP32 and its Lab 6
wiring.

1. **Take out the TF-Luna.** Unplug it and put it aside. The lidar replaces it.
   Leave the motors and IMU as they are.
2. **Mount the lidar** on top of the car, in the middle, with nothing taller
   than its head around it.
3. **Wire it** to the ESP32:

| Lidar wire | Connect to |
| --- | --- |
| TX | ESP32 **GPIO 16** |
| RX | ESP32 **GPIO 17** |
| MOTOCTL | ESP32 **GPIO 4** |
| VCC_5 | 5 V (the 5 V the TF-Luna used) |
| 5V_MOTO | 5 V |
| GND | GND |
| GND_MOTO | GND |

All three sketches already use these pins. If GPIO 4, 16 or 17 is already
used on your car, tell a TA before you rewire.

!!! danger "5 V only"
    Both lidar supply wires go to 5 V. Never connect them to the ESP32's 3.3 V
    pin.

!!! warning "Handling"
    Do not hold or stop the spinning head, and keep wires clear of it.

## Step 2 · Check the lidar

1. Upload `ME222_Lab7_1_LidarTest.ino`.
2. Switch the battery on. The lidar head should start spinning.
3. Open the Serial Monitor at **115200** baud. About once a second you see:

```text title="Serial Monitor"
6.3 turns/s 318 pts | nearest   412 mm at  87 | ahead  clear | left  clear | right  clear | CLEAR | sync errors 0
```

It is working if *turns/s* is between 5 and 10 and *sync errors* stays at 0.
Move your hand around the lidar at 30 cm: the *nearest* bearing follows your
hand.

<div class="worksheet" data-worksheet="lab07-bringup" markdown>

| Turns per second | Points per turn | Sync errors |
| --- | --- | --- |
| `______` | `______` | `______` |

</div>

## Step 3 · Point it forward and check the lane

**Bearings** are angles around the car: 0° is straight ahead, 90° is the
right side, 180° behind, 270° the left side.

1. Put the box 40 cm **straight ahead** of the car. Read the *nearest*
   bearing. Write that number into `ANGLE_OFFSET_DEG` and upload again. The box
   should now read close to 0 (anything from 357 to 3 is fine).
2. Move the box to the car's **right** side. It should read near 90. If it
   reads near 270, set `ANGLE_REVERSED = true` and upload.
3. Remove the box. If *nearest* still shows a short distance that never moves,
   the lidar is seeing part of your car. Raise `D_MIN_MM` until it goes away.
4. Measure your car's width and set `CAR_WIDTH_MM`.

The sketch checks a **lane** as wide as your car plus a margin, straight ahead
and 45° to each side. *ahead*, *left* and *right* are the distances to the
nearest thing in each lane.

```text
          left lane      ahead lane      right lane
              \           |      |           /
               \          |      |          /
                \         | car  |         /
```

5. Place the box at each position below and record what the sketch says.

<div class="worksheet" data-worksheet="lab07-lanes" markdown>

| Box position | ahead (mm) | left (mm) | right (mm) | The car would… | Expected |
| --- | --- | --- | --- | --- | --- |
| No box | | | | | CLEAR |
| 40 cm ahead, mostly on the right | | | | | AVOID LEFT |
| 40 cm ahead, mostly on the left | | | | | AVOID RIGHT |
| 40 cm ahead, boxes beside the car on both sides | | | | | BLOCKED |

</div>

<div class="worksheet" data-worksheet="lab07-values" markdown>

| `ANGLE_OFFSET_DEG` | `ANGLE_REVERSED` | `D_MIN_MM` | `CAR_WIDTH_MM` |
| --- | --- | --- | --- |
| `______` | `______` | `______` | `______` |

</div>

## Step 4 · Drive

### What the car does

1. You press **START**. The car stays still for 1 second while it calibrates
   the gyro.
2. It drives forward and holds its heading with the gyro, as in Lab 4.
3. When something is in the *ahead* lane closer than `D_AVOID_MM` for two
   lidar turns in a row, it turns 45° away from it and keeps driving.
4. When the lidar shows that the original direction is clear again, it turns
   back and carries on. It ends up parallel to where it started, a little to
   one side.
5. If there is no way through, it stops and waits (**BLOCKED**). It drives on
   by itself when the path clears.
6. It stops by itself after 20 seconds.

**START and STOP:** press the **BOOT** button on the ESP32, or send `g` (go)
and `s` (stop) in the Serial Monitor.

### Set up

1. Open `ME222_Lab7_2_Drive.ino`. In **section 1**, copy in your four values
   from Step 3.
2. Check the motor pins and polarity match your Lab 3 car (left 25/26, right
   27/14 by default).
3. Upload.

### Test with the wheels raised

Put the car on a stand so the wheels are off the table.

1. Press **BOOT**. After 1 second, all four wheels turn **forward**. If a side
   turns backward, set that side's polarity to `-1`.
2. Turn the car gently to the left by hand. The left wheels should speed up
   and the right wheels slow down. If it is the other way round, set
   `YAW_SIGN = -1.0f`.
3. Hold the box 40 cm in front of the lidar. The wheels should switch to
   turning on the spot. On the stand the car cannot actually turn, so after
   4 seconds it stops with *turn took too long*. That is expected.
4. Press **BOOT** to stop, or to start again.

### Test on the floor

Unplug the USB cable. The car runs from its battery.

1. **No obstacle.** Press BOOT and let the car drive. It should go straight.
2. **One box** about 1.5 m ahead. Run it three times with the box in
   different places, and record the results.

<div class="worksheet" data-worksheet="lab07-runs" markdown>

| Run | Box position | Side the car chose | Touched the box? | Back to straight? |
| --- | --- | --- | --- | --- |
| 1 | Centre | | | |
| 2 | Mostly right | | | |
| 3 | Mostly left | | | |

</div>

### Tune

Change one value at a time in **section 2**, then upload again.

| If the car… | Change |
| --- | --- |
| Turns too late or brushes the box | Raise `D_AVOID_MM` |
| Clips the box while passing it | Raise `SIDE_MARGIN_MM` |
| Turns back toward the box too early | Raise `SIDE_MARGIN_MM` |
| Swings left and right while driving straight | Lower `KH` |
| Does not turn on the spot, or turns slowly | Raise `P_TURN` |
| Is hard to follow | Lower `P_DRIVE` |

<div class="worksheet" data-worksheet="lab07-tuning" markdown>

| `P_DRIVE` | `P_TURN` | `D_AVOID_MM` | `D_STOP_MM` | `SIDE_MARGIN_MM` | `KH` |
| --- | --- | --- | --- | --- | --- |
| `______` | `______` | `______` | `______` | `______` | `______` |

</div>

## Step 5 · Start it from ThingsBoard

The same program, now connected to the ThingsBoard server on the Raspberry Pi,
as in Lab 6.

1. Open `ME222_Lab7_3_Drive_ThingsBoard.ino`. Copy in **sections 1 and 2** from
   your tuned drive sketch.
2. Fill in the Wi-Fi password, the Raspberry Pi IP (both given in the lab) and
   your Lab 6 device token.
3. Upload. The Serial Monitor should show `ThingsBoard connected`.
4. On your laptop, open your Lab 6 dashboard. Add cards for these keys:

| Key | Shows | Unit |
| --- | --- | --- |
| `state` | READY, CALIBRATING, DRIVING, TURNING, BLOCKED, FINISHED, STOPPED | — |
| `reason` | Why the state last changed | — |
| `ahead`, `left`, `right` | Distance in each lane (300 = nothing seen) | cm |
| `heading` | Heading since START | ° |

5. Your Lab 6 **START MISSION** and **STOP** buttons work unchanged: they set
   `command` to `START` or `STOP`.
6. Press **START** on the laptop. The car calibrates for 1 second, then
   drives. Press **STOP** to stop it. The BOOT button still works too.

!!! warning "No link, no driving"
    If the car loses Wi-Fi or ThingsBoard during a run, it stops, because
    STOP could no longer reach it. It cannot start without the link either.

## Questions

1. The car waits for two full lidar turns before it turns. What would go
   wrong with one? With ten?
2. The lidar scans one flat plane at its own height. Name an obstacle it would
   miss.
3. Why must the car stop when the ThingsBoard link drops, instead of carrying
   on?

**Deliverable:** the completed tables and a run started from the ThingsBoard
dashboard.

## Troubleshooting

??? failure "`NO ANSWER from the lidar`"
    Head not spinning: check the two 5 V wires and the MOTOCTL wire on GPIO 4.
    Head spinning: the TX and RX wires are probably swapped. Lidar TX goes to
    GPIO 16, lidar RX to GPIO 17.

??? failure "`NO IMU`, or the car will not start"
    The IMU is not answering on SDA 21 / SCL 22. Check its wires. The drive
    sketches will not start without it.

??? failure "The car keeps saying *car moved: calibrating again*"
    Keep the car completely still for the first second after START.

??? failure "The car stops with *no lidar data*"
    The lidar stopped sending. Check its wiring and that the battery is on.

??? failure "The nearest point never moves and is very close"
    The lidar sees part of the car. Raise `D_MIN_MM`, or move the wire or part.

??? failure "The car does not see the box"
    Black, shiny or see-through objects return little light. Anything lower
    than the lidar head is not seen at all. Use a matte, light box that is
    taller than the lidar.

??? failure "`ThingsBoard connect failed`"
    `rc=5` means a wrong token. `rc=-2` means the ESP32 cannot reach the Pi:
    check the Wi-Fi password and the Pi IP.

## Code

??? example "ME222_Lab7_1_LidarTest.ino"
    ```cpp
    --8<-- "docs/labs/lab07/ME222_Lab7_1_LidarTest.ino"
    ```

??? example "ME222_Lab7_2_Drive.ino"
    ```cpp
    --8<-- "docs/labs/lab07/ME222_Lab7_2_Drive.ino"
    ```

??? example "ME222_Lab7_3_Drive_ThingsBoard.ino"
    ```cpp
    --8<-- "docs/labs/lab07/ME222_Lab7_3_Drive_ThingsBoard.ino"
    ```
