---
title: Lab 3 — LiDAR / ToF Distance Sensing
---

# Lab 3 — LiDAR / ToF Distance Sensing

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">90 minutes</span></div>
<div><span class="k">Depends on</span><span class="v">Lab 2</span></div>
<div><span class="k">Milestone</span><span class="v">Reliable obstacleDetected() + sensor test data</span></div>
<div><span class="k">Status</span><span class="v">Outline — to be written</span></div>
</div>

## Practical focus

Mount the distance sensor; measure known distances; determine useful range and
field of view; implement basic obstacle detection.

!!! warning "Class 1 laser product"
    The scanners are Class 1 and safe in normal use. Do not open a housing or
    run a unit with a cracked case — report it instead.

## Objectives

1. Mount a distance sensor rigidly to the chassis.
2. Calibrate readings against known distances.
3. Determine the sensor's **useful range** and **field of view** experimentally.
4. Write a dependable `obstacleDetected()` function.

## Procedure outline

TODO — expand.

1. Mount the sensor. Height and tilt matter more than students expect — a sensor aimed slightly down will see the floor as an obstacle.
2. Place a target at known distances and record readings.

    | True distance | Reading 1 | Reading 2 | Reading 3 | Mean | Error |
    | --- | --- | --- | --- | --- | --- |
    | 10 cm | | | | | |
    | 25 cm | | | | | |
    | 50 cm | | | | | |
    | 100 cm | | | | | |
    | 200 cm | | | | | |

3. Find the near limit and far limit where readings become unreliable.
4. Sweep a target sideways to map the field of view.
5. Implement and test `obstacleDetected()`.

!!! tip "Make the threshold a named constant"
    ```cpp
    const int OBSTACLE_CM = 30;   // tune this, don't scatter magic numbers
    ```
    You will change this value in Labs 5, 9, and 10. Give it one home now.

## Deliverable

**Reliable `obstacleDetected()` function + sensor test data.**

Include the distance and field-of-view tables, and state the range over which
you trust the sensor.
