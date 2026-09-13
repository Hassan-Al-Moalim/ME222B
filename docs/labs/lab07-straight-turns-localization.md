---
title: Lab 7 — Straight Driving, Accurate Turns & Localization
---

# Lab 7 — Straight Driving, Accurate Turns & Localization

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">90 minutes</span></div>
<div><span class="k">Depends on</span><span class="v">Lab 6</span></div>
<div><span class="k">Milestone</span><span class="v">Repeatable straight run + controlled turn + localization</span></div>
<div><span class="k">Status</span><span class="v">Outline — to be written</span></div>
</div>

## Practical focus

Use encoder and IMU feedback to **correct the drift problem from
[Lab 1](lab01-pwm-motors.md)**; drive a fixed distance; perform 90° turns;
estimate position and heading between known points.

!!! note "This is the payoff lab"
    Everything since Week 1 has been building the measurement and control tools
    needed to solve the problem the students met on day one. Say that out loud —
    it is the moment the course structure becomes visible to them.

## Objectives

1. Drive a straight line using feedback, and beat your Lab 1 drift figure.
2. Execute a repeatable 90° turn.
3. Drive a commanded distance and stop accurately.
4. Estimate position and heading by dead reckoning between two known points.

## Procedure outline

TODO — expand.

1. Close a heading loop using the IMU, or a wheel-speed-matching loop using encoders — or both.
2. **Repeat the exact Lab 1 drift test.** Same 4 m track, same 5 runs, same measurement.
3. Compare directly against the Lab 1 numbers.

    | | Lab 1 (open loop) | Lab 7 (closed loop) |
    | --- | --- | --- |
    | Mean drift over 4 m | | |
    | Spread across 5 runs | | |

4. Calibrate a 90° turn. Measure the actual angle; correct; repeat until consistent.
5. Dead-reckon between two marked points and measure the final position error.

!!! warning "Dead reckoning accumulates error"
    Every small heading error integrates into a growing position error. Measure
    how far off the estimate is after 4 m, then after 8 m. The growth is the
    point — it is why Labs 9 and 10 keep an absolute reference in the loop.

## Deliverable

**Repeatable straight run + controlled turn + simple localization**, reported
against your Lab 1 baseline.
