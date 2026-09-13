---
title: Setup — Arduino IDE
---

# Setup — Arduino IDE & ESP32

!!! danger "Do this before Week 1"
    Lab 1 starts with everyone wiring motors. If you arrive with a laptop that
    cannot talk to an ESP32, you will spend the session installing drivers
    instead of doing the lab — and the session is only 90 minutes.

    Budget 45 minutes at home.

## 1. Install the Arduino IDE

Download and install from [arduino.cc/en/software](https://www.arduino.cc/en/software).

## 2. Add ESP32 board support

1. Open **File → Preferences**.
2. In **Additional boards manager URLs**, paste:

    ```text
    https://espressif.github.io/arduino-esp32/package_esp32_index.json
    ```

3. Open **Tools → Board → Boards Manager**, search `esp32`, and install
   **esp32 by Espressif Systems**.
4. Select your board under **Tools → Board → ESP32 Arduino**.
   TODO — instructor: confirm the exact board entry for the kit (commonly
   *ESP32 Dev Module*).

## 3. USB driver and port

The ESP32 talks over a USB-to-serial chip. Check the small square chip next to
the USB connector to know which driver you need.

=== "Windows"

    Windows usually installs the driver automatically. If no port appears under
    **Device Manager → Ports (COM & LPT)**:

    - Chip marked **CP2102** → install the Silicon Labs CP210x VCP driver.
    - Chip marked **CH340** / **CH9102** → install the WCH CH34x driver.

    Get drivers from the **manufacturer's own site**. Third-party driver sites
    that bundle installers are a known malware vector.

=== "macOS"

    Recent macOS includes both drivers. Check the port appeared:

    ```bash
    ls /dev/cu.*
    ```

    Look for `/dev/cu.usbserial-*` or `/dev/cu.SLAB_USBtoUART`. If nothing shows
    up, install the CP210x or CH34x driver, then approve it in
    **System Settings → Privacy & Security** — macOS blocks new drivers silently
    and you have to go press the button.

=== "Linux"

    The drivers are already in the kernel. The problem on Linux is **permissions**:

    ```bash
    ls -l /dev/ttyUSB* /dev/ttyACM*
    ```

    The port is owned by group `dialout`. Add yourself:

    ```bash
    sudo usermod -aG dialout $USER
    ```

    !!! warning "Log out and back in"
        Group membership is read at login. A new terminal is not enough — you
        must log out and in. Check with `groups` and look for `dialout`.

!!! warning "The single most common failure: a charge-only USB cable"
    Many USB cables carry power but no data. They charge a phone fine and are
    useless here — the board lights up and **no serial port ever appears**. If
    your board powers on but no port shows, suspect the cable before the driver.

!!! checkpoint "Checkpoint — port visible"
    Plug in the board and confirm a port appears under **Tools → Port**. Note its
    name (`COM4`, `/dev/ttyUSB0`, `/dev/cu.usbserial-0001`). Everything below
    depends on this.

## 4. Upload a test sketch

**File → New**, paste this, and upload with the **→** arrow:

```cpp title="setup_test.ino"
// TODO: confirm the onboard LED pin for the kit's board.
// GPIO 2 is correct for most ESP32 dev boards.
const int LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("ME 222 - setup OK");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
  Serial.printf("up %lu ms\n", millis());
}
```

Open **Tools → Serial Monitor** and set the baud rate to **115200**. You should
see:

```text
ME 222 - setup OK
up 1003 ms
up 2005 ms
```

??? failure "Upload fails — *Failed to connect to ESP32: Timed out waiting for packet header*"
    The board is not entering bootloader mode. In order of likelihood:

    1. **Charge-only cable.** Swap it.
    2. **Hold the BOOT button.** Press and hold `BOOT` (sometimes `IO0`), start
       the upload, release when "Connecting…" turns to dots. Many boards need
       this every time; it is not a fault.
    3. **Serial Monitor is open.** Only one program can hold the port. Close it.
    4. **USB hub.** Plug straight into the laptop.

??? failure "Serial Monitor shows garbage characters"
    Baud rate mismatch. Set the dropdown to **115200** to match
    `Serial.begin(115200)`.

??? failure "Nothing blinks but serial works"
    Your board's LED is on a different pin, or it has none. Try `LED_PIN = 5`.
    If serial works, your toolchain is fine — move on.

!!! checkpoint "Checkpoint — done"
    Blinking LED and uptime printing. That is the whole toolchain proven.

## Useful snippets

**Scan the I2C bus** — the first thing to run whenever a sensor goes quiet in
Labs 3–5:

```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);   // SDA, SCL
  Serial.println("scanning...");
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) Serial.printf("found 0x%02X\n", a);
  }
  Serial.println("done");
}

void loop() {}
```

**Non-blocking timing** — you need this from Lab 5 onward, once the car has to
sense and drive at the same time. `delay()` stops everything:

```cpp
unsigned long last = 0;
const unsigned long PERIOD = 20;   // ms

void loop() {
  unsigned long now = millis();
  if (now - last >= PERIOD) {
    last = now;
    // control loop body — runs every 20 ms
  }
}
```

**CSV logging** — use this format from Lab 5 so data pastes straight into a
spreadsheet:

```cpp
Serial.printf("%lu,%d,%d,%ld,%ld,%d\n", millis(), pwmL, pwmR, encL, encR, dist);
```
