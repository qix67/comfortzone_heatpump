"""Shared helpers: parse a capture into frames and group them into control panel polling cycles."""
import re, struct, collections

def load(fn):
    """Return [(cmd, reg, payload)] for every 'UNK:' hex dump in a capture file.
    cmd is one of R/W/r/w, reg is the last 3 register bytes as 'XX XX XX', payload excludes header and crc."""
    txt = open(fn, encoding='latin-1').read().replace('\r', '')
    out = []; prev = None
    # "UNK:<hex> => n" (unknown register), "RAW:<hex> => n" (every frame, newer builds),
    # "RAW R_REPLY_...: <hex>" (status decoders in DEBUG mode)
    for m in re.finditer(r'^(?:UNK:|RAW:|RAW [A-Za-z0-9_]+: )((?:[0-9A-F]{2} ?)+)', txt, re.M):
        b = bytes.fromhex(m.group(1).replace(' ', ''))
        if len(b) < 22 or b[10] != len(b):
            continue
        if b == prev:          # same frame dumped by two printers
            continue
        prev = b
        out.append((chr(b[11]), b[18:21].hex(' ').upper(), b[21:-1]))
    return out

def cycles(frames, anchor='01 04 00', anchor_len=114):
    """Split frames into cycles, each starting with the 0x88 time/sensor frame (2.50 register 01 04 00).
    Each cycle is {'x88': payload, 'r': {reg: payload}, 'w': {reg: [payloads]}}."""
    out = []; cur = None
    for cmd, reg, pl in frames:
        if cmd == 'r' and reg == anchor and len(pl) == anchor_len:
            cur = {'x88': pl, 'r': {}, 'w': {}}; out.append(cur); continue
        if cur is None:
            continue
        if cmd == 'r':
            cur['r'][reg] = pl
        elif cmd == 'W':
            cur['w'].setdefault(reg, []).append(pl)
    return out

def s16(p, o): return struct.unpack('<h', p[o:o+2])[0]
def u16(p, o): return struct.unpack('<H', p[o:o+2])[0]
def u32(p, o): return struct.unpack('<I', p[o:o+4])[0]

PERIODIC_WRITES = {'01 D9 03', '01 DB 03', '01 E1 03', '01 68 05'}

def references(cycles_):
    """Known values per cycle index, used as correlation targets."""
    ref = collections.defaultdict(dict)
    for i, c in enumerate(cycles_):
        x = c['x88']; r = c['r']
        for k in range(31):
            ref[f'TE{k}'][i] = s16(x, 52 + 2*k)
        ref['clock_sec'][i] = x[0]*3600 + x[1]*60 + x[2]
        x51 = r.get('01 D1 02')
        if x51:
            ref['comp_freq'][i] = s16(x51, 33); ref['x51_byte3'][i] = x51[3]
        ba = r.get('01 40 03')
        if ba:
            ref['P_comp'][i] = u16(ba, 0); ref['P_in'][i] = u16(ba, 14)
        for reg, name in (('01 D9 03', 'panel_freq_req'), ('01 E1 03', 'panel_setpoint')):
            if reg in c['w']:
                ref[name][i] = u16(c['w'][reg][-1], 0)
    return ref
