---
title: Lab 8 — Payload Servo + ThingsBoard Mission Dispatch
---

# Lab 8 — Payload Servo + ThingsBoard Mission Dispatch

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">90 minutes</span></div>
<div><span class="k">Depends on</span><span class="v">Lab 7</span></div>
<div><span class="k">Milestone</span><span class="v">Dashboard sends mission → car executes → payload release</span></div>
<div><span class="k">Status</span><span class="v">Outline — to be written</span></div>
</div>

## Practical focus

Add a simple servo release mechanism; connect the car to ThingsBoard; report
vehicle state; receive pickup and destination; implement mission states.

!!! warning "Infrastructure to arrange before this session"
    TODO — instructor, all of these block the lab if unresolved:

    - ThingsBoard instance — hosted at `thingsboard.cloud` or self-hosted? URL?
    - Device provisioning: one device per car, with access tokens issued in advance.
    - **Campus WiFi.** ESP32 WiFi commonly cannot join enterprise WPA2-Enterprise
      networks (eduroam-style) without extra configuration. Test this on the
      actual network before the session, or provide a dedicated AP.

## Objectives

1. Drive a servo to release a payload at a commanded moment.
2. Connect the ESP32 to WiFi and to a ThingsBoard instance.
3. Publish vehicle telemetry (state, position, battery, sensor readings).
4. Subscribe to mission commands containing pickup and destination.
5. Implement the mission as an explicit **state machine**.

## Procedure outline

TODO — expand.

1. Mount and test the servo standalone. Find the open and closed angles.
2. Connect to WiFi. Confirm with a printed IP address.
3. Publish one telemetry key and watch it appear on the dashboard.
4. Receive one RPC command and act on it.
5. Build the state machine:

    ```text
    IDLE ──► MISSION_RECEIVED ──► DRIVING_TO_PICKUP ──► AT_PICKUP
                                                           │
                    ┌──────────────────────────────────────┘
                    ▼
          DRIVING_TO_DESTINATION ──► AT_DESTINATION ──► RELEASING ──► IDLE
    ```

!!! tip "Make the state visible"
    Publish the current state name as telemetry and show it on the dashboard.
    When a car misbehaves in Lab 9, the state readout is how you find out why
    without stopping it.

!!! danger "Servo current on a shared rail"
    A stalled servo draws far more than its idle current and can brown out the
    ESP32 mid-mission. Confirm the servo is powered from the battery rail, not
    the ESP32's regulator.

## Deliverable

**Dashboard sends mission → car executes → payload release.**
