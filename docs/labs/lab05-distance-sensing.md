---
title: Lab 5 - Obstacle Avoidance
---

# Lab 5 - Obstacle Avoidance

<div class="lab-meta" markdown>
<div><span class="k">Date</span><span class="v">Wednesday 28 October</span></div>
<div><span class="k">Depends on</span><span class="v">Lab 4</span></div>
<div><span class="k">Milestone</span><span class="v">Car detects, confirms and avoids an obstacle using one front ToF sensor</span></div>
</div>

**Goal:** use the front time-of-flight (ToF) sensor to detect an obstacle,
confirm it is real, scan left and right by turning the car, and avoid it on the
side with more clearance.

## Hardware

| Component | Purpose |
| --- | --- |
| ESP32 | Runs sensing, decision logic and motor commands |
| TF-Luna / front ToF sensor | Measures distance to objects in front of the robot |
| H-bridge motor driver | Controls motor direction and PWM |
| Four DC motors | Moves and rotates the vehicle |
| Battery | Powers the vehicle |

## Part A · Obstacle detection

Continuously read the front distance:

$$ d = \text{measured ToF distance} $$

Choose an initial avoidance threshold $D_{avoid}$. Start around **40 cm**, then
test and tune it for your vehicle.

$$ d > D_{avoid} \;\Rightarrow\; \text{path clear, continue moving} $$

$$ d \le D_{avoid} \;\Rightarrow\; \text{possible obstacle} $$

## Part B · Confirm the obstacle

A single short reading may be noise. Do not enter avoidance mode on one sample.
Require several **consecutive** readings below $D_{avoid}$.

Example: obstacle confirmed if $d < D_{avoid}$ for $N = 3$ consecutive samples.

Record the values you choose:

<div class="worksheet" data-worksheet="lab05-confirm" markdown>

| Parameter | Selected value | Reason |
| --- | --- | --- |
| $D_{avoid}$ | `____ cm` | |
| N | `____ samples` | |

</div>

## Part C · Scan left and right

The robot has only one front-facing ToF sensor. After an obstacle is confirmed,
the car itself must turn to inspect the space on each side.

1. **Stop.** Stop the vehicle once the obstacle is confirmed.
2. **Scan left.** Rotate a small, repeatable amount to the left and measure $D_L$.
3. **Scan right.** Rotate through the center toward the right and measure $D_R$.
4. **Compare.** Compare $D_L$ and $D_R$.
5. **Select.** Choose the direction with the larger measured clearance.

$$ D_L > D_R \;\Rightarrow\; \text{avoid LEFT} \qquad D_R > D_L \;\Rightarrow\; \text{avoid RIGHT} $$

## Student algorithm · no code provided

Write the firmware yourselves from this outline.

```text
LOOP
    Read front ToF distance d

    IF path is clear:
        move forward

    ELSE IF obstacle is confirmed:
        stop
        scan left  -> DL
        scan right -> DR

        IF DL > DR:
            avoid left
        ELSE:
            avoid right

        continue moving

REPEAT
```

## Required experiments

### Test 1 · Single obstacle

Place one obstacle in the robot's forward path. The robot must detect it, avoid
it and continue without contact.

```text
START ─────► CAR ─────► [ OBSTACLE ] ─────► FINISH
```

### Test 2 · Offset obstacle

Place the obstacle so one side has visibly more free space. Check whether the
scan selects the expected side.

## Parameters to tune

<div class="worksheet" data-worksheet="lab05-params" markdown>

| Parameter | Meaning | Final value | Observation |
| --- | --- | --- | --- |
| $D_{avoid}$ | Obstacle detection threshold | `____` | |
| N | Consecutive readings required | `____` | |
| $P_{scan}$ | PWM used while scanning | `____` | |
| $T_{scan}$ | Scan rotation time / amount | `____` | |
| $P_{avoid}$ | PWM during avoidance | `____` | |
| $T_{turn}$ | Initial avoidance turning time / amount | `____` | |

</div>

## Results

<div class="worksheet" data-worksheet="lab05-results" markdown>

| Test | $D_{avoid}$ | $D_L$ | $D_R$ | Chosen side | Collision? | Success? |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | | | | | | |
| 2 | | | | | | |
| 3 | | | | | | |
| 4 | | | | | | |
| 5 | | | | | | |
| 6 | | | | | | |

</div>

## Rules

- No remote-control commands after START.
- No touching or repositioning the robot after START.
- Obstacle sensing must use the front ToF sensor.
- The robot must make its own left/right avoidance decisions.
- A collision means the run is unsuccessful.
