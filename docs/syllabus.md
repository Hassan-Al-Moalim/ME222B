---
title: Syllabus
---

# Syllabus

!!! warning "Draft — instructor input needed"
    The structure below is a scaffold. Percentages, policies, and dates are
    marked `TODO` and must be set by the instructor before this page goes live.
    Where your institution has a mandated policy text (academic integrity,
    accommodations), paste the official wording rather than paraphrasing it.

## Course description

TODO — one paragraph. What the course covers and what a student can do at the end.

Suggested framing based on the lab sequence: over ten 90-minute sessions students
bring up a four-motor ESP32 robot car, characterise and then correct its motion
with closed-loop control, add distance and inertial sensing, connect it to a
telemetry dashboard, and finish by coordinating multiple vehicles in a shared
arena.

## Learning outcomes

By the end of the course, a student should be able to:

1. Bring up a microcontroller-controlled vehicle safely and verify each subsystem before relying on it.
2. Drive DC motors under PWM through an H-bridge, and explain why identical commands produce different behaviour.
3. Characterise a physical actuator experimentally — dead zone, saturation, and unit-to-unit variation.
4. Integrate distance and inertial sensors, calibrate them against known references, and quantify their noise and bias.
5. Design and tune a closed-loop controller (P, PI, PID) against a stated performance spec.
6. Estimate vehicle position and heading, and explain how dead-reckoning error accumulates.
7. Connect an embedded system to a telemetry platform and implement a mission state machine.
8. Coordinate multiple autonomous vehicles safely in a shared space.
9. Document an engineering build and report failures honestly enough that a peer could reproduce the result.

## Prerequisites

TODO — list the required prior courses.

Assumed working knowledge: basic circuits (Ohm's law, voltage dividers, reading a
datasheet) and programming in C or C++ at the level of loops, functions, arrays,
and structs. No prior embedded experience is assumed — [Setup](resources/setup.md)
starts from an empty machine.

## Assessment

| Component | Weight | Notes |
| --- | --- | --- |
| Weekly lab milestones (Weeks 1–9) | TODO % | One deliverable per session — see [Schedule](schedule.md) |
| Lab participation & checkpoints | TODO % | Attendance plus TA sign-offs |
| Final fleet challenge (Week 10) | TODO % | Live demo + performance report, team-assessed |
| TODO — quizzes/exam? | TODO % | |
| **Total** | **100 %** | |

## Teams

Labs are done in **groups**, one car per group. TODO — confirm group size and
state how groups are formed and whether they are fixed for the semester.

Both members must be able to explain any part of the build. Checkpoints may be
directed at either team member.

## Lab reports

Unless a lab says otherwise, a report is **at most 4 pages** and contains:

- What you built, with a photo of your actual wiring — not a stock diagram.
- The measurements you were asked to take, in a table with units.
- Your analysis of those measurements, including anything that disagreed with theory.
- Any part you damaged and what you think caused it. **Reporting damage honestly
  costs you nothing.** Hiding it is an integrity matter.

Submit as PDF on Canvas. TODO — confirm submission channel and deadline convention.

## Attendance and make-ups

TODO — state the policy. Lab courses usually need a stricter one than lecture
courses because the hardware is only available during scheduled sessions.

## Late work

TODO — state the policy.

## Academic integrity

TODO — paste your institution's official statement.

Course-specific guidance worth stating explicitly for a lab course:

- **Code**: discussing approaches across teams is fine; copying another team's
  source is not. Cite any code you adapt from a library example or datasheet.
- **Data**: report the numbers your robot actually produced. A result that
  disagrees with theory is a finding, not a failure — fabricating a clean number
  is misconduct.
- **AI tools**: TODO — state your policy explicitly. Students will ask.

## Accommodations

TODO — paste your institution's official statement and the office contact.

## Safety

Lab safety is a **pass/fail gate**, not a graded component. A student who has not
completed the [safety briefing](resources/safety.md) sign-off does not work with
batteries or powered motors. Since Week 1 is already a full motion lab, the
sign-off must happen before the first session. See that page for the full rules.
