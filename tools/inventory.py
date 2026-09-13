#!/usr/bin/env python3
"""List every register seen in a capture with reply length, count and a sample payload.

usage: inventory.py capture.txt"""
import re, collections, sys
txt = open(sys.argv[1], encoding='latin-1').read()
frames = re.findall(r'UNK:((?:[0-9A-F]{2} )+) => ([0-9A-F]+)', txt)
regs = collections.OrderedDict()
for hexs, size in frames:
    b = hexs.split()
    if len(b) < 21: continue
    cmd = b[11]; reg = ' '.join(b[12:21])
    key = (reg, cmd)
    e = regs.setdefault(key, {'n':0,'len':len(b),'sample':hexs.strip(), 'samples':set()})
    e['n'] += 1
    e['samples'].add(' '.join(b[21:-1]))
print("frames:", len(frames), "distinct (reg,cmd):", len(regs))
for (reg, cmd), e in sorted(regs.items(), key=lambda kv: kv[0][0]):
    pl = ' '.join(e['sample'].split()[21:-1])
    print(f"{cmd} {reg} len=0x{e['len']:02X} ({e['len']:3d}) n={e['n']:3d} var={len(e['samples'])} payload: {pl}")
