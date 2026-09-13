---
title: Hardware & Pinout
---

# Hardware & Pinout

Reference for the four-motor car platform: **ESP32 + dual H-bridge + four DC motors**.

## System architecture

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

The ESP32 controls the H-bridge. The H-bridge supplies the current the motors
need. The two left motors are driven together as one side; likewise the right.

!!! warning "Common ground, separate supplies"
    The ESP32 ground and the H-bridge ground **must** be connected — a PWM signal
    is meaningless without a shared reference.

    The ESP32's 3.3 V rail and the motor supply must **not** be connected.

    TODO — instructor: state unambiguously whether USB and battery power may be
    connected at the same time on this hardware, and whether the driver's 5 V
    regulator jumper stays fitted. Ambiguity here destroys boards.

## Your pin assignments

Groups choose their own pins in [Lab 1](../labs/lab01-pwm-motors.md) and keep
them for the rest of the course. Record them here and in your notebook.

<div class="pinout" markdown>

| Function | ESP32 pin |
| --- | --- |
| Left IN1 | `______` |
| Left IN2 | `______` |
| Left PWM / Enable | `______` |
| Right IN1 | `______` |
| Right IN2 | `______` |
| Right PWM / Enable | `______` |
| Encoder left A / B | `______` |
| Encoder right A / B | `______` |
| I2C `SDA` | `21` (default) |
| I2C `SCL` | `22` (default) |
| Distance sensor | `______` |
| Payload servo | `______` |

</div>

!!! danger "Pins that will bite you"
    - **GPIO 34–39 are input-only.** No output, no internal pull-up. Usable for encoders, useless for motor control.
    - **GPIO 0, 2, 12, 15 are strapping pins.** Their level at boot selects boot mode. A circuit that works until you power-cycle it is usually one of these.
    - **GPIO 6–11 are wired to the onboard flash.** Never connect anything.
    - **ADC2 pins stop working when WiFi is on** — which matters from Lab 8. Use ADC1 (GPIO 32–39) for analogue reads.

## I2C address map

One bus, several devices across Labs 3–5. Scan it whenever something stops
responding — the scanner sketch is in [Setup](setup.md#useful-snippets).

| Address | Device |
| --- | --- |
| `0x0D` | QMC5883L magnetometer (clone chip) |
| `0x1E` | HMC5883L magnetometer |
| `0x29` | VL53L0X ToF (factory default) |
| `0x40` | INA219 current sensor |
| `0x44` | SHT31 temperature / humidity |
| `0x68` | MPU6050 IMU |

!!! warning "Look-alike boards in lab stock"
    The lab holds **HMC5883L magnetometer** breakouts that closely resemble
    MPU6050 IMU boards, and **LMV358 op-amp** breakouts that resemble buck
    converters. If a board does not answer at the address you expect, scan the
    bus before concluding it is dead — you may be holding a different part.

!!! warning "Two ToF sensors collide"
    Every VL53L0X boots at `0x29`. To run two on one bus, hold one in reset via
    its `XSHUT` pin, bring up and readdress the other, then release the first.
    This must be redone on every power cycle — the new address is not persistent.

## PWM on the ESP32

The lab code uses `analogWrite()`, which the ESP32 Arduino core maps onto the
hardware LEDC peripheral. Default resolution is 8-bit, so **PWM values run 0–255**.

!!! note "Why the car does not move at low PWM"
    Below roughly 10–20 % duty the motors cannot overcome their own static
    friction. This **dead zone** is measured in
    [Lab 2](../labs/lab02-motor-characterization.md) and compensated for in
    [Lab 6](../labs/lab06-feedback-control.md).
