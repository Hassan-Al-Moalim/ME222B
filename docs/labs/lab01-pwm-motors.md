---
title: Lab 1 — PWM, Four Motors & Motor Mismatch
---

# Lab 1 — Four-Motor Car Bring-Up and Motor Mismatch

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">90 minutes</span></div>
<div><span class="k">Platform</span><span class="v">ESP32 + dual H-bridge + 4-motor car</span></div>
<div><span class="k">Milestone</span><span class="v">Working car + PWM table + measured drift</span></div>
<div><span class="k">Bring</span><span class="v">Laptop with Arduino IDE</span></div>
</div>

## 1. Lab objective

In this first laboratory you will connect and test the four DC motors of the
robot car using an ESP32 and an H-bridge motor driver.

The main experiment investigates a simple question:

!!! question "The question this lab is built around"
    **If the same PWM command is sent to the motors on both sides of the car,
    will the car travel straight?**

Hold onto your answer. Week 7 is where you fix whatever you find today.

## 2. Hardware required

Each group requires:

- Four-motor robot car chassis
- ESP32 development board
- Dual H-bridge motor driver
- Battery
- Jumper wires
- USB cable
- Computer with Arduino IDE
- Measuring tape

!!! danger "Before you power anything"
    Lift the car so all four wheels spin free before the first run. A car that
    starts unexpectedly on the bench will drive itself onto the floor, and a
    trailing USB cable will take your laptop with it.

## 3. Electrical architecture

```text
                   ESP32
                     │
              Control Signals
                     │
                     ▼
              ┌──────────────┐
Battery ─────►│   H-BRIDGE   │
              │ MOTOR DRIVER │
              └──────┬───────┘
                     │
             ┌───────┴───────┐
             │               │
             ▼               ▼
        Left Motors      Right Motors
```

The ESP32 controls the H-bridge. The H-bridge supplies the current required by
the motors.

!!! warning "The ESP32 never powers the motors"
    Motor current comes from the battery, through the H-bridge. The ESP32 only
    sends signals. If you try to run motors off the ESP32's 3.3 V pin you will
    brown out the board and it will reset the moment the wheels load up.

### Record your connections

Before writing the program, complete this table. Keep it — every later lab
assumes these pins.

<div class="pinout-pair" markdown>

<div class="pinout" data-worksheet="left-driver" markdown>

**Left driver** — both left-side motors

| Function | ESP32 pin |
| --- | --- |
| Forward IN1 | `______` |
| Forward IN2 | `______` |
| Forward PWM / Enable | `______` |
| Backward IN1 | `______` |
| Backward IN2 | `______` |
| Backward PWM / Enable | `______` |

</div>

<div class="pinout" data-worksheet="right-driver" markdown>

**Right driver** — both right-side motors

| Function | ESP32 pin |
| --- | --- |
| Forward IN1 | `______` |
| Forward IN2 | `______` |
| Forward PWM / Enable | `______` |
| Backward IN1 | `______` |
| Backward IN2 | `______` |
| Backward PWM / Enable | `______` |

</div>

</div>

!!! note "One driver board per side, two motors each"
    Each dual H-bridge carries two channels, so one board runs an entire side of
    the car: **Forward** is that side's front motor, **Backward** is its rear
    motor. Steering comes from driving the left board differently to the right
    board — which is why all four motors must be wired the right way round
    before any of the drift measurements mean anything.

## 4. Starting program

Complete the missing parts.

```cpp title="ME222_Lab1.ino"
// ======================================
// ME 222B - LAB 1
// Four-Motor Robot Car
// Two H-bridges: one per side
// ======================================


// --------------------------------------
// Enter your ESP32 pin numbers.
// Copy them from the tables above.
// --------------------------------------

// LEFT driver

#define L_FWD_IN1   ___
#define L_FWD_IN2   ___
#define L_FWD_PWM   ___

#define L_BWD_IN1   ___
#define L_BWD_IN2   ___
#define L_BWD_PWM   ___


// RIGHT driver

#define R_FWD_IN1   ___
#define R_FWD_IN2   ___
#define R_FWD_PWM   ___

#define R_BWD_IN1   ___
#define R_BWD_IN2   ___
#define R_BWD_PWM   ___


// --------------------------------------
// Initial motor commands
// SAME command for both sides
// --------------------------------------

int leftSpeed  = 180;
int rightSpeed = 180;


// ======================================
// SETUP
// ======================================

void setup()
{
  pinMode(L_FWD_IN1, OUTPUT);
  pinMode(L_FWD_IN2, OUTPUT);
  pinMode(L_FWD_PWM, OUTPUT);

  pinMode(L_BWD_IN1, OUTPUT);
  pinMode(L_BWD_IN2, OUTPUT);
  pinMode(L_BWD_PWM, OUTPUT);

  pinMode(R_FWD_IN1, OUTPUT);
  pinMode(R_FWD_IN2, OUTPUT);
  pinMode(R_FWD_PWM, OUTPUT);

  pinMode(R_BWD_IN1, OUTPUT);
  pinMode(R_BWD_IN2, OUTPUT);
  pinMode(R_BWD_PWM, OUTPUT);

  stopCar();
}


// ======================================
// MOVE FORWARD
//
// Fill in HIGH and LOW.
//
// You may need to change the direction
// depending on your motor wiring.
// ======================================

void forward()
{
  // LEFT SIDE - front motor

  digitalWrite(L_FWD_IN1, ___);
  digitalWrite(L_FWD_IN2, ___);


  // LEFT SIDE - rear motor

  digitalWrite(L_BWD_IN1, ___);
  digitalWrite(L_BWD_IN2, ___);


  // RIGHT SIDE - front motor

  digitalWrite(R_FWD_IN1, ___);
  digitalWrite(R_FWD_IN2, ___);


  // RIGHT SIDE - rear motor

  digitalWrite(R_BWD_IN1, ___);
  digitalWrite(R_BWD_IN2, ___);


  // Apply motor speed

  analogWrite(L_FWD_PWM, leftSpeed);
  analogWrite(L_BWD_PWM, leftSpeed);

  analogWrite(R_FWD_PWM, rightSpeed);
  analogWrite(R_BWD_PWM, rightSpeed);
}


// ======================================
// STOP
// ======================================

void stopCar()
{
  digitalWrite(L_FWD_IN1, LOW);
  digitalWrite(L_FWD_IN2, LOW);

  digitalWrite(L_BWD_IN1, LOW);
  digitalWrite(L_BWD_IN2, LOW);

  digitalWrite(R_FWD_IN1, LOW);
  digitalWrite(R_FWD_IN2, LOW);

  digitalWrite(R_BWD_IN1, LOW);
  digitalWrite(R_BWD_IN2, LOW);

  analogWrite(L_FWD_PWM, 0);
  analogWrite(L_BWD_PWM, 0);

  analogWrite(R_FWD_PWM, 0);
  analogWrite(R_BWD_PWM, 0);
}


// ======================================
// MAIN TEST
// ======================================

void loop()
{
  forward();

  // Long enough for the car
  // to travel several metres.

  delay(8000);

  stopCar();

  // Do not repeat automatically.

  while(1);
}
```

