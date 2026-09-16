---
title: Lab Safety
---

# Lab Safety

Read this before your first lab. It covers the batteries, tools, and moving
hardware you will be working with.

## Emergency information

| | |
| --- | --- |
| Emergency number | **TODO** |
| Lab supervisor | **TODO** — name, phone |
| Nearest fire extinguisher | **TODO** — location, type |
| Fire blanket | **TODO** — location |
| LiPo fire bucket (sand) | **TODO** — location |
| Eye wash / first aid kit | **TODO** — location |
| Incident reporting | **TODO** — procedure |

Know these locations **before** you need them. You will be asked.

## Lithium polymer batteries

LiPo packs store a lot of energy and have no internal protection circuit. They
are safe when treated correctly and genuinely dangerous when not.

### Always

- [ ] **Inspect before every use.** Puffed, swollen, dented, or punctured pack → stop, bag it, tell a TA. Do not charge it, do not use it.
- [ ] **Charge in a LiPo-safe bag**, on a non-flammable surface, in the designated charging area.
- [ ] **Charge in balance mode** at the correct cell count. A 2S pack charged as 3S will be destroyed and may ignite.
- [ ] **Stay in the room while charging.** 
- [ ] **Store at storage charge**
- [ ] **Disconnect the pack** when you are not actively testing.

### Never

- Never short the leads. A LiPo pack across a dropped screwdriver will weld it and start a fire in under a second.
- Never charge a pack that is warm from use. Let it cool.
- Never leave a pack loose in a bag with metal objects.
- Never put a damaged pack in a normal bin.

### If a pack vents or catches fire

1. **Do not use water.** Do not use a CO₂ extinguisher on the cell itself.
2. Move people away. Lithium fires produce toxic smoke — clear the area.
3. Smother with sand if it can be done safely from a distance.
4. Pull the fire alarm and call **TODO**.
5. A vented pack that has stopped burning is still dangerous. Do not touch it.

## Powered motors and moving robots

- Wheels off the bench surface when testing on the bench.
- Tie back long hair; no loose sleeves, cords, or lanyards near a driveline.
- Keep fingers clear of gears, wheels, and shafts under power.
- Every robot needs a way to stop it **without chasing it** — a reachable kill switch or battery connector.

## Electrical

- Check polarity twice before connecting a supply. Reversed polarity destroys parts instantly and silently.
- Rewire only with power disconnected, not just switched off.
- Do not exceed **3.3 V** on any ESP32 GPIO. 5 V sensors need a level shifter.
- Lift the car so all four wheels spin free before the first powered test.
- Keep liquids off the bench entirely. No exceptions for water bottles.

## Housekeeping

- Clear bench at the end of every session. Loose wire offcuts short things.
- Damaged or suspect parts go to a TA with a note, not back in the bin.
- Batteries go back to the charging station at storage charge, in their bags.

## Reporting

Report **every** incident, including near-misses and ones where nothing was
damaged. 
