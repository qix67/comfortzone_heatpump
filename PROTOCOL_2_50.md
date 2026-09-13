# Protocol 2.50 notes

Reverse engineering notes for Comfortzone control panel firmware 2.50, captured on an EX50
in September 2026. Everything here comes from bus snooping with the `DEBUG` frame dump and the
scripts in `tools/`. Confidence is marked per item.

## Addressing

Register bytes 6..8 are `{bank, addr_lo, addr_hi}`. The read command parameter is the number
of bytes to read. Bank byte flags: `0x80`/`0x81` = set bits, `0x40`/`0x41` = clear bits of the
16-bit word at that address (the write value is the mask, or its complement for clear). The
map is the 2.21 map with a few bytes inserted, so 2.21 frames reappear at an address shifted
by 0 to 5 bytes.

The control panel polls about 25 registers every 3 seconds and does the regulation itself: it
writes a compressor frequency request and a temperature setpoint to the heat pump board every
cycle. The heat pump board is mostly a sensor and actuator interface.

Once a minute the panel additionally reads the board's whole memory in contiguous 178-byte
blocks (bank 0: 0x00B1..0x067E, bank 1: 0x0000..0x0573, plus pieces of banks 3, 4, 5).
Navigating the panel menus causes no extra reads, so menu pages cannot be matched to
registers by timing. Instead, `tools/image.py` assembles the blocks into a memory image and
values read off the panel are searched in it (`tools/overlay.py` prints a named layout at an
address).

**The 1.6 firmware used the same addressing.** Its "Status NN" registers are bank/address
pairs too (Status 01 = bank 0, 0x037A) and the 2.50 map keeps most blocks within a few bytes
of the 1.6 addresses. The 1.6 structs in `comfortzone_frame.h` therefore name many 2.50
fields once the local shift is found: the hot water settings block is 1.6 Status 01 shifted
by -0x38 and holds byte for byte up to `hot_water_extra_time`.

## Decoded (in the library)

| Register | Bytes | Content | Confidence |
|---|---|---|---|
| `00 89 02` | 0xC3 | settings; room heating setting at offset 9 | high (matches 2.21) |
| `01 04 00` | 0x88 | time, date, 31 sensors, status flags | high |
| `01 D1 02` | 0x51 | compressor frequency at offset 33 | high |
| `00 45 03` | 0xC8 / 2 | hot water settings block (1.6 Status 01 layout): user setting, hysteresis, three 15-step tables, max runtime, pause, compressor min/max, extra setting and time | high, values confirmed on the panel |
| `05 28 00` | 0x2E | energies and runtimes | high (matches 1.8 and 2.21) |
| `00 F7 03` | 0x57 | filter change countdown in days at offset 6 | medium |
| `01 40 03` | 0xBA | powers (offsets 0, 2, 6, 10, 14) | high (matches 2.21) |
| `81/41 1D 00` | write | extra hot water on/off, bit 0x0040 | high, seen from panel |
| `01 1D 00` | 2 | flag word, bit 0x0040 = extra hot water | high |
| `80/40 0E 00` | write | fan boost on/off, bit 0x0200 | high, seen from panel |
| `80/40 12 00` | write | cooling mode on/off, bit 0x0400 (reply 0x7F/0x7B) | high, seen from panel |
| `00 12 00` | 2 | mode flags word, bit 0x0400 = cooling mode | high |
| `81/41 13 00` | write | fireplace mode on/off, bit 0x0400 (replies 0x04/0x08), same as 1.8 and 2.21 | high, seen from panel |
| `01 13 00` | 2 | flags word, bit 0x0400 = fireplace mode | high |
| `80/40 00 00` | write | automatic daylight saving on/off, bit 0x0040 (reply 0x8E), same as 1.6 | high, seen from panel |
| `00 00 00` | 0x2E | system flags word (bit 0x0040 = automatic daylight saving) + 22 constant bytes | high |
| `00 7F 00` | 0x20 | sensor offsets TE0..TE9, one signed byte each, °C*10; write 1 byte to 0x7F+n, reply echoes | high, TE3 seen from panel |
| `00 89 02` | write 1 byte | "LED mode" (0 and 3 seen), first byte of the heating settings frame, reply echoes | high, seen from panel |
| `00 0E 00` | 2 / write | flags word: bits 0x0003 = hot water priority (1 low, 2 normal, 3 high, as in 1.6), bit 0x0200 = fan boost. The panel rewrites the whole word to change priority | high, seen from panel |
| `01 40 03` | 0xBA | additional (electric heater) power confirmed: 6000 W while the heater ran, total = compressor + additional to the watt | high |
| `01 D9 03` | write | panel → compressor frequency request, Hz*10 | high |
| `01 E1 03` | write | panel → temperature setpoint, °C*10 | medium (which setpoint is unclear) |
| `00 8D 04` | 0xAF | regulator settings; cooling group at the end: max frequency 50.0 Hz, P-band 1 10.0, I-time 1 180 min, P-band 2 200.0, I-time 2 40 s (bank 0 0x051C..0x0525) | high, confirmed on the panel |
| `01 50 05` | 0x32 | panel identity: UI software 2.28.1, hardware F, heartbeat counter | high, confirmed on the panel |

