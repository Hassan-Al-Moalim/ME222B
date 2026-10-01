---
title: Lab 2 — Encoders, Motor Characterization & BLE
---

# Lab 2 — Encoders, Motor Characterization & Wireless (BLE) Control

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">90 minutes + report</span></div>
<div><span class="k">Depends on</span><span class="v">Lab 1 (PWM table + drift)</span></div>
<div><span class="k">Milestone</span><span class="v">PWM–speed curves + characterization table + wireless floor test</span></div>
<div><span class="k">Due</span><span class="v">Before Lab 3, one report per team on Blackboard</span></div>
</div>

## 1. Why this lab?

In [Lab 1](lab01-pwm-motors.md) you gave both sides the same PWM and the car
still drifted. PWM sets a duty cycle, not a speed. Every motor, gearbox and wheel
turns that duty cycle into a slightly different speed. Before we can control
speed (Lab 6) or drive straight (Lab 7), we need to measure what each motor
really does.

| Part | What you do |
| --- | --- |
| **1. Encoders** | Wire the wheel encoders and prove each one senses its own motor: counts, direction, counts per revolution. |
| **2. PWM sweep** | Sweep each motor on the stand and measure its speed. Find the dead zone, gain and saturation. |
| **3. BLE link** | Cut the cable: connect the ESP32 to a phone and laptop over Bluetooth Low Energy and test the automatic stop. |
| **4. Ground test** | Drive wirelessly on the floor, compare with the stand, and test a corrected PWM against the Lab 1 drift. |

The BLE link you build today is used in every lab from now on.

## 2. Background

### Incremental encoders

An incremental encoder produces pulses as the shaft turns. A quadrature encoder
has two channels, A and B, 90° out of phase. The number of pulses tells you how
far the shaft turned; which channel leads tells you the direction.

