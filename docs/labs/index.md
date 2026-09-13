---
title: Labs
---

# Labs

Ten labs, 90 minutes each, building one autonomous car — then a fleet of them.

The sequence is deliberate. **Week 1 hands you a problem you cannot yet solve**:
the car will not drive straight. Everything through Week 6 builds the measurement
and control tools you need, and **Week 7 is where you finally fix it**. Weeks
8–10 take the working vehicle and put it in a coordinated fleet.

<div class="grid cards" markdown>

-   **[Lab 1 — PWM, Four Motors & Motor Mismatch](lab01-pwm-motors.md)**

    ---

    First full motion lab. Run four motors, introduce PWM, drive 4 m, and measure
    how far off straight you end up.

-   **[Lab 2 — Motor Characterization & Encoders](lab02-motor-characterization.md)**

    ---

    Measure what the wheels are *actually* doing. PWM–speed curves, dead zone,
    saturation, and the left/right mismatch quantified.

-   **[Lab 3 — LiDAR / ToF Distance Sensing](lab03-distance-sensing.md)**

    ---

    Mount a distance sensor, find its real range and field of view, and write a
    dependable `obstacleDetected()`.

-   **[Lab 4 — IMU + Sensor Calibration](lab04-imu-calibration.md)**

    ---

    Accelerometer and gyro, bias and noise, heading measurement, and calibration
    against known references.

-   **[Lab 5 — Logging, Filtering & Sensor-Based Motion](lab05-logging-filtering.md)**

    ---

    Log everything at once, compare raw against filtered, and make the car slow
    down as an obstacle gets closer.

-   **[Lab 6 — Feedback Control (P / PI / PID)](lab06-feedback-control.md)**

    ---

    Desired versus actual. P first, then PI, then PID — and the windup failure
    in between.

-   **[Lab 7 — Straight Driving, Turns & Localization](lab07-straight-turns-localization.md)**

    ---

    The payoff. Close the loop, re-run the Week 1 drift test, and beat it.

-   **[Lab 8 — Payload Servo + ThingsBoard](lab08-servo-thingsboard.md)**

    ---

    A servo release mechanism, telemetry to a dashboard, and missions dispatched
    back to the car.

-   **[Lab 9 — Multi-Car Coordination](lab09-multi-car.md)**

    ---

    Two cars, one arena. Zone reservation or priority, tested at a crossing and
    in a narrow corridor.

-   **[Lab 10 — Final Autonomous Fleet Challenge](lab10-fleet-challenge.md)**

    ---

    Pickup, navigate, avoid, deliver, release, report. Multiple cars, shared
    arena, unattended.

</div>

## Before every lab

- [ ] Read the lab page **before** you arrive. Sessions are 90 minutes; there is no time to read from cold.
- [ ] Laptop with the Arduino IDE working — see [Setup](../resources/setup.md).
- [ ] Your group's car, battery charged.
- [ ] Your pin table from [Lab 1](lab01-pwm-motors.md#record-your-connections). Every lab after the first assumes it.
- [ ] Know where the fire blanket and the LiPo bucket are. See [Safety](../resources/safety.md).
