# ME 222 — Robotics Lab

Course materials for ME 222. Everything you need for the labs lives here, and is
published as a website:

### 📘 **[hassan-al-moalim.github.io/ME222B](https://hassan-al-moalim.github.io/ME222B/)**

Read the labs on the website — it has search, and the code blocks have copy
buttons. This repository is just where the source text lives.

---

## The course

Ten weeks, one 90-minute lab per week. You build a four-motor autonomous robot
car on an ESP32, then make a fleet of them work together.

Week 1 hands you a problem you cannot yet solve: send the **same** PWM command to
both sides of the car and it still curves away from the line. Everything through
Week 6 builds the measurement and control tools you need. **Week 7 is where you
fix it** — by re-running the exact Week 1 test and beating your own number.

## Labs

| Week | Lab | Milestone |
| :--: | --- | --- |
| 1 | [PWM, Four Motors & Motor Mismatch](docs/labs/lab01-pwm-motors.md) | Working car + PWM table + measured drift |
| 2 | [Motor Characterization & Encoders](docs/labs/lab02-motor-characterization.md) | PWM–speed curves, dead zone, saturation |
| 3 | [LiDAR / ToF Distance Sensing](docs/labs/lab03-distance-sensing.md) | Reliable `obstacleDetected()` + sensor data |
| 4 | [IMU + Sensor Calibration](docs/labs/lab04-imu-calibration.md) | Calibration curve + corrected readings |
| 5 | [Logging, Filtering & Sensor-Based Motion](docs/labs/lab05-logging-filtering.md) | Raw-vs-filtered plots + `speed = f(distance)` |
| 6 | [Feedback Control — P / PI / PID](docs/labs/lab06-feedback-control.md) | Closed-loop controller with measured error |
| 7 | [Straight Driving, Turns & Localization](docs/labs/lab07-straight-turns-localization.md) | Repeatable straight run + controlled turn |
| 8 | [Payload Servo + ThingsBoard](docs/labs/lab08-servo-thingsboard.md) | Dashboard sends mission → car executes |
| 9 | [Multi-Car Coordination](docs/labs/lab09-multi-car.md) | Two-car demonstration with no contact |
| 10 | [Final Autonomous Fleet Challenge](docs/labs/lab10-fleet-challenge.md) | Fleet demonstration + performance results |

## Before your first session

1. **[Set up the Arduino IDE](docs/resources/setup.md)** — install the ESP32
   toolchain and get a blinking LED. Do this at home; it takes about 45 minutes.
   Sessions are only 90 minutes and there is no time to install drivers.
2. **[Read the lab safety rules](docs/resources/safety.md)** — batteries, powered
   motors, and a vehicle that will drive itself off a bench. Sign-off is required
   before Lab 1.

## Reference

- **[Setup — Arduino IDE](docs/resources/setup.md)** — toolchain, drivers, first upload
- **[Lab Safety](docs/resources/safety.md)** — LiPo handling, soldering, moving robots
- **[Hardware & Pinout](docs/resources/hardware.md)** — system architecture, pin table, I2C addresses
- **[Troubleshooting](docs/resources/troubleshooting.md)** — the failures we see every year, and what fixes them

## Platform

ESP32 development board · dual H-bridge motor driver · four DC motors ·
battery · Arduino IDE

## Found a mistake?

If something on a lab page is wrong, unclear, or contradicts what you saw at the
bench, tell a TA — or open an issue here. Corrections from students are welcome
and they make next year's version better.
