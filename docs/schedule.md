---
title: Schedule
---

# Schedule

**Fall 2026/2027** · Semester runs **30 August – 10 December 2026**

Lectures: **Sunday & Wednesday, 08:30–10:00**, Building 9 – Classroom 4120.

## Lecture schedule

| Week | Dates | Topic |
| :--: | --- | --- |
| 1 | Sun 30 Aug · Wed 2 Sep | Course introduction and thermal sensors |
| 2 | Sun 6 Sep · Wed 9 Sep | Prototyping of sensor/actuator based systems |
| 3 | Sun 13 Sep · Wed 16 Sep | Flow and rotary sensors |
| 4 | Sun 20 Sep | Amplifiers and sensor noise |
| 5 | Sun 27 Sep · Wed 30 Sep | AC motor designs |
| 6 | Sun 4 Oct · Wed 7 Oct | AC motor controls |
| 7 | Sun 11 Oct · Wed 14 Oct | DC and stepper motors |
| 8 | Sun 18 Oct · Wed 21 Oct | **Semester break** |
| 9 | Sun 25 Oct · Wed 28 Oct | Pressure sensors |
| 10 | Sun 1 Nov · Wed 4 Nov | Force and strain sensors |
| 11 | Sun 8 Nov · Wed 11 Nov | Position sensors |
| 12 | Sun 15 Nov · Wed 18 Nov | Motion sensors |
| 13 | Sun 22 Nov · Wed 25 Nov | Systems development: embedded architecture trade-offs, standards, engineering best practices, CPU architectures |
| 14 | Sun 29 Nov · Wed 2 Dec | ARM architecture and demo, appliance case study, processor selection and benchmarking |
| 15 | Sun 6 Dec · Wed 9 Dec | Embedded operating systems and tools |
| 16 | — | **Final exam week** |

!!! note "Week 4 has one session"
    The syllabus lists only Sunday 20 September for Week 4 — no Wednesday
    session. TODO — instructor: confirm whether Wed 23 Sep is cancelled
    (it falls on Saudi National Day) and say so here.

## Lab schedule

Ten labs, one 90-minute session per week, building a four-motor autonomous car.

!!! warning "Lab dates not yet set"
    Lab sessions are not in the official syllabus. TODO — instructor: set the
    day, time, room, and which semester weeks the ten labs occupy. Week 8 is the
    semester break, and Week 16 is final exams, so ten labs fit weeks 1–7 and
    9–11, or 2–7 and 9–12.

    See also the [note on the two lab plans](syllabus.md#planned-labs) — this
    sequence differs from the one printed in the syllabus.

**Legend** — <span class="pill open">open</span> released ·
<span class="pill soon">soon</span> not yet released

| Lab | Date | Practical focus | Milestone / deliverable | Status |
| :--: | --- | --- | --- | :--: |
| [1](labs/lab01-pwm-motors.md) | TODO | First full motion lab. Run four motors; introduce PWM; test PWM 100/150/200/255; find minimum useful PWM; drive ~4 m with equal left/right PWM; measure drift | Working car + PWM table + measured straight-line drift | <span class="pill open">open</span> |
| [2](labs/lab02-motor-characterization.md) | TODO | Measure actual wheel speed; encoder counts; RPM; test several PWM values; compare left/right sides; identify dead zone and saturation | PWM–speed curves + left/right characterization | <span class="pill soon">soon</span> |
| [3](labs/lab03-distance-sensing.md) | TODO | Mount distance sensor; measure known distances; determine useful range and field of view; implement basic obstacle detection | Reliable `obstacleDetected()` + sensor test data | <span class="pill soon">soon</span> |
| [4](labs/lab04-imu-calibration.md) | TODO | Read accelerometer/gyro; observe bias and noise; basic heading/turn measurement; calibrate against known references | Calibration curve + corrected sensor readings | <span class="pill soon">soon</span> |
| [5](labs/lab05-logging-filtering.md) | TODO | Log PWM, encoder, distance and IMU data; compare raw/filtered; moving average/median filter; car slows as obstacle nears | Raw-vs-filtered plots + `speed = f(distance)` | <span class="pill soon">soon</span> |
| [6](labs/lab06-feedback-control.md) | TODO | Now introduce feedback. Encoder speed or stopping distance; desired vs actual; P first, then PI/PID; experimental tuning | Closed-loop controller with measured error | <span class="pill soon">soon</span> |
| [7](labs/lab07-straight-turns-localization.md) | TODO | Encoder + IMU feedback; correct the Lab 1 drift problem; drive fixed distance; 90° turns; estimate position/heading | Repeatable straight run + controlled turn + localization | <span class="pill soon">soon</span> |
| [8](labs/lab08-servo-thingsboard.md) | TODO | Servo release mechanism; connect car to ThingsBoard; report vehicle state; receive pickup/destination; mission states | Dashboard sends mission → car executes → payload release | <span class="pill soon">soon</span> |
| [9](labs/lab09-multi-car.md) | TODO | Run at least two cars; local obstacle safety; zone reservation / priority; crossing and narrow-corridor cases | Two-car coordinated demonstration with no contact | <span class="pill soon">soon</span> |
| [10](labs/lab10-fleet-challenge.md) | TODO | Full pickup → navigation → obstacle handling → delivery → payload release → telemetry; multiple cars in shared arena | Final fleet demonstration + performance results | <span class="pill soon">soon</span> |

### Notes on the lab sequence

**Lab 1 poses the problem; Lab 7 solves it.** The drift measured in the first
session is the thread running through the whole sequence. Lab 7 re-runs the exact
Lab 1 test so you can see your own improvement as a number.

**Lab 6 is the hinge.** Everything before it is open loop — command and hope.
Everything after it is closed loop.

**Lab 8 has an infrastructure dependency.** ThingsBoard access and WiFi an ESP32
can actually join must be confirmed before the session. See the
[Lab 8 page](labs/lab08-servo-thingsboard.md).

## Assessment dates

| Item | Weight | Date |
| --- | --: | --- |
| Midterm exam | 20 % | TODO |
| Final exam | 20 % | Week 16 — TODO exact date |
| Course project(s) | 45 % | TODO — two projects, milestones |
| Quizzes | 10 % | TODO |

## Office hours

| Who | When | Where |
| --- | --- | --- |
| Amr Talaat Abdel Hamid | TODO | TODO |
| TA — TODO | TODO | TODO |

The syllabus does not list office hours or an office location. TODO — instructor:
fill these in; they are the most-asked question of the first two weeks.