<figure class="credit" markdown>
![A rotary encoder disc turning, its two contacts opening and closing a quarter cycle apart](https://lastminuteengineers.com/wp-content/uploads/arduino/rotary-encoder-working-animation.gif){ loading=lazy width="420" }
<figcaption>
How the disc makes and breaks the two contacts as it turns.
Animation by <a href="https://lastminuteengineers.com/esp32-rotary-encoder-tutorial/">Last Minute Engineers</a>,
embedded from their site and reproduced here with credit.
</figcaption>
</figure>

- **1× decoding:** count rising edges of A only, read B for direction. Simplest; used today.
- **2× decoding:** count both edges of A.
- **4× decoding:** count every edge of A and B. Best resolution; needed later for precise positioning.

{{ svg assets/quadrature.svg }}

This is exactly what the interrupt in `part1_encoders.ino` does — on A's rising
edge it reads B, and the level it finds decides which way the count moves:

```cpp
if (digitalRead(ENC_B[m])) counts[m]--; else counts[m]++;
```

If the encoder sits on the motor shaft, before the gearbox, one wheel revolution
produces many pulses:

$$
\text{CPR}_\text{wheel} = \text{PPR}_\text{motor} \times \text{decoding factor} \times \text{gear ratio}
$$

Datasheet values are often wrong or missing, so in Part 1 you measure CPR directly.

### From PWM to speed

The H-bridge switches the battery voltage on and off. The average voltage at the
motor is roughly

$$
V_\text{avg} \approx \frac{\text{PWM}}{255} \times V_\text{battery}
$$

Below a minimum voltage the motor cannot overcome static friction and the wheel
does not turn: the **dead zone**. Above it, speed rises roughly linearly. Near
full duty the curve flattens (**saturation**), partly from driver voltage drop and
battery sag. We describe each motor with five numbers:

| Parameter | Meaning |
| --- | --- |
| `PWM_start` | Lowest PWM at which the wheel starts turning from rest (ramping up) |
| `PWM_stop` | PWM at which a turning wheel stops (ramping down). Usually lower than `PWM_start`: static vs. kinetic friction. |
| `K` | Slope of the linear region, RPM per PWM step: RPM ≈ K × (PWM − PWM_0) |
| `PWM_0` | Where the fitted line crosses the PWM axis, the effective dead zone |
| `RPM_max` | Speed at PWM 255, or where the curve flattens |

### BLE UART: a wireless serial port

BLE has no built-in serial port. A device offers *services*, each with
*characteristics* the other side can write or subscribe to. We use the **Nordic
UART Service (NUS)**, which behaves like a serial cable and is supported by many
phone apps.

| Characteristic | UUID | Direction |
| --- | --- | --- |
| Service | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` | – |
| RX (write) | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` | phone/laptop → ESP32 (commands) |
| TX (notify) | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` | ESP32 → phone/laptop (data) |

- **Packet size.** A notification carries 20 bytes by default. The code splits longer lines; the receiver joins them at the newline.
- **One connection at a time.** The car accepts the phone *or* the laptop, not both.
- **Unique names.** Each team advertises its own name (`ME222-T1` … `ME222-T5`) so you connect to your own car.

### Useful equations

| Quantity | Equation |
| --- | --- |
| Wheel revolutions | rev = Δcounts / CPR |
| Wheel speed | RPM = (Δcounts / CPR) × (60 / Δt) |
| Linear speed | v = RPM × π × D / 60  (D = wheel diameter, m) |
| Distance | d = (counts / CPR) × π × D |
| Left/right mismatch | (RPM_L − RPM_R) / ((RPM_L + RPM_R) / 2) × 100 % |
| Feed-forward PWM | PWM = PWM_0 + RPM_target / K |

### Further reading

- [ESP32 Rotary Encoder tutorial](https://lastminuteengineers.com/esp32-rotary-encoder-tutorial/) — Last Minute Engineers. Animated walk-through of how a rotary encoder produces its two signals, with a worked ESP32 example. Covers a detented knob encoder rather than a motor encoder, but the quadrature idea is identical.

## 3. Pre-lab

1. Measure your wheel diameter D to the nearest mm.
2. Find the motor/encoder model on your car. Write down the datasheet PPR and gear ratio if available, and compute the expected CPR for 1× decoding.
3. If CPR = 700 and you count 350 pulses in 100 ms, what is the wheel RPM? What is the linear speed for D = 65 mm?
4. Why must the encoder counter be declared `volatile`, and why do we copy it with interrupts disabled?

## 4. Code

One sketch per part. Each is complete on its own and adds one feature to the
previous one, so you always know which new code you are testing. **Download them,
do not retype them.**

| File | Part | New in this sketch | Commands |
| --- | --- | --- | --- |
| [`part1_encoders.ino`](lab02/part1_encoders.ino) | 1 | Encoder interrupts, motor driving, live counts and RPM | `c` `z` `p` `m<L>,<R>` `x` |
| [`part2_sweep.ino`](lab02/part2_sweep.ino) | 2 | Speed measurement, PWM sweep, dead-zone search, STOP during a test | adds `s<m>` `f<m>` |
| [`part3_ble.ino`](lab02/part3_ble.ino) | 3 | BLE UART link, output over BLE, disconnect failsafe | `c` `z` `m<L>,<R>` `x` over BLE |
| [`part4_floor_ble.ino`](lab02/part4_floor_ble.ino) | 4 | Complete sketch: BLE + sweep + dead zone + floor run | all, plus `r<pwm>` `r<L>,<R>` |

In every sketch the arrays in the configuration block list the four motors in
the order **0 = front-left, 1 = front-right, 2 = rear-left, 3 = rear-right**. The
same index is used everywhere: motor `m`, encoder `m`, `CPR[m]`. Edit only the
configuration block: pins, CPR, wheel diameter, BLE name.

## 5. Procedure

### Part 1 — Encoders: each encoder senses its own motor (25 min)

**Goal:** every encoder is wired, counts in the correct direction, belongs to
the right motor in the code, and has a measured CPR.

#### Step 1.1 — Wire the encoders (battery disconnected)

- Encoder VCC to 3.3 V, GND to the common ground. If your encoder needs 5 V, its outputs must go through a level shifter or voltage divider.
- Keep encoder wires short and away from the motor power leads. Twist each encoder cable with its ground if possible.

#### Step 1.2 — Upload and read the counts

1. Open `part1_encoders.ino`, set the pin numbers to your wiring, leave CPR at 700 for now.
2. Upload over USB. Serial Monitor at **115200 baud, line ending Newline**. You should see `Part 1 - encoder test`.
3. Send `c`: one line with four counts, e.g. `counts: 0 0 0 0`.
4. Turn each wheel slowly by hand, one at a time, and send `c` after each. **Only that wheel's count should change.** If the wrong number changes, your `ENC_A`/`ENC_B` pins are swapped between motors in the code.
5. Turn a wheel forward, then backward: the count must go up, then down.

??? failure "A count never changes"
    Check encoder power, the pull-up on the A line, and the pin number, in that order.

#### Step 1.3 — Check with the motors running (car on the stand)

1. Connect the battery to the motor drivers. Car on the stand.
2. Send `z` (reset), `p` (live view every 250 ms), then `m100,100`. Watch a few seconds, then send `x` and `p`.
3. All four counts must increase and all four RPM values must be positive and roughly similar.
4. Repeat until `m100,100` gives four positive counts and all wheels turn forward.

??? failure "One count goes negative while the wheel turns forward"
    A and B are swapped for that encoder. Swap the two wires, or the two pin
    numbers in the code.

??? failure "A wheel turns backwards on m100,100"
    The motor itself is reversed. Swap its `IN1`/`IN2` pins in the code, or its
    motor wires.

#### Step 1.4 — Measure CPR

1. Put a tape mark on the wheel and a reference mark on the chassis. Send `z`.
2. Turn the wheel by hand exactly **10 full revolutions** forward. Send `c` and record the count.
3. CPR = count / 10. Repeat once; the two runs should agree within 1–2 %.
4. Do this for every encoder and compare with your pre-lab value.
5. Write down the CPR values and `WHEEL_D`. You enter them in the Part 2, 3 and 4 sketches.

<div class="worksheet" data-worksheet="lab02-cpr" markdown>

| Encoder | Run 1 counts | Run 2 counts | CPR (avg) | Expected CPR |
| --- | --- | --- | --- | --- |
| Front-left (0) | `____` | `____` | `____` | `____` |
| Front-right (1) | `____` | `____` | `____` | `____` |
| Rear-left (2) | `____` | `____` | `____` | `____` |
| Rear-right (3) | `____` | `____` | `____` | `____` |

</div>

### Part 2 — PWM sweep and dead zone (30 min, stand, USB)

**Goal:** the PWM–speed curve of every motor, plus its exact start and stop PWM.

#### Step 2.1 — Automatic sweep

`s<m>` sweeps motor `m`: PWM 0 → 255 in steps of 15, then back down. At each
step it waits 0.5 s to settle, averages speed over 1.5 s and prints one CSV
line. One sweep takes about **72 s**.

1. Disconnect the battery. Open `part2_sweep.ino`, enter pins, CPR values and `WHEEL_D`, upload. Reconnect the battery, car on the stand.
2. Measure and record the battery voltage. It must be above **11.1 V**.
3. Clear the Serial Monitor, send `s0` (front-left).
4. When `sweep done` appears, copy the lines from the header `motor,dir,pwm,…` to the last data line into `motor0_unloaded.csv`.
5. Repeat with `s1`, `s2`, `s3`.
6. **Quick check:** plot one motor before moving on. The curve should be flat at zero, then rise, then flatten near the top. Up and down curves should be close except near the dead zone.

CSV columns: `motor, dir (up/down), pwm, rpm, speed_mps, counts`.

#### Step 2.2 — Dead zone and hysteresis

Steps of 15 are too coarse for the dead zone. `f<m>` ramps up in steps of 2
(every 300 ms) until the wheel exceeds 5 RPM (`PWM_start`), then ramps down from
150 until it stops (`PWM_stop`).

Send `f0`, `f1`, `f2`, `f3`. Repeat each three times and record the averages.

<div class="worksheet" data-worksheet="lab02-deadzone" markdown>

| Motor | PWM_start (avg) | PWM_stop (avg) | Hysteresis | Notes |
| --- | --- | --- | --- | --- |
| Front-left (0) | `____` | `____` | `____` | |
| Front-right (1) | `____` | `____` | `____` | |
| Rear-left (2) | `____` | `____` | `____` | |
| Rear-right (3) | `____` | `____` | `____` | |

</div>

!!! checkpoint "Checkpoint 2 (TA sign-off)"
    Four CSV files, one quick plot, and the dead-zone table complete.

!!! question "Discussion"
    Why is `PWM_start` higher than `PWM_stop`? What does this mean for a
    controller that must drive very slowly, for example when approaching a
    delivery point?

### Part 3 — BLE serial link (15 min, car still on the stand)

**Goal:** the same commands work without the cable, and the car stops by itself
if the link is lost. The Part 3 sketch is Part 1 with BLE added; only the
communication changes.

1. Disconnect the battery. Open `part3_ble.ino`, set `BLE_NAME` to your team name (e.g. `ME222-T3`) and your pins, upload over USB. The Serial Monitor shows `BLE ready: ME222-T3`.
2. Unplug the USB cable.
3. On the phone: **nRF Toolbox → UART → Connect**, select your team name.
4. Send `c`: the counts appear in the app. Send `m100,100`, then `x`. Same behaviour as over USB.
5. **Failsafe test:** send `m120,120`, then turn off Bluetooth on the phone. The wheels must stop within about 1 s. Turn Bluetooth back on and reconnect: the car must accept commands again.
6. Disconnect the phone. On the laptop run `python ble_logger.py ME222-Tx test` and type `c`: the line appears on the laptop.

??? failure "The ESP32 resets, and the link drops, every time the motors start"
    The 5 V supply is sagging. Check the buck converter, its input wiring and
    the common ground.

!!! checkpoint "Checkpoint 3 (TA sign-off)"
    Show: (1) wireless drive and `x` from the phone, (2) the car stops when
    Bluetooth is switched off, (3) the laptop logger receives data.

### Part 4 — Ground test, fully wireless (15 min)

**Goal:** see how the motors behave under load, and whether your measured
correction reduces the Lab 1 drift. With no cable pulling the car, the drift you
measure is the real drift.

#### Step 4.1 — Equal-PWM runs

1. Car on the stand, battery disconnected: plug in USB, open `part4_floor_ble.ino`, enter `BLE_NAME`, pins, CPR and `WHEEL_D`, upload. Unplug USB, reconnect the 5 V buck and the battery. Check the phone still connects and `x` works.
2. Mark a straight 3 m track with masking tape: start line, 1 m, 2 m, 3 m.
3. Start the logger with one file per run: `python ble_logger.py ME222-Tx floor_120`.
4. Put the car on the start line and send `r120`. All four motors run at PWM 120 for 3 s, each wheel's RPM is logged every 100 ms, and the encoder distance of each wheel is printed at the end.
5. Measure the real distance with the tape and the lateral drift at the end (cm left or right of the line).
6. Repeat with `r180`, and `r240` if the track is long enough. The stop person keeps `x` ready in the logger.

#### Step 4.2 — Corrected run (feed-forward)

1. Pick a target speed, e.g. the average wheel RPM you measured in the r150 range. Using your Part 2 fits (average the two motors on each side), compute PWM_L = PWM_0,L + RPM_target / K_L, and PWM_R likewise.
2. Run `r<PWM_L>,<PWM_R>`, for example `r150,138`. Measure the drift again and compare with the equal-PWM run and with Lab 1.

<div class="worksheet" data-worksheet="lab02-floor" markdown>

| Run | RPM FL | RPM FR | RPM RL | RPM RR | Encoder dist (m) | Drift (cm) |
| --- | --- | --- | --- | --- | --- | --- |
| r120 | `____` | `____` | `____` | `____` | `____` | `____` |
| r180 | `____` | `____` | `____` | `____` | `____` | `____` |
| r240 | `____` | `____` | `____` | `____` | `____` | `____` |
| r`__`,`__` | `____` | `____` | `____` | `____` | `____` | `____` |

</div>

!!! checkpoint "Checkpoint 4 (TA sign-off)"
    Floor table complete, including one corrected run. State in one sentence
    whether the correction reduced the drift.

## 6. How the code works

### Part 1 — `part1_encoders.ino`

| Command | Action |
| --- | --- |
| `c` | Print the four encoder counts |
| `z` | Reset all counts to zero |
| `p` | Live view on/off: counts and RPM of each wheel every 250 ms |
| `m100,100` | Drive left side, then right side, at −255…255. Type the numbers directly — no angle brackets and no spaces: `m100,100` forward, `m100,-100` spin, `m-100,-100` reverse |
| `x` | STOP all motors |

**Configuration.** Pin arrays for the enable (PWM) and direction pins of each
motor, and for channels A and B of each encoder. PWM runs at 20 kHz, above
hearing so the motors do not whine, with 8-bit resolution (0–255).

**Encoder interrupt (`encISR`).** Called on every rising edge of channel A,
whatever the main program is doing. It reads B: low means forward and the count
goes up; high means it goes down. It is marked `IRAM_ATTR` so it runs from fast
internal RAM, and it is kept very short. One routine serves all four encoders;
`attachInterruptArg()` passes it the motor number.

**Safe reading (`readCount`, `resetCounts`).** `counts[]` is `volatile` because
it changes inside an interrupt, so the compiler always reads real memory.
`readCount()` disables interrupts while copying so the value cannot change
mid-copy.

**Motor control (`setMotor`, `stopAll`).** Positive sets IN1 high and IN2 low
(forward), negative reverses, zero sets both low. The magnitude goes to the
enable pin through `ledcWrite()`, the ESP32 hardware PWM.

**Live view.** Every 250 ms prints the counts and RPM = (Δcounts / CPR) × (60 / Δt).

**Commands.** `loop()` reads a line and passes it to `handle()`, which switches on
the first letter. For `m` the text is split at the comma. Left motors are 0 and
2, right motors 1 and 3.

??? example "part1_encoders.ino"
    ```cpp
    --8<-- "docs/labs/lab02/part1_encoders.ino"
    ```

### Part 2 — `part2_sweep.ino`

Same encoder and motor code, plus measurement. New commands `s<m>` and `f<m>`;
`x` stops a running test.

**STOP during a test (`pollStop`, `waitMs`).** `delay()` cannot be interrupted
and a sweep takes 72 s. `waitMs()` waits in 2 ms slices and checks for `x` in
each; if found it stops the motors and returns `false` so the test ends.

**Measuring speed (`measureRPM`).** Reads the count, waits a window, reads it
again, and uses the real elapsed time. A longer window is smoother but slower:
1.5 s in the sweep, 0.1 s in the dead-zone search.

**Sweep.** Up then down, 18 steps of 15. Each step: set PWM, settle 0.5 s,
measure 1.5 s, print one CSV line. Sweeping both ways shows hysteresis.

**Dead zone.** Ramps up in steps of 2 until the wheel exceeds 5 RPM
(`PWM_start`), spins it at 150, then ramps down until it drops below 5 RPM
(`PWM_stop`).

??? example "part2_sweep.ino"
    ```cpp
    --8<-- "docs/labs/lab02/part2_sweep.ino"
    ```

### Part 3 — `part3_ble.ino`

Commands `c`, `z`, `m`, `x` now arrive over BLE and every answer goes back over
BLE. USB Serial still works for debugging.

**Set-up (`setupBLE`).** Creates the Nordic UART Service with TX (notify, with a
`BLE2902` descriptor so the phone can subscribe) and RX (write), then advertises
it under the team name.

**Output (`blePrintf`).** Works like `printf`: prints to USB and, if connected,
sends BLE notifications 20 bytes at a time with a 4 ms pause so the stack is not
flooded.

**Receiving (`RxCB::onWrite`).** Runs in the BLE stack, so it does not execute
the command; it copies the text into `cmdBuf` and `loop()` runs it. The exception
is `x`, which stops the motors immediately.

**Failsafe (`ServerCB::onDisconnect`).** When the link is lost, this stops all
motors and restarts advertising so you can reconnect. It is the most important
safety feature of a wireless robot. **Never remove it.**

??? example "part3_ble.ino"
    ```cpp
    --8<-- "docs/labs/lab02/part3_ble.ino"
    ```

### Part 4 — `part4_floor_ble.ino`

The complete program: Part 3's link and failsafe, Part 2's measurement, and the
floor run. You keep using it in later labs.

| Command | Action |
| --- | --- |
| `c`, `z`, `m<L>,<R>`, `x` | As in Parts 1 and 3 |
| `s<m>`, `f<m>` | Sweep and dead zone, now over BLE |
| `r<pwm>` | Floor run: all motors at the same PWM for 3 s |
| `r<L>,<R>` | Floor run with different left and right PWM |

**STOP over BLE.** `x` arrives through the BLE callback, which sets `stopReq`;
`waitMs()` only checks the flag. A disconnect also sets it, so a sweep or floor
run ends if the link drops.

**Floor run.** Resets the counts, sets the left pair to L and the right pair to
R, and every 100 ms prints time and each wheel's RPM. After 3 s it stops and
prints each wheel's distance, d = counts / CPR × π × D.

??? example "part4_floor_ble.ino"
    ```cpp
    --8<-- "docs/labs/lab02/part4_floor_ble.ino"
    ```

### Phone apps

- **nRF Toolbox** (Android, iPhone; recommended): UART → Connect → your team name. Set up macro buttons with `x` first, then `c`, `m100,100`, `m100,-100`.
- **Bluefruit Connect** (Android, iPhone): select your team name → UART. Do not use its Controller game pad; it sends a different format.
- **Serial Bluetooth Terminal** (Android only): ☰ → Devices → Bluetooth LE → Scan → your team → connect. Long-press M1 and set it to `x`.

## 7. Analysis and report

One short report per team, max. 6 pages plus an appendix with the CSV files.

1. **Encoder verification (Part 1):** CPR table, measured vs. expected, and the problems you found and fixed.
2. **PWM–speed curves (Part 2):** one plot of RPM vs. PWM with all four motors (up-sweep), dead zone and saturation marked. One plot with up and down sweeps of one motor.
3. **Linear fit:** for each motor fit RPM = K × (PWM − PWM_0) over the linear region only. Report K, PWM_0 and R².
4. **Motor characterization table** (below).
5. **Wireless link (Part 3):** in two or three sentences, what happens when the link drops and why it matters for a delivery robot.
6. **Loaded vs. unloaded (Part 4):** RPM at the same PWM on the stand and on the floor, and encoder distance vs. tape distance (odometry error in %).
7. **Correction:** drift with equal PWM vs. corrected PWM vs. Lab 1. Is a fixed correction enough? Why will we still need feedback in Lab 6?

<div class="worksheet" data-worksheet="lab02-characterization" markdown>

| Motor | CPR | PWM_start | PWM_stop | PWM_0 (fit) | K (RPM/PWM) | RPM_max | Mismatch % |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Front-left | `__` | `__` | `__` | `__` | `__` | `__` | `__` |
| Front-right | `__` | `__` | `__` | `__` | `__` | `__` | `__` |
| Rear-left | `__` | `__` | `__` | `__` | `__` | `__` | `__` |
| Rear-right | `__` | `__` | `__` | `__` | `__` | `__` | `__` |

</div>

## Backup method without encoders

Use this only if your car has no working encoders.

**Unloaded RPM with slow-motion video.** Put a bright tape mark on the wheel.
Run the motor on the stand with `m<L>,<R>` at PWM 60, 90, 120, 150, 180, 210,
255. Record 5 s of slow-motion video (240 fps) at each and count full
revolutions: RPM = revolutions / time × 60.

**Loaded speed with timed runs.** Drive the 3 m track at PWM 120, 180 and 240.
Time the run between the 1 m and 3 m marks: v = 2 m / t. Record the lateral
drift at 3 m as in Part 4. Find the dead zone by eye with `m` in steps of 2.
