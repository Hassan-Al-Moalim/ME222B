---
title: Lab 2 — Motor Characterization & Encoders
---

# Lab 2 — Motor Characterization & Encoders

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">90 minutes</span></div>
<div><span class="k">Depends on</span><span class="v">Lab 1</span></div>
<div><span class="k">Milestone</span><span class="v">PWM–speed curves + left/right characterization</span></div>
<div><span class="k">Status</span><span class="v">Outline — to be written</span></div>
</div>

## Practical focus

Measure actual wheel speed; encoder counts (if the hardware allows); RPM; test
several PWM values; compare left and right sides; identify the **dead zone** and
**saturation**.

!!! note "Encoders are conditional"
    The lab plan marks encoder counting as *"if we can do it"*. TODO — instructor:
    confirm whether the chassis has encoders before the session, and write the
    fallback path (timed revolutions by hand, marked wheel) either way. Students
    need a speed number regardless of which method produces it.

## Objectives

1. Measure actual wheel speed rather than assuming it from the PWM command.
2. Build a PWM-vs-speed curve for each side of the vehicle.
3. Identify the **dead zone** — the PWM below which the wheels do not turn at all.
4. Identify **saturation** — the PWM above which speed stops increasing.
5. Quantify the left/right mismatch you observed in [Lab 1](lab01-pwm-motors.md).

## Procedure outline

TODO — expand into numbered steps.

1. Set up a repeatable speed measurement (encoder counts, or timed revolutions of a marked wheel).
2. Sweep PWM across the usable range. Suggested points: 0, 60, 100, 130, 160, 190, 220, 255.
3. Record speed for **each side separately**, with the car lifted.
4. Plot both curves on one graph.
5. Read the dead zone and saturation point off the curves.

| PWM | Left speed | Right speed | Difference |
| --- | --- | --- | --- |
| 0 | | | |
| 60 | | | |
| 100 | | | |
| 130 | | | |
| 160 | | | |
| 190 | | | |
| 220 | | | |
| 255 | | | |

## Deliverable

**PWM–speed curves + left/right motor characterization.**

!!! question "Carry this into Lab 6"
    You now know the two sides differ, and by how much. Could you cancel the
    drift by simply offsetting one side's PWM by the difference you measured?
    Try it. Note carefully where it works and where it stops working — that gap
    is the argument for feedback control.
