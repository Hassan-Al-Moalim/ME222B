---
title: Troubleshooting
---

# Troubleshooting

The failures that come up every year. Check here before flagging a TA — and if
your problem isn't listed, tell us so it gets added.

## Upload and connection

??? failure "No serial port appears"
    1. **Charge-only USB cable.** The most common cause by a wide margin. Swap it for a known data cable.
    2. Board not powered — no LED at all? Try another USB socket, not a hub.
    3. Missing driver (CP210x or CH34x) — see [Setup](setup.md#3-usb-driver-and-port).
    4. On Linux: port exists but permission denied → you're not in the `dialout` group, or you added yourself and didn't log out.

??? failure "Failed to connect to ESP32: Timed out waiting for packet header"
    The board isn't entering bootloader mode.

    - Hold `BOOT` (or `IO0`) while the upload starts; release at "Connecting…". Many boards need this every time.
    - Close the Serial Monitor.
    - Lower the upload speed under **Tools → Upload Speed** to `115200`. Long or poor cables can't sustain higher rates.
    - Disconnect everything from GPIO 0, 2, 12, and 15. A strapping pin held at the wrong level blocks boot.

??? failure "Board boots into a reset loop / brownout detector triggered"
    The supply is sagging. Almost always the motors: their inrush current pulls
    the shared rail below the ESP32's brownout threshold.

    - Power motors from the battery, logic through the buck converter — not both off USB.
    - Add bulk capacitance across the motor supply.
    - Confirm your buck converter is actually delivering 5 V **under load**, not just unloaded.

## I2C sensors

??? failure "Sensor not found / bus scan returns nothing"
    1. **Run the scan first** — see [Setup](setup.md#useful-snippets). It answers "wiring or code?" in three seconds.
    2. SDA and SCL swapped. Extremely common. Swap them and rescan.
    3. Missing pull-up resistors. Most breakouts include them; a bare chip doesn't.
    4. Sensor not powered — measure VCC at the sensor's own pin, not at the rail.
    5. Loose jumper. Wiggle each one while scanning.

??? failure "Bus scan finds a device, but at an unexpected address"
    You may have a different part than you think. Check the [address map](hardware.md#i2c-address-map) —
    lab stock includes HMC5883L magnetometer boards that resemble MPU6050 IMUs.

??? failure "Two identical sensors, only one shows up"
    Address collision. Two VL53L0X boards both boot at `0x29`. Hold one in reset
    via `XSHUT`, bring up and readdress the other, then release the first. Redo
    this on every power cycle — the reassignment is not persistent.

??? failure "Readings freeze after a while"
    The bus has hung, usually from electrical noise coupling in from the motors.

    - Route I2C wires away from motor leads. Keep them short.
    - Lower the bus speed: `Wire.setClock(50000)`.
    - Add a timeout and bus recovery rather than blocking forever on a read.

## Motors

??? failure "Motor runs at full speed regardless of PWM"
    The driver's enable jumpers are still fitted, tying enable permanently high
    so your PWM never reaches the motor. Pull them off and drive the enable pin
    from the ESP32.

??? failure "Motor doesn't turn at low duty cycle"
    Expected — that's the deadband. Below roughly 10–20 % duty the motor can't
    overcome static friction. Characterise it in [Lab 2](../labs/lab02-motor-characterization.md)
    and compensate in [Lab 6](../labs/lab06-feedback-control.md).

??? failure "Motor only turns one direction"
    - One of `IN1`/`IN2` isn't reaching the board — check continuity.
    - The pin you're using is input-only (GPIO 34–39). Move it.
    - The H-bridge has one side blown. Swap the motor to the other channel: if it works there, the driver is damaged. Tell a TA.

??? failure "One of the four motors spins backwards"
    Normal on first bring-up. Fix it either by reversing that motor's two wires,
    or by correcting the direction logic in code. For [Lab 1](../labs/lab01-pwm-motors.md),
    use whichever is simpler — just be consistent and write down what you did.

??? failure "Car curves when both sides are commanded equally"
    **Expected** — this is the whole point of [Lab 1](../labs/lab01-pwm-motors.md).
    The two sides are not identical and their dead zones differ. Measure it, don't
    fix it yet. [Lab 7](../labs/lab07-straight-turns-localization.md) is where you
    close the loop and beat your Week 1 number.

??? failure "ESP32 resets whenever the motors start"
    See the brownout entry above. Separate the supplies.

## Encoders

??? failure "Counts jump erratically or drift with the motor running"
    - Interrupt handler too slow. Keep it to incrementing a `volatile` counter — no `Serial.print` inside an ISR.
    - Electrical noise from the motor. Twist the encoder wires, keep them away from motor leads, add a small filter cap.
    - Counter not declared `volatile`, so the compiler optimised away the read.

??? failure "Counts go the wrong way"
    Swap the A and B channel wires, or negate in software. Do it once, in one
    place, and write down which you chose.

## Power

??? failure "Buck converter output is wrong"
    Adjust the trimpot **with the output disconnected**, verify with a
    multimeter, then connect the load. Many turns are needed — these are
    multi-turn pots and it feels like nothing is happening at first.

??? failure "Battery dies fast"
    Measure the actual draw rather than guessing. Check that the
    pack is genuinely 2S and genuinely charged — the lab holds a mix of 2S and 3S
    packs, and a pack at storage charge looks fine until it's under load.

??? danger "Battery is puffed, hot, or damaged"
    Stop. Disconnect it. Put it in a LiPo-safe bag, on a non-flammable surface,
    away from people. Tell a TA immediately. Do not charge it, do not use it, do
    not bin it. See [Safety](safety.md#lithium-polymer-batteries).