### Status flags in the 0x88 frame

Payload byte offsets (add 21 for the position in the raw frame):

| Byte | Bit | Meaning | Confidence |
|---|---|---|---|
| 18 | 0x20 | extra hot water active | high |
| 19 | 0x03 | extra hot water active | high |
| 19 | 0x20 | fan boost active | high |
| 20 | 0x20 | compressor running (matches power > 0) | high |
| 20 | 0x04 | set once hot water was requested, never cleared in the capture | low |
| 20 | 0x08 | cleared during fan boost | low |
| 25 | 0xC0 | extra hot water active | high |
| 26 | 0x10 | fan boost active | high |
| 27 | 0x03 | cooling mode on | high |
| 16 | 0x0C | fireplace mode on | high |
| 31..32 | word | pending alarm bitmask, bit 0x0002 = filter replacement (1.8 scheme, 1.8 had it 2 bytes earlier) | high |
| 39..40 | word | acknowledged alarm bitmask = bank 1 0x002B, written through 81 2B 00 | high |
| 13 / 14 / 24 | 0x05 / 0x10 / 0x40 | alarm indication, set while any alarm is active | high |
| 11 | value | countdown 90 → 0 in 10 s steps, started when an alarm is raised | observed |

Bytes 19 (0x20) and 26 (0x10) are also set during fireplace mode, so they mean "fan
override" rather than fan boost specifically.
| 28 | 0x08 | set once hot water was requested | low |

### Sensors in the 0x88 frame

TE0..TE7 and TE24 keep the 2.21 meaning. Unused sensors read -99.0. Of the others, these move:

| Sensor | Range seen | Note |
|---|---|---|
| TE25 | 3.6 to 11.2 | noisy every sample, probably not a temperature |
| TE26 | 15.8 to 28.9 | rises with the compressor, copied to 0x51 frame offset 17 |
| TE27 | 10.4 to 16.0 | copied to 0x51 frame offset 19 |
| TE28 | 0 to 14.2 | non zero only while the compressor runs |
| TE30 | 18.5, once 99.3 | |

## Leads (not in the library yet)

Values that clearly mean something but need another capture or a look at the panel's
advanced menu to name. Offsets are payload offsets.

**0x51 frame (`01 D1 02`)**
- byte 3 (fan speed, as on 2.21): on the captured unit the ventilation is driven by
  CO2/humidity sensors, so it stepped 1, 2, 3, 1 by itself around the hot water run and
  read 0 while the fan boost was active. Units with a manual fan speed setting should see
  the usual 1..3 here and should show a `KR_FAN_SPEED` style write when changed on the panel.
- off 10 and 12: 0 → about 130 and 28 → about 310 from the moment hot water was requested,
  stay up after the compressor stops. Circulation pump or fan duty?
- off 15: 224 → 464 → decays. Temperature-like, tracks hot gas but higher.
- off 23: equals compressor frequency while running, decays slowly after stop.
- off 31: max(off 23, 150). Minimum frequency clamp?
- off 35, 37, 39, 41: -1050 (invalid marker) when idle, negative values while running.
  Superheat or pressure-derived values?
