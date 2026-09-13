---
title: Lab 4 — IMU + Sensor Calibration
---

# Lab 4 — IMU + Sensor Calibration

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">90 minutes</span></div>
<div><span class="k">Depends on</span><span class="v">Lab 3</span></div>
<div><span class="k">Milestone</span><span class="v">Calibration curve + corrected sensor readings</span></div>
<div><span class="k">Status</span><span class="v">Outline — to be written</span></div>
</div>

## Practical focus

Read accelerometer and gyroscope; observe bias and noise; basic heading and turn
measurement; calibrate the IMU or the distance sensor against known references.

!!! warning "Check which board you have"
    Lab stock includes **HMC5883L magnetometer** breakouts that closely resemble
    MPU6050 IMU boards. A magnetometer gives heading, not rotation rate — they
    are not interchangeable. Scan the I2C bus first; it settles the question in
    three seconds. See [Hardware & Pinout](../resources/hardware.md#i2c-address-map).

## Objectives

1. Read raw accelerometer and gyroscope data over I2C.
2. Observe and quantify **bias** (a non-zero reading when stationary) and **noise**.
3. Measure a turn angle by integrating gyro rate.
4. Produce a calibration curve correcting measured values against known references.

## Procedure outline

TODO — expand.

1. Scan the I2C bus and confirm the device address.
2. Log 30 seconds of data with the car **completely stationary**. The mean of that is your bias; the spread is your noise.
3. Subtract the bias and re-log. Confirm it now sits near zero.
4. Rotate the car through known angles (90°, 180°) and compare the integrated gyro estimate against truth.
5. Build the calibration curve.

!!! tip "Let them watch the drift"
    Integrate the gyro for two full minutes while the car sits still. The heading
    estimate will walk away from zero on its own. That visible drift is the entire
    reason Lab 7 fuses the IMU with encoders instead of trusting it alone.

## Deliverable

**Calibration curve + corrected sensor readings.**
