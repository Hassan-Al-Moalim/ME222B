"""ME222 Lab 7 - SLAM on the laptop.

The car sends every lidar turn and its gyro heading over Wi-Fi (the SLAM
add-on in the ThingsBoard sketch). This script builds a map of the room and
works out where the car is in it, at the same time, and draws both live.

How it works, once per lidar turn:
  1. Guess the new pose: the old position, turned by what the gyro says.
  2. Scan matching: try small shifts and turns around that guess and keep
     the one where the scan points land best on walls already in the map.
  3. Add the scan to the map: cells where the beam ended are walls, cells
     the beam passed through are free.

Install once:   pip install numpy matplotlib
Run:            python lab7_slam.py
Record too:     python lab7_slam.py --record run1.bin
Replay later:   python lab7_slam.py --replay run1.bin

In the map window: press  s  to save the map as a PNG, r  to start a new map.
Windows asks whether Python may use the network the first time: allow it on
private networks, or the car's packets never arrive.
"""
import argparse
import math
import socket
import struct
import time

import numpy as np
import matplotlib.pyplot as plt

PORT = 5005             # same as SLAM_PORT in the sketch
RES = 0.05              # map cell size, m
SIZE_M = 20.0           # map is SIZE_M x SIZE_M, car starts in the middle
MAX_RANGE = 6.0         # ignore points farther than this, m
MIN_POINTS = 60         # skip turns with fewer valid points
TURNING_DEG = 6.0       # heading change per turn above which the scan is smeared

STATES = ["READY", "CALIBRATING", "DRIVING", "TURNING", "BLOCKED", "FINISHED", "STOPPED"]
PKT = struct.Struct("<4sIfB3x720s")
COS = np.cos(np.radians(np.arange(360)))
SIN = np.sin(np.radians(np.arange(360)))


def decode(data):
    """Packet -> (turn number, heading deg, state name, points (N,2) in the car frame, m)."""
    if len(data) != PKT.size:
        return None
    magic, seq, heading, state, raw = PKT.unpack(data)
    if magic != b"L7S1":
        return None
    mm = np.frombuffer(raw, dtype="<u2").astype(float)
    ok = (mm > 0) & (mm < MAX_RANGE * 1000)
    b = np.nonzero(ok)[0]
    r = mm[ok] / 1000.0
    # bearing b: 0 front, 90 right (clockwise). Car frame: x forward, y left.
    pts = np.column_stack((r * COS[b], -r * SIN[b]))
    name = STATES[state] if state < len(STATES) else str(state)
    return seq, heading, name, pts


