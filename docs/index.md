---
title: Home
---

# ME 222 — Robotics Lab

!!! warning "Placeholders to fill in"
    Every `TODO` on this site is something only the instructor can supply —
    dates, room, assessment weights, emergency numbers, ThingsBoard details.
    Run `grep -rn TODO docs/` before sharing the link with students.

Ten weeks. You build a four-motor autonomous car, then make a fleet of them
work together.

<div class="lab-meta" markdown>
<div><span class="k">Sessions</span><span class="v">10 × 90 minutes</span></div>
<div><span class="k">Platform</span><span class="v">ESP32 + dual H-bridge + 4-motor car</span></div>
<div><span class="k">Software</span><span class="v">Arduino IDE</span></div>
<div><span class="k">Lab room</span><span class="v">TODO — building / room</span></div>
</div>

## Start here

<div class="grid cards" markdown>

-   :material-download: **[Setup — Arduino IDE](resources/setup.md)**

    ---

    Do this **before** Week 1. Installs the ESP32 toolchain and proves your
    laptop can talk to the board. ~45 minutes.

-   :material-shield-alert: **[Lab Safety](resources/safety.md)**

    ---

    Batteries, powered motors, a vehicle that drives itself off the bench. Sign
    off before Lab 1.

-   :material-car: **[Lab 1 — Four-Motor Bring-Up](labs/lab01-pwm-motors.md)**

    ---

    First session. Run four motors, drive 4 metres, and find out that your car
    does not go straight.

-   :material-calendar: **[Schedule](schedule.md)**

    ---

    All ten weeks, with the practical focus and deliverable for each.

</div>

## The thread running through the course

Week 1 gives you a problem you cannot yet solve: send the **same** PWM command to
both sides and the car still curves away from the line.

You cannot fix it in Week 1, because you have no way to measure what the wheels
are actually doing. So:

```text
Week 1   measure the problem        drift over 4 m
Week 2   measure the wheels         PWM–speed curves, dead zone
Weeks 3-5  measure the world        distance, heading, filtered data
Week 6   learn to correct           P → PI → PID
Week 7   SOLVE IT                   re-run the Week 1 test and beat it
Weeks 8-10  scale it up             telemetry, missions, a coordinated fleet
```

Keep your Week 1 drift number. You will be compared against it.

## How the lab pages work

Each page follows the same shape:

| Section | What's in it |
| --- | --- |
| **Practical focus** | What the 90 minutes is spent doing |
| **Objectives** | What you should be able to do by the end |
| **Procedure** | Numbered steps, with tables to fill in |
| **Checkpoints** | Stop and get a TA before continuing |
| **Deliverable** | The milestone for that week |

!!! checkpoint "Checkpoints are not optional"
    A checkpoint means **stop and get a TA**. They sit where a mistake is
    expensive — a reversed battery, a car about to drive off a bench, a motor
    wired backwards before a measured run. Powering past one without a signature
    is how kit gets destroyed and data gets wasted.

## Getting help

- **In lab** — ask a TA. That is what they are there for.
- **Outside lab** — TODO: put the real channel here (Teams / Piazza / email).
- **Something broken?** — check [Troubleshooting](resources/troubleshooting.md)
  first. If it is genuinely dead, tell a TA so it gets logged and replaced.
