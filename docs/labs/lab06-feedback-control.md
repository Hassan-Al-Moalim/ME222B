---
title: Lab 6 — Feedback Control (P / PI / PID)
---

# Lab 6 — Feedback Control — P / PI / PID

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">90 minutes</span></div>
<div><span class="k">Depends on</span><span class="v">Lab 5</span></div>
<div><span class="k">Milestone</span><span class="v">Closed-loop controller with measured error</span></div>
<div><span class="k">Status</span><span class="v">Outline — to be written</span></div>
</div>

## Practical focus

**Now introduce feedback.** Use encoder speed or measured stopping distance;
desired vs actual value; P first, then PI/PID; experimental tuning.

## Background

Discrete PID at fixed timestep $\Delta t$:

$$ u_k = K_p e_k + K_i \sum_{j=0}^{k} e_j \Delta t + K_d \frac{e_k - e_{k-1}}{\Delta t} $$

where $e_k = \text{desired} - \text{actual}$.

TODO — expand. Cover **integral windup** explicitly: the motor dead zone you
measured in [Lab 2](lab02-motor-characterization.md) means small errors produce
no motion at all, so the integral term charges up with nowhere to go and then
overshoots hard. Clamping the integral is the fix, and it is worth letting
students hit the bug before you name it.

## Objectives

1. State a control problem as desired value, actual value, and error.
2. Implement proportional control and observe steady-state error.
3. Add integral action and observe both the benefit and the windup failure.
4. Add derivative action and observe noise amplification.
5. Tune experimentally against a stated performance target.

## Procedure outline

TODO — expand. Suggested arc, in this order:

1. **P only.** Watch it settle short of target. Measure the steady-state error.
2. **Add I.** Watch the error go to zero — then watch it overshoot badly after a stall. That is windup.
3. **Clamp the integral.** Watch the overshoot disappear.
4. **Add D.** Watch it get twitchy on noisy encoder data, then filter the derivative.
5. Tune to spec and record the final gains.

| Controller | $K_p$ | $K_i$ | $K_d$ | Settling time | Steady-state error | Overshoot |
| --- | --- | --- | --- | --- | --- | --- |
| P | | — | — | | | |
| PI | | | — | | | |
| PID | | | | | | |

!!! tip "Fix the timestep"
    Run the controller on a fixed interval with `millis()`, not once per `loop()`.
    A variable $\Delta t$ makes $K_i$ and $K_d$ meaningless and the tuning
    irreproducible.

## Deliverable

**Closed-loop speed or stopping controller with measured error.**