!!! warning "Every motor is written out separately"
    Four motors means four pairs of `digitalWrite` and four `analogWrite` calls.
    Miss a pair and that wheel sits dead, which makes the car curve hard — and
    looks exactly like the motor mismatch you are about to measure. Check all
    four turn in Task 1 before trusting any drift number.

!!! note "Why `while(1)` at the end"
    Without it, `loop()` restarts and the car drives off again the moment you
    catch it. The program runs **once** per reset, deliberately.

## 5. Task 1 — Test the four motors

1. **Lift the car** so that all four wheels are free to rotate.
2. Run the program.
3. Check each motor turns:

- [ ] Left-front motor rotates
- [ ] Left-rear motor rotates
- [ ] Right-front motor rotates
- [ ] Right-rear motor rotates

### If one motor rotates in the wrong direction

You may correct it by:

- reversing the two wires connected to that motor, **or**
- correcting the direction in the motor-control logic.

For this first lab, use the simplest reliable solution.

!!! checkpoint "Checkpoint 1 — all four turning, correct direction"
    Get a TA before putting the car on the floor. Four motors turning the right
    way is the whole prerequisite for everything below.

## 6. Task 2 — Verify motion

1. Place the car on the floor.
2. Use:

    ```cpp
    leftSpeed  = 180;
    rightSpeed = 180;
    ```

3. Run the car for approximately **1–2 metres**.

Your goal at this stage is only to confirm that **the entire vehicle moves
forward**.

!!! warning "Stop and fix wiring first"
    If the car rotates on the spot or one side moves backward, correct the
    wiring before continuing. Do not proceed to the drift measurement with a
    vehicle that isn't driving forward — you'd be measuring the wrong thing.

## 7. Prepare the test track

Use a straight section of floor approximately **4 metres** long. A longer track
may be used if space allows.

Mark:

- **START** position
- Intended straight reference line
- **FINISH** position

```text
START
  │
  │
  │
  │
  │     Intended Path
  │
  │
  │
  ▼
FINISH
```

Place the centre of the vehicle on the reference line. **Align the vehicle
carefully before every run** — a sloppy start looks exactly like motor mismatch
and will ruin your data.

Run the car **5 times**. Observe its trajectory. At the end of each run, measure
the **lateral drift** — the sideways distance from the reference line to the
centre of the vehicle.

<div class="worksheet" data-worksheet="drift-runs" markdown>

| Run | Left PWM | Right PWM | Distance | Drift | Direction |
| --- | --- | --- | --- | --- | --- |
| 1 | 180 | 180 | `____` m | `____` cm | `______` |
| 2 | 180 | 180 | `____` m | `____` cm | `______` |
| 3 | 180 | 180 | `____` m | `____` cm | `______` |
| 4 | 180 | 180 | `____` m | `____` cm | `______` |
| 5 | 180 | 180 | `____` m | `____` cm | `______` |

</div>

### Discuss what you observed

Answer:

1. Does the vehicle travel perfectly straight?
2. Does it tend to move repeatedly toward the same side?
3. Is the error exactly the same on every run?


## 8. Write your problem statement

Complete the following. This is the statement Lab 7 asks you to revisit.

<div class="worksheet" data-worksheet="problem-statement" markdown>

**Observation**

> When we applied the same PWM command of `______` to both sides of the vehicle,
> the car `_________________________________________________`.
>
> After travelling approximately `______` metres, the vehicle had a lateral
> error of approximately `______` cm.

**Engineering problem**

> Although the left and right motors receive the same electrical command,
> `_________________________________________________`.

**Consequence**

> Because of this difference, the vehicle
> `_________________________________________________`.

</div>

## Deliverable

**Working car + PWM table + measured straight-line drift.**

Submit:

1. Your completed pin connection table.
2. Your 5-run drift table.
3. Your completed problem statement.

!!! question "Carry this into Lab 2"
    You have just measured a problem you cannot yet fix, because you have no way
    to know how fast each wheel is *actually* turning — only what you commanded.
    Lab 2 gives you that measurement.
