---
title: Lab 5 — Data Logging, Filtering & Sensor-Based Motion
---

# Lab 5 — Data Logging, Filtering & Sensor-Based Motion

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">90 minutes</span></div>
<div><span class="k">Depends on</span><span class="v">Lab 4</span></div>
<div><span class="k">Milestone</span><span class="v">Raw-vs-filtered plots + speed = f(distance)</span></div>
<div><span class="k">Status</span><span class="v">Outline — to be written</span></div>
</div>

## Practical focus

Log PWM, encoder, distance and IMU data; compare raw and filtered readings;
apply a moving average or median filter; make the car slow as obstacle distance
decreases.

## Objectives

1. Log four data streams simultaneously with consistent timestamps.
2. Implement a moving-average and a median filter, and compare them.
3. Explain the trade-off filtering makes: less noise, more lag.
4. Make vehicle speed a **function of measured distance** rather than a constant.

## Procedure outline

TODO — expand.

1. Set up CSV-style serial logging: `millis(), pwmL, pwmR, encL, encR, distance, heading`.
2. Capture a run. Import to a spreadsheet and plot.
3. Apply a moving average over N samples. Try N = 3, 5, 10 and overlay the results.
4. Try a median filter on the distance channel specifically — it rejects single-sample spikes that a mean smears out.
5. Implement `speed = f(distance)` so the car slows as it approaches an obstacle.

!!! warning "Filtering is not free"
    A 10-sample moving average on a 20 Hz sensor delays your obstacle detection
    by a quarter of a second. At 0.5 m/s that is 12 cm of extra stopping distance.
    Choose N deliberately and justify it in your report.

!!! tip "Log to CSV from the start"
    ```cpp
    Serial.printf("%lu,%d,%d,%ld,%ld,%d\n", millis(), pwmL, pwmR, encL, encR, dist);
    ```
    Paste straight into a spreadsheet. Do not print prose you then have to clean up.

## Deliverable

**Raw-vs-filtered plots + `speed = f(distance)` behaviour.**
