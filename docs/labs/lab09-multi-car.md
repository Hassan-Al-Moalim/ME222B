---
title: Lab 9 — Multi-Car Coordination & System Integration
---

# Lab 9 — Multi-Car Coordination & System Integration

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">90 minutes</span></div>
<div><span class="k">Depends on</span><span class="v">Lab 8</span></div>
<div><span class="k">Milestone</span><span class="v">Two-car coordinated demo with no contact</span></div>
<div><span class="k">Status</span><span class="v">Outline — to be written</span></div>
</div>

## Practical focus

Run at least two cars; local obstacle safety; simple zone reservation or
priority; test crossing and narrow-corridor cases.

## Objectives

1. Operate two vehicles in a shared space without contact.
2. Keep local obstacle avoidance authoritative over any remote command.
3. Implement a simple coordination scheme — zone reservation or fixed priority.
4. Test the two cases that actually break naive schemes: a crossing and a narrow corridor.

## Procedure outline

TODO — expand.

1. Two cars, independent missions, no coordination. Observe what goes wrong. **Do this first** — it motivates everything after it.
2. Add local obstacle safety: a car stops for anything in front of it regardless of its mission.
3. Add coordination. Two workable schemes:
    - **Zone reservation** — the arena is divided into zones; a car requests a zone before entering and releases it on exit.
    - **Fixed priority** — each car has a rank; lower rank yields at conflicts.
4. Test the crossing case: both cars arrive at an intersection together.
5. Test the narrow corridor: only one car fits, and both want through.

!!! danger "Local safety always wins"
    A car must stop for an obstacle even when the dashboard is telling it to
    drive. If a remote command can override the local obstacle check, the system
    is unsafe — and in Lab 10's shared arena it will find the failure for you.

!!! warning "Deadlock is the expected failure"
    Two cars that each politely wait for the other will wait forever. That is a
    genuine finding, not a bug to hide. Detect it — a timeout is enough — and
    say in your report how you broke the tie.

## Deliverable

**Two-car coordinated demonstration with no contact.**