- off 49: requested frequency (equals the panel's write one cycle later). 250 at start.
- off 55: same, one more cycle later.

**0xBA frame (`01 40 03`)**
- off 16: after compressor stop counts down from 298 in seconds. Restart delay (5 min).
- off 20: from compressor start counts down from 899 in seconds (15 min).
- off 26: 60 → 47 → 60 and off 59: 45 → 0 → 45: start-up countdowns.
- off 24: 15 before the electric heater started, 3855 (0x0F0F) while it ran, 0 after it
  stopped. Heater stage or request/active mask?
- off 82 (0 → 273), off 84 (0 → -382), off 88 (0 → 154): compressor-run-only values.
- off 93: 30 → 0 countdown at start.
- off 98 (4103 → 4406) and off 100 (750 → 902): rise with the compressor and freeze at
  the last value after stop. DC bus voltage and a pressure are candidates.
- off 109, 115, 122: sawtooth counters, ignore.
- off 161: equals the panel's temperature setpoint write.

**Hot water block tail (bank 0, 0x03C2..0x03F6, after `hot_water_extra_time`)**
Confirmed on the panel: 0x03C2 legionella setpoint 65.0, 0x03C4 legionella temperature 1
(60), 0x03C5 legionella temperature 2 (65), 0x03C6 legionella time 1 (60 min), 0x03C7 hot
water overheating alarm 73.0. Then 10, 50, 30 (bytes), 500, and 50 at 0x03CE which is
presumably "minimal frequency time" 5.0 min. Fan settings confirmed on the panel: 0x03DD
normal 50.0 %, 0x03DF slow -53.0 %, 0x03E1 fast 133.0 % (same order as 1.6), 0x03EB
increase when cooling +65.0 %, 0x03F5 CCM setting -5.0 %; "CCM increase when cooling +0 %"
is presumably the 0 at 0x03E3. Still unnamed: 550, 360, 400, 730, 600, 600, 2 (0x03D0..0x03DC),
15, 100, 150, 1000 (0x03E5..0x03EA), 750, 1000, 950, 1000 (0x03ED..0x03F4). "Legionella alert: yes" and "legionella day: Monday" are not
visible as plain values; they are probably bits or a 0-based day in one of the bytes.

**Compressor block (bank 0, 0x03F7..0x0437, the 0x57 frame)**
700, 0, 50, 18 (filter change countdown in days at 0x03FD), 300, 266, 20, 5, 2600, 0, 3200,
30, 50, 400, 300, 40, 20, 5, 5, 10, 1350, 3400, 5000, 150, then confirmed by writes:
compressor max frequency 105.0 (0x041D) and night max frequency 105.0 (0x041F); then 250,
20, 550, 700, 1050, 1020, 1050 (unnamed), then blocked frequencies 1..5 = 404, 855, 878, 833,
0 (0x042E..0x0437, confirmed by a write to number 5). Night start/end (22:00/07:00) and the
legionella day (Monday) do not appear in the board's memory in any encoding: the panel
keeps schedule settings locally.

**Settings confirmed by change-and-revert writes (2026-09-13)**

| Setting | Bank/addr | Encoding | Write reply |
|---|---|---|---|
| Compressor max frequency | 0 / 0x041D | u16 Hz*10 | 0x04 |
| Night max frequency | 0 / 0x041F | u16 Hz*10 | 0x04 |
| Heating curve | 0 / 0x0291 | u8 *10 (5.4 => 54) | echo |
| Fan normal speed | 0 / 0x03DD | u16 %*10 | 0x01 |
| Legionella temperature 1 | 0 / 0x03C4 | u8 °C | echo |
| Blocked frequency 5 | 0 / 0x0436 | u16 Hz*10 | 0x00 |
| Filter interval | 0 / 0x0576 | u16 days | 0x00 |
| Filter days left | 0 / 0x03FD | u16 days (writing 0 raises the alarm, writing the interval resets it) | 0x00 |
| Acknowledge alarm | 0x81 / 0x002B | set-bit write of the alarm mask (0x0002 filter); 1.6 used 0x0029 | 0x00 |

The heating curve byte sits right before `heating_calculated_setting` in the 0xC3 frame:
this panel has no room setpoint, the 18.0 there is computed from the curve. Write replies
vary per register, so setters for these need `header_check_only`.

**Small frames**
- `01 47 01` (22 bytes): off 4 and 6: 0 → 289 → 32 with hot water; off 10 noisy 37..112;
  off 12: 332 → 537 → 350; off 14: 885 → 725 → 973; off 16 tracks TE28; off 20 ramps
  190 → 1016 → 190 around the compressor peak.
- `01 A7 01` (8 bytes): off 0: 333 idle, 500 running, 993 peak, 0 during fan boost;
  off 4: 1000 → 450 once hot water requested; off 6: 60 → about 175 running.
- `01 F9 01` (118 bytes): off 101: counts down from 1799 s when extra hot water was
  switched on (30 min extra hot water runtime). off 116: 1585 idle → 3753 with the
  compressor → 0 during fan boost → 1321. Behaves like a fan or pump RPM.
- `01 50 05` (28 bytes): the panel's identity block, written by the panel itself: UI software
  version 2.28.1 and hardware version F (1.8 "UI versions" layout, decoded), then the
  heartbeat counter at off 24 (the panel's 10-byte write goes to 0x0568).
- Regulator version 2.50 appears as the byte pair 2, 50 at bank 0 0x054C, followed by 60,
  1298, -150, 0, 3 (WP box version 3?), 8, 7. Confirmed on the panel: UI 2.28.1 / F,
  regulator 2.50, WP box 3.
- `03 0B 0D`, `03 BD 0D`, `03 6F 0E`: the alarm log, decoded in DEBUG mode. A ring of 30
  records of 14 bytes from bank 3 0x0D0C with the next write index at 0x0D0B. Record: day,
  month, year, hour, minute, second, event, type, 6 zero bytes. Event 1 = raised, 2 =
  acknowledged (the 81 2B 00 write), 3 = reset (for the filter: the countdown written back); type 0x12 = filter replacement (confirmed by forcing the alarm),
  0x3D = communication error (the alarm the panel raised while the adapter jammed the bus).
  Event 6 with type 0 appears weekly at 05:00 on the legionella day. Event 0 with type 0
  appears at what look like restarts. `03 FC 0E` right after the log is probably per-type
  counters.
- `00 00 00`, `00 7F 00`, `00 27 01`, `00 61 06`, `00 8D 04`, `00 50 05`, `01 20 04`,
  `03 FC 0E`: constant during both captures, so settings or identification.

## Bus timing and transmitting

Measured with `test_tx/`: 345 frames in 20 s, a frame every ~56 ms, no idle period between
polling cycles at all. After a reply the panel's next query follows within 40 ms (mostly
within 20). The heat pump's reply to a query completes 45 to 165 ms after the query
(mostly transmission time of the long replies). So the only slot for a second master is the
few ms right after a reply completes. The library's 2.50 `push_settings()` sends there and
tolerates the panel's next query arriving before the pump's reply to us. Both test commands
succeeded on the first attempt with no collision.

## Not found yet

- 2.21-style fan speed byte write (`KR_FAN_SPEED`). The captured panel has no fan speed
  setting because CO2/humidity sensors control the ventilation; the only user action
  produced the fan boost bit toggle. A 2.50 unit with manual fan speed still needs a capture.
- Room temperature write. Changing the room temperature on the panel produced no write at
  all, so `set_room_temperature()` uses a derived register (`00 92 02`) that is unverified.

## Method

1. Flash a sketch with `DEBUG` on and record the serial port with `tools/capture.py`
   (it clears DTR/RTS before opening, otherwise the board resets and jams the bus).
2. `tools/inventory.py capture.txt` lists every register with length and a sample.
3. `tools/timeline.py capture.txt` prints the known values per polling cycle with panel
   writes marked as events. Do something on the panel during the capture so that values move.
4. `tools/correlate.py capture.txt '01 D1 02'` lists which 16-bit fields of a frame change and
   which known value they copy or track. Check word alignment: some frames have a 1-byte
   field early on, so the meaningful words sit at odd offsets.
5. Replay the capture through the real decoder on the host (see CLAUDE.md) before trusting a
   new field.