class Slam:
    def __init__(self):
        self.n = int(SIZE_M / RES)
        self.reset()

    def reset(self):
        self.logodds = np.zeros((self.n, self.n), dtype=np.float32)
        self.field = np.zeros((self.n, self.n), dtype=np.float32)
        self.pose = np.zeros(3)           # x, y (m), theta (rad, positive = left)
        self.path = [self.pose[:2].copy()]
        self.last_heading = None
        self.last_state = None
        self.turns = 0

    # ---- grid helpers ----
    def cells(self, xy):
        return np.floor(xy / RES + self.n / 2).astype(int)

    def inside(self, ij):
        return (ij[..., 0] >= 0) & (ij[..., 0] < self.n) & (ij[..., 1] >= 0) & (ij[..., 1] < self.n)

    def to_world(self, pts, pose):
        c, s = math.cos(pose[2]), math.sin(pose[2])
        return np.column_stack((pose[0] + c * pts[:, 0] - s * pts[:, 1],
                                pose[1] + s * pts[:, 0] + c * pts[:, 1]))

    def update_field(self):
        """Blurred wall map used for matching: highest on a wall, falling off over ~10 cm."""
        occ = (self.logodds > 1.0).astype(np.float32)
        f = np.zeros_like(occ)
        total = 0.0
        for di in range(-3, 4):
            for dj in range(-3, 4):
                w = math.exp(-(di * di + dj * dj) / 2.0)
                f += w * np.roll(np.roll(occ, di, 0), dj, 1)
                total += w
        self.field = np.minimum(f / total * 4.0, 1.0)

    # ---- scan matching ----
    def score(self, pts, x, y, th):
        """Sum of the wall map under the scan, for many (x, y) at one theta."""
        c, s = math.cos(th), math.sin(th)
        wx = c * pts[:, 0] - s * pts[:, 1]
        wy = s * pts[:, 0] + c * pts[:, 1]
        gx = (wx[None, :] + x[:, None]) / RES + self.n / 2
        gy = (wy[None, :] + y[:, None]) / RES + self.n / 2
        i = np.clip(gx.astype(int), 0, self.n - 1)
        j = np.clip(gy.astype(int), 0, self.n - 1)
        return self.field[i, j].mean(axis=1)

    def search(self, pts, guess, span_xy, step_xy, span_th, step_th):
        offs = np.arange(-span_xy, span_xy + 1e-9, step_xy)
        dx, dy = np.meshgrid(offs, offs)
        dx, dy = dx.ravel(), dy.ravel()
        best = (-1e9, guess)
        for dth in np.arange(-span_th, span_th + 1e-9, step_th):
            th = guess[2] + dth
            sc = self.score(pts, guess[0] + dx, guess[1] + dy, th)
            # small pull towards the guess, so ties and flat corridors do not slide
            sc = sc - 0.02 * ((dx * dx + dy * dy) / 0.1 ** 2 + (dth / math.radians(2)) ** 2)
            k = int(np.argmax(sc))
            if sc[k] > best[0]:
                best = (sc[k], np.array([guess[0] + dx[k], guess[1] + dy[k], th]))
        return best

    # ---- map update ----
    def insert(self, pts, pose):
        ends = self.to_world(pts, pose)
        # free cells along each beam, sampled every half cell, stopping short of the end
        start = pose[:2]
        d = ends - start
        r = np.hypot(d[:, 0], d[:, 1])
        k = np.maximum((r / (RES / 2)).astype(int) - 2, 0)
        idx = np.repeat(np.arange(len(r)), k)
        t = (np.concatenate([np.arange(m) for m in k]) if k.sum() else np.zeros(0)) * (RES / 2)
        if len(idx):
            frac = t / r[idx]
            free = start + d[idx] * frac[:, None]
            fc = np.unique(self.cells(free), axis=0)
            fc = fc[self.inside(fc)]
            self.logodds[fc[:, 0], fc[:, 1]] -= 0.4
        oc = np.unique(self.cells(ends), axis=0)
        oc = oc[self.inside(oc)]
        self.logodds[oc[:, 0], oc[:, 1]] += 0.9
        np.clip(self.logodds, -4, 6, out=self.logodds)
        self.update_field()

    # ---- one lidar turn ----
    def step(self, heading_deg, state, pts):
        if len(pts) < MIN_POINTS:
            return "too few points"
        moving = ("DRIVING", "TURNING")
        if self.last_heading is None or state not in moving or self.last_state not in moving:
            # stopped, or the heading was just reset at START: let scan matching find any turn
            dh = 0.0
        else:
            dh = heading_deg - self.last_heading
        self.last_heading = heading_deg
        self.last_state = state
        guess = self.pose.copy()
        guess[2] += math.radians(dh)
        self.turns += 1
        if self.turns == 1:
            self.insert(pts, guess)
            return "first scan"
        if abs(dh) > TURNING_DEG:
            # turning on the spot: the scan is smeared, trust the gyro, do not draw
            self.pose = guess
            return "turning"
        _, p = self.search(pts, guess, 0.20, 0.025, math.radians(5), math.radians(1))
        _, p = self.search(pts, p, 0.025, 0.01, math.radians(1), math.radians(0.25))
        self.pose = p
        self.path.append(p[:2].copy())
        self.insert(pts, p)
        return "matched"

    def probability(self):
        return 1.0 - 1.0 / (1.0 + np.exp(self.logodds))


def packets_live(port, record):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", port))
    sock.setblocking(False)
    out = open(record, "wb") if record else None
    host = socket.gethostbyname(socket.gethostname())
    print(f"listening on UDP port {port}. This laptop's IP is probably {host} "
          f"(check with ipconfig). Put it in LAPTOP_IP in the sketch.")
    while True:
        try:
            data, _ = sock.recvfrom(2048)
        except BlockingIOError:
            yield None
            continue
        if out:
            out.write(struct.pack("<dH", time.time(), len(data)) + data)
            out.flush()
        yield data


def packets_replay(path):
    with open(path, "rb") as f:
        while True:
            head = f.read(10)
            if len(head) < 10:
                return
            _, n = struct.unpack("<dH", head)
            yield f.read(n)


