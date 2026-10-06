# ME 222B — Mechatronics and Intelligent Systems

Lab materials for ME 222B, KAUST, Fall 2026/2027. Everything you need for the labs lives here, and is
published as a website:

### 📘 **[hassan-al-moalim.github.io/ME222B](https://hassan-al-moalim.github.io/ME222B/)**

Read the labs on the website — it has search, and the code blocks have copy
buttons. This repository is just where the source text lives.

---

## The labs

Twelve labs, one 90-minute session per Wednesday. You build a four-motor
autonomous delivery car on an ESP32, then make a fleet of them work together.

Lab 1 hands you a problem you cannot yet solve: send the **same** PWM command to
both sides of the car and it still curves away from the line. Lab 4 is where you
fix it with feedback and compare against your own baseline.

## Labs

| Lab | Date | Topic | Milestone |
| :--: | --- | --- | --- |
| 1 | 30 Sep | [Motors and PWM](docs/labs/lab01-pwm-motors.md) | Working car + PWM table + measured drift |
| 2 | 30 Sep | [Encoders](docs/labs/lab02-motor-characterization.md) | PWM–speed curves, dead zone, BLE link |
| 3 | 7 Oct | [BLE Motor Control + IMU](docs/labs/lab03-motors-imu.md) | Working car, phone controls, IMU readings |
| 4 | 14 Oct | [Encoders + Straight Line](docs/labs/lab04-encoders-straight-line.md) | Distance scale + IMU-corrected runs |
| 5 | 21 Oct | [Distance Sensing](docs/labs/lab05-distance-sensing.md) | Calibration curve + usable range |
| 6 | 28 Oct | [Filtering + Obstacle Stopping](docs/labs/lab06-filtering-obstacle-stop.md) | Approach trials, stopping error |
| 7 | 4 Nov | [Navigation + Mission States](docs/labs/lab07-navigation-mission-states.md) | Three autonomous A-to-B runs |
| 8 | 11 Nov | [ThingsBoard Telemetry](docs/labs/lab08-thingsboard-telemetry.md) | Live dashboard from the moving car |
| 9 | 18 Nov | [ThingsBoard Missions](docs/labs/lab09-thingsboard-missions.md) | Dashboard-triggered mission |
| 10 | 25 Nov | [Multi-Car Coordination](docs/labs/lab10-multi-car.md) | Two-car mission without contact |
| 11 | 2 Dec | [Integration + Testing](docs/labs/lab11-integration-testing.md) | Five missions, performance table |
| 12 | 9 Dec | [Final Demonstration](docs/labs/lab12-final-demo.md) | Delivery-car demo + handover |

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
- **[Syllabus](docs/syllabus.md)** — outcomes, assessment, policies, lecture schedule

## Platform

ESP32 development board · dual H-bridge motor driver · four DC motors ·
battery · Arduino IDE

## Found a mistake?

If something on a lab page is wrong, unclear, or contradicts what you saw at the
bench, tell a TA — or open an issue here. Corrections from students are welcome
and they make next year's version better.
