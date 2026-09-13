---
title: Lab 10 — Final Autonomous Fleet Challenge
---

# Lab 10 — Final Autonomous Fleet Challenge

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">Final session</span></div>
<div><span class="k">Depends on</span><span class="v">Labs 1–9</span></div>
<div><span class="k">Milestone</span><span class="v">Fleet demonstration + performance results</span></div>
<div><span class="k">Status</span><span class="v">Outline — to be written</span></div>
</div>

## The challenge

Full mission, multiple cars, shared arena:

```text
pickup ──► navigation ──► obstacle handling ──► delivery ──► payload release ──► telemetry
```

Every subsystem from Weeks 1–9, running together, unattended.

## What is assessed

TODO — instructor: set the rubric and the pass bar. A workable split:

| Component | Weight | What it means |
| --- | --- | --- |
| Mission completion | TODO % | Pickup to release, unattended |
| Safety | TODO % | No contact between cars, no contact with obstacles |
| Telemetry | TODO % | Dashboard reflects true vehicle state throughout |
| Repeatability | TODO % | Three consecutive runs, not one |
| Report & analysis | TODO % | Performance data, and honest failure analysis |

!!! tip "Require three consecutive successful runs"
    A robot that works once got lucky. Demanding three in a row is what forces
    teams to fix the intermittent problems they have been working around all
    semester — and the gap between one success and three is where most of the
    engineering learning in this course actually happens.

## Performance results to report

TODO — specify. Suggested measurements:

- Mission completion time, per run.
- Position error at pickup and at delivery.
- Number of obstacle stops, and number of coordination conflicts resolved.
- Any run that failed, and the diagnosis.

!!! note "Failed runs are data"
    A report that says *"run 2 failed because the heading estimate drifted 15°
    after the third turn, which we traced to gyro bias re-emerging as the board
    warmed up"* is worth more than a report with three clean runs and no
    analysis. Say so in the rubric, or students will hide their failures.

## Deliverable

**Final fleet demonstration + performance results.**
