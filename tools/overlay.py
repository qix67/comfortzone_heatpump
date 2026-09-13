#!/usr/bin/env python3
"""Overlay a named field layout on the memory image built from a capture and print the values.

usage: overlay.py capture.txt LAYOUT BANK ADDR      e.g. overlay.py capture.txt status01 0 0x0342

Layouts are the 1.6 status structs from comfortzone_frame.h. 1.6 also addressed the board's
memory as {bank, addr_lo, addr_hi}, and 2.50 keeps most of its blocks at nearly the same
addresses, so a 1.6 struct usually fits a 2.50 block after a small shift. Try shifts of a few
bytes until the known values (see PROTOCOL_2_50.md) land on their names.

Field kinds: t = °C*10 signed, f = Hz*10, pct = %*10, u8, u16, t[] = array of t, raw."""
import sys
from image import build

LAYOUTS = {
 # 1.6 R_REPLY_STATUS_01 (register 0B 07 00 7A 03), payload only. On 2.50 it fits at bank 0, 0x0342 up to hot_water_extra_time.
 'status01': [('unknown',2,'raw'),('extra_hot_water',1,'u8'),('hot_water_user_setting',2,'t'),('hot_water_hysteresis',2,'t'),
   ('hot_water_normal_steps',30,'t[]'),('hot_water_high_steps',30,'t[]'),('hot_water_extra_steps',30,'t[]'),
   ('unknown5a',1,'raw'),('hot_water_max_runtime',1,'u8'),('hot_water_pause_time',1,'u8'),('unknown5b',16,'t[]'),
   ('hot_water_compressor_min_frequency',2,'f'),('hot_water_compressor_max_frequency',2,'f'),('unknown6',4,'raw'),
   ('hot_water_extra_setting',2,'t'),('hot_water_extra_time',2,'u16'),('unknown7',7,'raw'),
   ('normal_fan_speed',2,'pct'),('reduce_fan_speed',2,'pct'),('fan_boost_increase',2,'pct'),('unknown8',16,'raw'),
   ('supply_fan_t12_adjust',2,'pct'),('unknown8a',6,'raw'),('fan_time_to_filter_change',2,'u16'),('unknown9',6,'raw'),
   ('heating_compressor_min_frequency',2,'f'),('heating_compressor_max_frequency',2,'f'),('unknown9a',1,'raw')],
}

fn, layout, bank, addr = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4], 0)
m = build(fn)[bank]
def s16(x):
    v = m.get(x, 0) | (m.get(x+1, 0) << 8); return v - 65536 if v > 32767 else v
a = addr
for name, size, kind in LAYOUTS[layout]:
    if kind in ('t', 'f', 'pct'):
        val = f"{s16(a)/10:.1f} " + {'t': '°C', 'f': 'Hz', 'pct': '%'}[kind]
    elif kind == 'u16': val = str(m.get(a, 0) | (m.get(a+1, 0) << 8))
    elif kind == 'u8': val = str(m.get(a, 0))
    elif kind == 't[]': val = ' '.join(f"{s16(a+i)/10:.1f}" for i in range(0, size, 2))
    else: val = bytes(m.get(a+i, 0) for i in range(size)).hex(' ')
    print(f"{a:04x} {name:36s} {val}")
    a += size
