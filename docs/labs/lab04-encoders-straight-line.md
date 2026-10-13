---
title: Lab 4 — Car Calibration
---

# Lab 4 — Car Calibration

<div class="lab-meta" markdown>
<div><span class="k">Date</span><span class="v">Wednesday 14 October</span></div>
<div><span class="k">Depends on</span><span class="v">Lab 3 + IMU homework controller</span></div>
<div><span class="k">Milestone</span><span class="v">Encoder distance scale + F vs. C straight-line comparison</span></div>
</div>

**Goal:** measure wheel movement, remove the gyro offset, and check whether IMU
feedback improves straight driving.

## Setup

- Use four motors and **two encoders**, one on each side. Both left motors share the left PWM/DIR command; both right motors share the right one.
- Record which wheel each encoder measures.
- Bring the car, mounted IMU, battery, ESP32 and USB cable, existing firmware, phone with UART buttons, and a tape measure.
- Keep last week's motor wiring. IMU: SDA GPIO21, SCL GPIO22. Use unused suitable inputs for the encoders, with 3.3 V compatible signals. GPIO34–39 have no internal pull-ups.
- Turn motor drive off while wiring; raise the wheels for the first tests.

**Definitions.** PWM sets motor duty (drive level). DIR sets direction. An
encoder produces pulses as a wheel or shaft rotates. An IMU contains motion
sensors; its gyroscope measures turning rate.

## Task 1 · Check and calibrate the encoders

1. With motors off, rotate the left measured wheel. Confirm only its count changes. Repeat on the right. Record the wheel location and whether each encoder has one channel or A/B channels.
2. Mark each measured wheel. Turn it through **five full revolutions** and record the counts. Repeat once to check consistency. Measure each wheel diameter.
3. Calculate C and q with the equations below. Then roll the car straight over **1 m** and compare the encoder distance with the tape measure.

$$ q = \frac{\pi D}{C} \qquad s = q\,n $$

| Symbol | Meaning / unit |
| --- | --- |
| n | Counts during the measurement (absolute count change) |
| C | Counts per complete wheel turn, using your firmware's counting method |
| D | Wheel diameter, m |
| q | Wheel distance per count, m/count |
| s | Estimated wheel travel, m |

Calculate left and right separately.

<div class="worksheet" data-worksheet="lab04-encoders" markdown>

| Side / wheel | Counts over 5 turns (twice) | D (m) | C | q (m/count) | 1 m roll estimate (m) |
| --- | --- | --- | --- | --- | --- |
| Left | `____` | `____` | `____` | `____` | `____` |
| Right | `____` | `____` | `____` | `____` | `____` |

</div>

## Task 2 · Calibrate the gyro and check correction

1. Stop the motors and keep the car still for 2–3 s. Average the yaw gyro readings to find the bias b. Reject the calibration if the car moves or sensor reads fail.
2. Subtract the bias from each reading. While still, the corrected rate should be near zero. Rotate the car left by hand and check that corrected yaw is **positive**.
3. Use your homework controller. With the wheels raised, start `C` and gently rotate the chassis left. Left duty should increase and right duty decrease to oppose the turn. **Stop immediately if the correction reinforces the turn.**

### Gyro equations

$$ b = \frac{g_1 + g_2 + \dots + g_N}{N} \qquad \omega = g - b \qquad \theta_\text{new} = \theta_\text{old} + \omega\,\Delta t $$

| Symbol | Meaning / unit |
| --- | --- |
| g | Measured yaw gyro reading, rad/s |
| b | Gyro bias: average reading while the car is still, rad/s |
| N | Number of valid stationary samples |
| ω | Corrected turning rate, rad/s |
| θ | Change in heading since the start of the run, rad |
| Δt | Actual time between updates, s |

Yaw is turning left/right about the vertical axis. One radian is about 57.3°.
Reset θ to zero at the start of every `C` run. Use the same units throughout.

### Homework controller reminder

$$ u = \operatorname{clip}(K_h\theta + K_r\omega,\ -u_{max},\ +u_{max}) $$

$$ P_L = \operatorname{clip}(P_0 + u, 0, 255) \qquad P_R = \operatorname{clip}(P_0 - u, 0, 255) $$

| Symbol | Meaning |
| --- | --- |
| P₀ | Base PWM duty for both sides, 0–255 |
| P_L, P_R | Final left and right PWM duties |
| u | Duty correction added on the left and subtracted on the right |
| K_h | Heading gain, PWM/rad |
| K_r | Turning-rate gain, PWM/(rad/s) |
| u_max | Largest permitted correction; start with 30 PWM counts |
| clip(x, a, b) | Keep x between lower limit a and upper limit b |

Update locally every 10–20 ms using the actual Δt. Keep BLE display updates at
1 Hz. Preserve Stop, disconnect and the three-second timeout. A failed IMU read
or a stale controller must stop `C`. Keep both motors on each side at the same
duty.

<div class="worksheet" data-worksheet="lab04-gains" markdown>

| b (rad/s) | P₀ | K_h | K_r | Control interval (ms) | Stale limit (ms) | u_max |
| --- | --- | --- | --- | --- | --- | --- |
| `____` | `____` | `____` | `____` | `____` | `____` | `____` |

</div>

## Task 3 · Compare straight driving

1. Mark one start position and a straight reference line. Use a clear floor and low speed. Keep the same base PWM, surface and battery condition.
2. Alternate two `F` runs and two `C` runs. Each run lasts three seconds. Return the car to the same start position and heading each time.
3. Measure forward distance x and sideways displacement y. Complete the table and explain whether `C` improved the result.

`F` = forward without IMU correction. `C` = forward with IMU correction. `S` = stop.

<div class="worksheet" data-worksheet="lab04-runs" markdown>

| Run | Forward distance x (cm) | Sideways displacement y (cm) | Observation |
| --- | --- | --- | --- |
| F1 | `____` | `____` | |
| C1 | `____` | `____` | |
| F2 | `____` | `____` | |
| C2 | `____` | `____` | |

</div>

### Simple comparison

$$ \text{Mean forward distance} = \frac{x_1 + x_2}{2} \qquad E = \frac{|y_1| + |y_2|}{2} $$

x is distance along the starting direction; y is sideways displacement from the
reference line; |y| ignores left/right sign; E is the average sideways error.
Calculate each mean separately for F and C.

<div class="worksheet" data-worksheet="lab04-compare" markdown>

| Result | F: baseline | C: corrected |
| --- | --- | --- |
| Mean forward distance (cm) | `____` | `____` |
| Mean sideways error E (cm) | `____` | `____` |

</div>

A smaller E suggests less drift. Check forward distance too: a car that travels
less may also drift less. The IMU estimates heading; it does not measure
sideways position. Two encoders measure only their two wheels.

**Answer briefly:** Did `C` improve straight driving? What would you change next?
