#!/usr/bin/env python3
"""For every reply register, list the 16-bit fields that change over the capture and the known
value they track best (exact copy '==' or Pearson correlation '~').

usage: correlate.py capture.txt [reg ...]      e.g. correlate.py capture.txt '01 D1 02'"""
import sys, math
from frames import *
cy = cycles(load(sys.argv[1])); ref = references(cy)
want = sys.argv[2:]

def corr(a, b):
    n = len(a); ma = sum(a)/n; mb = sum(b)/n
    sa = math.sqrt(sum((x-ma)**2 for x in a)); sb = math.sqrt(sum((y-mb)**2 for y in b))
    return 0 if sa == 0 or sb == 0 else sum((x-ma)*(y-mb) for x, y in zip(a, b))/(sa*sb)

for reg in sorted({k for c in cy for k in c['r']}):
    if want and reg not in want:
        continue
    idx = [i for i, c in enumerate(cy) if reg in c['r']]
    if len(idx) < 10:
        continue
    L = min(len(cy[i]['r'][reg]) for i in idx)
    rows = []
    for o in range(L - 1):
        v = {i: s16(cy[i]['r'][reg], o) for i in idx}
        if len(set(v.values())) < 3:
            continue
        best = (None, 0, 0)
        for name, rv in ref.items():
            common = [i for i in v if i in rv]
            if len(common) < 10:
                continue
            a = [v[i] for i in common]; b = [rv[i] for i in common]
            eq = sum(1 for x, y in zip(a, b) if x == y)/len(common); r = corr(a, b)
            if (eq, abs(r)) > (best[1], abs(best[2])):
                best = (name, eq, r)
        name, eq, r = best
        tag = f"== {name}" if eq > 0.9 else (f"~ {name} r={r:+.2f}" if abs(r) > 0.9 else "?")
        vals = [v[i] for i in sorted(v)]
        rows.append(f"  off {o:3d}: {tag:30s} min {min(vals):6d} max {max(vals):6d} distinct {len(set(vals)):3d}  {vals[:4]} .. {vals[-3:]}")
    if rows:
        print(f"\n### reg {reg} ({L} byte payload)"); print('\n'.join(rows))