def main():
    ap = argparse.ArgumentParser(description="ME222 Lab 7 laptop SLAM")
    ap.add_argument("--port", type=int, default=PORT)
    ap.add_argument("--record", help="also save the packets to this file")
    ap.add_argument("--replay", help="play back a recorded file instead of listening")
    ap.add_argument("--save", help="replay only: save the final map to this PNG and exit")
    args = ap.parse_args()

    slam = Slam()
    source = packets_replay(args.replay) if args.replay else packets_live(args.port, args.record)

    show = not args.save
    if show:
        plt.ion()
    fig, ax = plt.subplots(figsize=(8, 8))
    ext = [-SIZE_M / 2, SIZE_M / 2, -SIZE_M / 2, SIZE_M / 2]
    img = ax.imshow(slam.probability().T, origin="lower", extent=ext, cmap="gray_r", vmin=0, vmax=1)
    path_line, = ax.plot([], [], "-", color="tab:blue", lw=1.5, label="path")
    scan_pts, = ax.plot([], [], ".", color="tab:red", ms=2, label="this scan")
    car, = ax.plot([], [], "o", color="tab:green", ms=8)
    nose, = ax.plot([], [], "-", color="tab:green", lw=2)
    ax.set_xlabel("x (m), forward at START")
    ax.set_ylabel("y (m), left at START")
    ax.legend(loc="upper right")
    title = ax.set_title("waiting for the car...")
    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)

    def on_key(ev):
        if ev.key == "s":
            name = time.strftime("lab7_map_%H%M%S.png")
            fig.savefig(name, dpi=150)
            print("saved", name)
        elif ev.key == "r":
            slam.reset()
            print("new map")
    fig.canvas.mpl_connect("key_press_event", on_key)

    last_draw, last_seq, got = 0.0, None, 0
    for data in source:
        if data is not None:
            pkt = decode(data)
            if pkt is None:
                continue
            seq, heading, state, pts = pkt
            if last_seq is not None and seq < last_seq:      # car restarted
                slam.reset()
            last_seq = seq
            what = slam.step(heading, state, pts)
            got += 1
            scan_world = slam.to_world(pts, slam.pose) if len(pts) else np.zeros((0, 2))
            info = f"turn {seq}  {state}  heading {heading:6.1f} deg  {len(pts)} pts  {what}"
        if not show:
            continue
        now = time.time()
        if data is None or now - last_draw > 0.2:
            if got:
                img.set_data(slam.probability().T)
                p = np.array(slam.path)
                path_line.set_data(p[:, 0], p[:, 1])
                scan_pts.set_data(scan_world[:, 0], scan_world[:, 1])
                x, y, th = slam.pose
                car.set_data([x], [y])
                nose.set_data([x, x + 0.25 * math.cos(th)], [y, y + 0.25 * math.sin(th)])
                title.set_text(info)
                # keep everything seen so far in view
                occ = np.argwhere(slam.logodds > 1.0)
                if len(occ):
                    lo = (occ.min(axis=0) - slam.n / 2) * RES - 0.5
                    hi = (occ.max(axis=0) - slam.n / 2) * RES + 0.5
                    ax.set_xlim(min(lo[0], x - 1), max(hi[0], x + 1))
                    ax.set_ylim(min(lo[1], y - 1), max(hi[1], y + 1))
                    ax.set_aspect("equal")
            fig.canvas.draw_idle()
            plt.pause(0.001 if data is not None else 0.02)
            last_draw = now
            if not plt.fignum_exists(fig.number):
                return

    # replay finished
    img.set_data(slam.probability().T)
    p = np.array(slam.path)
    path_line.set_data(p[:, 0], p[:, 1])
    occ = np.argwhere(slam.logodds > 1.0)
    if len(occ):
        lo = (occ.min(axis=0) - slam.n / 2) * RES - 0.5
        hi = (occ.max(axis=0) - slam.n / 2) * RES + 0.5
        ax.set_xlim(lo[0], hi[0])
        ax.set_ylim(lo[1], hi[1])
        ax.set_aspect("equal")
    title.set_text(f"{got} turns, end pose x {slam.pose[0]:.2f} m  y {slam.pose[1]:.2f} m  "
                   f"heading {math.degrees(slam.pose[2]):.1f} deg")
    if args.save:
        fig.savefig(args.save, dpi=150)
        print("saved", args.save)
        np.savetxt(args.save + ".path.csv", np.array(slam.path), delimiter=",", fmt="%.4f")
    else:
        plt.ioff()
        plt.show()


if __name__ == "__main__":
    main()
