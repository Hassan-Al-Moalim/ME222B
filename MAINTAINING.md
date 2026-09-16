# Maintaining this site

Notes for whoever is teaching or TA-ing ME 222. Students do not need this file.

## Local preview

```bash
python3 -m venv .venv && ./.venv/bin/pip install -r requirements.txt
./.venv/bin/mkdocs serve
```

Open <http://127.0.0.1:8000/ME222B/>. The browser live-reloads on save.

## Publishing

Push to `main`. `.github/workflows/deploy.yml` builds and publishes to GitHub
Pages, usually within a minute. The build runs `mkdocs build --strict`, so a
broken internal link fails CI rather than shipping a dead link to students.

The repository must stay **public** — GitHub Pages does not serve from private
repositories on the free plan.

## Where things live

| To change | Edit |
| --- | --- |
| A lab's instructions | `docs/labs/labNN-*.md` |
| Week dates, office hours | `docs/schedule.md` |
| Assessment, policies | `docs/syllabus.md` |
| Emergency info, LiPo rules | `docs/resources/safety.md` |
| Pinout, I2C addresses | `docs/resources/hardware.md` |
| Sidebar order, lab titles | `mkdocs.yml` |

## Outstanding placeholders

Course details that only the instructor can supply are marked `TODO`:

```bash
grep -rn "TODO" docs/ mkdocs.yml
```

Fill these before sharing the link with students — they are publicly visible:

- `docs/resources/safety.md` — emergency numbers, extinguisher and LiPo bucket locations
- `docs/schedule.md` — lab dates and room, exam dates, office hours
- `docs/syllabus.md` — office location and hours, integrity and AI policy, accommodations
- `docs/resources/hardware.md` — the USB-and-battery rule, verified against the boards
- `docs/labs/lab08-servo-thingsboard.md` — ThingsBoard URL, device tokens, WiFi

**Safety page.** The guidance there is standard practice for this class of
hardware. KAUST's EHS office has its own rules, emergency numbers and incident
reporting procedure, and those take precedence. Replace every `TODO` on that
page with the local specifics before students use it — Lab 1 is a full motion
lab with batteries.

**Unresolved: two lab plans.** The official syllabus lists six sensor-focused
labs (thermistor, motor voltage/current, rotary sensor, strain gauge,
closed-loop motor control, PID). This site follows a newer ten-week sequence
built around a four-motor robot car. They overlap but are not the same plan —
decide which is authoritative and reconcile before Week 1.

Labs 2–10 are structured outlines rather than finished worksheets. Lab 1 is
written out in full and is the model to follow.

## Writing a new lab

Conventions the site uses:

| Element | Markdown | When |
| --- | --- | --- |
| Safety-critical | `!!! danger` | Injury or destroyed hardware if ignored |
| Gotcha | `!!! warning` | Wrong results or wasted time |
| Aside | `!!! note` | Useful context, skippable |
| TA sign-off | `!!! checkpoint` | Stop until a TA verifies |
| Thinking prompt | `!!! question` | Open question to carry into the next lab |
| Fold-away fix | `??? failure "symptom"` | Troubleshooting, collapsed by default |
| Per-OS steps | `=== "Windows"` | Instructions differ by platform |

Use `!!! danger` sparingly. If everything is a danger, nothing is.

### Skeleton

````markdown
---
title: Lab N — Title
---

# Lab N — Title

<div class="lab-meta" markdown>
<div><span class="k">Duration</span><span class="v">90 minutes</span></div>
<div><span class="k">Depends on</span><span class="v">Lab N-1</span></div>
<div><span class="k">Milestone</span><span class="v">Deliverable</span></div>
</div>

## Objectives

1. ...

## Hardware required

Each group requires:

- ...

## Procedure

### 1. First stage

!!! checkpoint "Checkpoint 1 — what a TA verifies"
    State exactly what is checked and why it matters.

```cpp title="ME222_LabN.ino"
// Code students type or complete.
```

??? failure "A symptom they will actually hit"
    The fix, in order of likelihood.

## Deliverable

...
````

Then add the page to `nav:` in `mkdocs.yml` and add a row to `docs/schedule.md`.

### Writing notes

**Pre-lab work is real work.** If it is not checked, it will not get done, and a
90-minute session starts 20 minutes late.

**Checkpoints go where a mistake is expensive**, not at tidy intervals. Each one
costs TA throughput, so each should be preventing a dead part or an injury.

**Write the failure modes down.** The `??? failure` blocks are what a stuck
student actually reads. After each session, add the ones you saw.

**Say what the number should be.** "Measure the current" leaves students unable
to tell whether 2 A is right. "Expect 0.2–0.4 A unloaded" lets them catch their
own mistake.
