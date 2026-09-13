---
title: Schedule
---

# Schedule

Ten weeks, one 90-minute lab per week.

!!! warning "Dates are placeholders"
    Week dates are `TODO`. Set them once here — no other page hard-codes a date.

**Legend** — <span class="pill open">open</span> released ·
<span class="pill soon">soon</span> not yet released ·
<span class="pill closed">closed</span> past due

| Week | Date | Lab | 90-minute practical focus | Milestone / deliverable | Status |
| :--: | --- | --- | --- | --- | :--: |
| 1 | TODO | [PWM, Four Motors & Motor Mismatch](labs/lab01-pwm-motors.md) | First full motion lab. Run four motors; introduce PWM; test PWM 100/150/200/255; find minimum useful PWM; drive ~4 m with equal left/right PWM; measure drift; try one manual correction | Working car + PWM table + measured straight-line drift | <span class="pill open">open</span> |
| 2 | TODO | [Motor Characterization & Encoders](labs/lab02-motor-characterization.md) | Measure actual wheel speed; encoder counts; RPM; test several PWM values; compare left/right sides; identify dead zone and saturation | PWM–speed curves + left/right motor characterization | <span class="pill soon">soon</span> |
| 3 | TODO | [LiDAR / ToF Distance Sensing](labs/lab03-distance-sensing.md) | Mount distance sensor; measure known distances; determine useful range and field of view; implement basic obstacle detection | Reliable `obstacleDetected()` function + sensor test data | <span class="pill soon">soon</span> |
| 4 | TODO | [IMU + Sensor Calibration](labs/lab04-imu-calibration.md) | Read accelerometer/gyro; observe bias and noise; basic heading/turn measurement; calibrate IMU or distance sensor against known references | Calibration curve + corrected sensor readings | <span class="pill soon">soon</span> |
| 5 | TODO | [Logging, Filtering & Sensor-Based Motion](labs/lab05-logging-filtering.md) | Log PWM, encoder, distance and IMU data; compare raw/filtered readings; moving average/median filter; make car slow as obstacle distance decreases | Raw-vs-filtered plots + `speed = f(distance)` behaviour | <span class="pill soon">soon</span> |
| 6 | TODO | [Feedback Control — P / PI / PID](labs/lab06-feedback-control.md) | Now introduce feedback. Use encoder speed or measured stopping distance; desired vs actual value; P first, then PI/PID; experimental tuning | Closed-loop speed or stopping controller with measured error | <span class="pill soon">soon</span> |
| 7 | TODO | [Straight Driving, Turns & Localization](labs/lab07-straight-turns-localization.md) | Use encoder + IMU feedback; correct the Week 1 drift problem; drive fixed distance; perform 90° turns; estimate position/heading between known points | Repeatable straight run + controlled turn + simple localization | <span class="pill soon">soon</span> |
| 8 | TODO | [Payload Servo + ThingsBoard](labs/lab08-servo-thingsboard.md) | Add simple servo release mechanism; connect car to ThingsBoard; report vehicle state; receive pickup/destination; implement mission states | Dashboard sends mission → car executes → payload release | <span class="pill soon">soon</span> |
| 9 | TODO | [Multi-Car Coordination](labs/lab09-multi-car.md) | Run at least two cars; local obstacle safety; simple zone reservation / priority; test crossing and narrow-corridor cases | Two-car coordinated demonstration with no contact | <span class="pill soon">soon</span> |
| 10 | TODO | [Final Autonomous Fleet Challenge](labs/lab10-fleet-challenge.md) | Full pickup → navigation → obstacle handling → delivery → payload release → telemetry; multiple cars operating in shared arena | Final fleet demonstration + performance results | <span class="pill soon">soon</span> |

## Notes on the sequence

**Week 1 poses the problem; Week 7 solves it.** The drift measured in the first
session is the thread running through the whole course. Week 7 re-runs the exact
Week 1 test so students can see their own improvement as a number.

**Week 6 is the hinge.** Everything before it is open loop — command and hope.
Everything after it is closed loop. Say this explicitly in the session.

**Week 8 has an infrastructure dependency.** ThingsBoard access and WiFi that an
ESP32 can actually join must be confirmed *before* the session. See the
[Lab 8 page](labs/lab08-servo-thingsboard.md) for the checklist.

## Office hours

| Who | When | Where |
| --- | --- | --- |
| TODO — instructor | TODO | TODO |
| TODO — TA | TODO | TODO |

## Open lab time

TODO — if the lab is available outside scheduled sessions, say when and under
what supervision. With 90-minute sessions, teams will need it.
