#!/usr/bin/env python3
"""Per-cycle table of the known values plus any non-periodic panel writes (events).

usage: timeline.py capture.txt [every_n]"""
import sys
from frames import *
cy = cycles(load(sys.argv[1])); every = int(sys.argv[2]) if len(sys.argv) > 2 else 3
print("idx time     TE0  TE1  TE2  TE3  TE4  TE5  TE7 TE24 | freq b3 | Pcomp Pin | freq_req setpt | events")
for i, c in enumerate(cy):
    x = c['x88']; r = c['r']
    te = [s16(x, 52 + 2*k)/10 for k in (0, 1, 2, 3, 4, 5, 7, 24)]
    x51 = r.get('01 D1 02'); ba = r.get('01 40 03')
    ev = [k for k in c['w'] if k not in PERIODIC_WRITES]
    if i % every and not ev:
        continue
    fq = [u16(p, 0) for p in c['w'].get('01 D9 03', [])]; sp = [u16(p, 0) for p in c['w'].get('01 E1 03', [])]
    print(f"{i:3d} {x[0]:02d}:{x[1]:02d}:{x[2]:02d} " + ' '.join(f"{v:5.1f}" for v in te)
          + f" | {s16(x51,33)/10 if x51 else '-':>5} {x51[3] if x51 else '-'} | {u16(ba,0) if ba else '-':>5} {u16(ba,14) if ba else '-':>4} | {fq} {sp} | {ev}")
