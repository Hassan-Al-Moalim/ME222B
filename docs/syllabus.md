---
title: Syllabus
---

# Syllabus

**ME 222B — Mechatronics and Intelligent Systems**
Division of Mechanical Engineering · Fall 2026

<div class="lab-meta" markdown>
<div><span class="k">Semester</span><span class="v">30 Aug – 10 Dec 2026</span></div>
<div><span class="k">Lectures</span><span class="v">Sun &amp; Wed, 08:30–10:00</span></div>
<div><span class="k">Room</span><span class="v">Building 9, Classroom 4120</span></div>
<div><span class="k">Capacity</span><span class="v"> 18 students</span></div>
</div>

## Teaching team

| Role | Name | Email | Office | Office hours |
| --- | --- | --- | --- | --- |
| Instructor | Amr Talaat Abdel Hamid | [amr.abdelhamid@kaust.edu.sa](mailto:amr.abdelhamid@kaust.edu.sa) | *Pending* | *Pending* |
| Teaching Assistant | Hassan Al Moalim | [hassan.moalim@kaust.edu.sa](mailto:hassan.moalim@kaust.edu.sa) | *Pending* | *Pending* |

## Course description

This course introduces students to the practical application of a wide range of
sensors and motors commonly used in mechatronic systems. Emphasis is placed on
the integration of these components into embedded systems found in both consumer
and industrial products.

Students gain hands-on experience building functional systems that acquire
real-time sensor data, apply signal filtering techniques, and analyze the
resulting information. Students design basic schematics, connect and interface
sensors and actuators to embedded platforms, and develop firmware for system
operation. The course also provides insights into how industrial systems are
specified, designed, and evaluated.

## Learning outcomes

By the end of the course you should be able to:

1. Specify and apply appropriate sensors and actuators in the design of real-time mechatronic systems.
2. Develop embedded hardware and firmware solutions to acquire, process, and respond to physical measurements.
3. Design and implement signal conditioning circuits using operational amplifiers, filters, and analog interfaces to ensure accurate data acquisition.
4. Analyze and integrate different types of sensors, including thermistors, ultrasonic sensors, strain gauges, and rotary encoders.
5. Implement motor control systems — DC, servo, and stepper — with closed-loop feedback using embedded platforms.
6. Apply real-time signal processing techniques, such as FFT and smoothing filters, for sensor data analysis within embedded systems.
7. Develop schematics and lay out printed circuit boards (PCBs) for embedded mechatronic prototypes.
8. Simulate and model mechanical systems such as spring–mass–damper systems using Simulink and other tools.
9. Integrate and optimize embedded systems for low-power applications, demonstrating awareness of sustainability and efficient resource use.
10. Work collaboratively on open-ended engineering problems, culminating in a functional mechatronic project that reflects real-world industrial practices.

## Assessment

| Component | Weight |
| --- | --: |
| Course project(s) | **45 %** |
| Midterm exam | **20 %** |
| Final exam | **20 %** |
| Quizzes | **10 %** |
| Homework / assignments | **5 %** |
| **Total** | **100 %** |

## Required knowledge

- Basic knowledge of **assembly and C programming**, **digital logic design**, and **basic computer architecture**. Ideally a first course in each; alternatively, a completed laboratory-based undergraduate mechatronics class.
- Experience with simulation and circuit analysis tools such as **LTspice**, **OrCAD Capture**, **OrCAD PSpice**, **MATLAB**, or **Simulink**.

## Textbooks and materials

All four are available free of charge as e-books through the KAUST library.

- **Handbook of Modern Sensors**, Jacob Fraden, 5th ed., Springer.
  ISBN 978-3-319-19302-1 (hardcover), 978-3-319-19303-8 (ebook).
  [Springer](https://link.springer.com/book/10.1007%2F978-3-319-19303-8)
- **Industry 4.0**, Alasdair Gilchrist. ISBN 978-1-4842-2046-7.
  [Springer](https://link.springer.com/book/10.1007%2F978-1-4842-2047-4)
- **Computers as Components — Principles of Embedded Computing System Design**, Marilyn Wolf, 4th ed.
- **What Every Engineer Should Know about Developing Real-Time Embedded Products**, Kim R. Fowler.
  [Taylor & Francis](https://www.taylorfrancis.com/books/9780849379635)

Weekly online reading and videos are assigned and shared with students.

## Nature of the assignments

Homework problems cover class materials.

Lab experiments and **two course projects** are assigned. Labs give practical
experience prototyping embedded systems that interface with sensors and
actuators. All labs are conducted using an embedded platform. **Students must
demo their lab results and submit reports.**

### Planned labs

- **Thermistor-based temperature measurement** — sensor interfacing, analog signal conditioning, analog-to-digital conversion.
- **Motor voltage and current monitoring** — analyzing the electrical behaviour of motors using op-amp circuits and filtering.
- **Rotary sensor interfacing** — position measurement and motion tracking using rotary encoders and digital pulse counting.
- **Strain gauge measurement** — Wheatstone bridge configurations and low-level signal amplification for force or pressure sensing.
- **Closed-loop motor control** — feedback systems using sensor inputs such as encoders or potentiometers to control motor behaviour in real time.
- **PID control implementation** — proportional-integral-derivative strategies for precise motion and process control.

!!! warning "Two lab plans are in circulation"
    The list above is the one printed in the official syllabus. The
    [lab schedule on this site](schedule.md) follows a different, newer
    ten-week sequence built around a four-motor autonomous robot car.

    The two overlap substantially — rotary encoders, current monitoring,
    closed-loop control and PID all appear in both — but they are not the same
    plan. **TODO — instructor: decide which is authoritative and reconcile the
    two before Week 1.** Students should not have to guess.

## Course policies

### Late work

**No late homework will be accepted.**

### Collaboration

Consultation with other students on the problems is permitted, but **each student
must submit their own personal solution**. Two identical solutions will not be
accepted.

### Homework grading

| Outcome | Credit |
| --- | --- |
| Complete and legible | Full credit |
| Not well done | Half credit |
| Not submitted | No credit |

### Academic integrity

TODO — instructor: paste KAUST's official statement.

Course-specific guidance worth stating for a lab course:

- **Code** — discussing approaches across groups is fine; copying another group's source is not. Cite any code adapted from a library example or datasheet.
- **Data** — report the numbers your hardware actually produced. A result that disagrees with theory is a finding, not a failure; fabricating a clean number is misconduct.
- **AI tools** — TODO: state the policy explicitly. Students will ask.

### Accommodations

TODO — instructor: paste KAUST's official statement and the office contact.

### Safety

Lab safety is a **pass/fail gate**, not a graded component. A student who has not
completed the [safety briefing](resources/safety.md) sign-off does not work with
batteries or powered motors. See that page for the full rules.

---

!!! note "Syllabus changes"
    The instructor reserves the right to make changes to this syllabus as
    necessary.
