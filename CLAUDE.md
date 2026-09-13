# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

An Arduino/ESP32 library (C++, no build system of its own) that snoops and controls a
Comfortzone EX50 heat pump over its RS485 bus. Upstream is
`https://github.com/qix67/comfortzone_heatpump` (author Eric Prevoteau, GPLv3). The root
folder *is* the library: sources sit flat at the top level and `library.properties` /
`library.json` describe it for the Arduino IDE and PlatformIO.

## Building

There is no Makefile, CMake, PlatformIO project, or test suite. The library is compiled as
part of whichever sketch includes it:

- `~/Arduino/libraries/comfortzone_heatpump` is a symlink to this repo, so the Arduino IDE
  picks up edits directly. Neither `arduino-cli` nor `pio` is on PATH; build and flash via the
  Arduino IDE.
- Sketches: `examples/heatpump_dump/` (prints decoded status every 5 s),
  `examples/heatpump_frame_dump/` (only calls `process()`; relies on `DEBUG` to print frames),
  plus untracked scratch sketches `heatpump_dump.ino` and `test.ino/` at the root.
- Hard dependency: FrankBoesing's FastCRC (`FastCRC.h`), already in `~/Arduino/libraries`.
  Only `CRC8.maxim()` is used.

## Configuration is compile-time

`comfortzone_config.h` is the single knob file and is committed with *local* settings, so
check `git diff comfortzone_config.h` before assuming defaults:

- `HP_PROTOCOL` selects one of `HP_PROTOCOL_1_6` / `_1_8` / `_2_21` / `_2_50` (values
  160/180/221/250). This is a whole-library `#if` switch, not a runtime option: it changes
  the register table, the status frame layouts, which fields of `COMFORTZONE_STATUS` get
  populated, the enum members available, and setter value ranges. Upstream default is 1.6;
  this checkout has 2.50 forced on (the owner's EX50 has control panel firmware 2.50).
- `DEBUG` + `OUTSER` turn on very verbose frame dumping via `DPRINT`/`NPRINT` macros. Most
  of `comfortzone_decoder_basic.cpp` and the `dump_unknown` calls compile to nothing
  without `DEBUG`.

## Architecture

Three classes, all in the root directory:

- **`comfortzone_heatpump`** (`comfortzone_heatpump.{h,cpp}`) is the public API. `process()`
  must be polled continuously; it pulls bytes from the `RS485Interface`, hunts for a frame
  start, validates it, and dispatches to the decoder. `set_*()` methods craft a write command,
  craft the *expected* reply, and hand both to the private `push_settings()`, which waits for
  a quiet bus, transmits, and matches the reply. On success they also write straight into
  `comfortzone_status` so callers see the new value before the next status broadcast.
- **`czdec`** (`comfortzone_decoder*.cpp`, static methods) owns the register table
  `kr_decoder[]` in `comfortzone_decoder.cpp`. Each row is a 9-byte register number, a
  `czcraft::KR_*` craft name (or `KR_UNCRAFTABLE`), a display name, and four function pointers
  for the `R`/`W`/`r`/`w` cases. `process_frame()` does a linear `memcmp` over that table.
  The table is wrapped in `#if HP_PROTOCOL == ...` blocks, so one register (e.g. fan speed)
  has a different byte pattern per protocol. Big status frames are decoded in
  `comfortzone_decoder_status.cpp` (`reply_r_status_NN` for 1.6, `_v180_*` for 1.8,
  `_v221_*` for 2.21); small scalar registers use the generic helpers in
  `comfortzone_decoder_basic.cpp`.
- **`czcraft`** (`comfortzone_crafting.{h,cpp}`) builds outgoing `W_CMD` / `W_SMALL_CMD` /
  `W_REPLY` packets. Setters look up their register via
  `czdec::kr_craft_name_to_index(KR_...)`, which is why a setter fails with "not found" when
  the current protocol's table has no row for that `KR_*` name.

Supporting headers:

- `comfortzone_frame.h`: packed structs for every wire format. Frame = 4-byte destination,
  two CRC8-maxim bytes over destination and its complement, 4-byte source, total size, cmd
  char (`R`/`W`/`r`/`w`), 9-byte register number, payload, trailing CRC8 over everything
  before it. Multi-byte values are little-endian; temperatures are `int16 * 10`.
- `comfortzone_status.h`: `COMFORTZONE_STATUS`, the output struct. Every field is a
  `Subscribable<T>` whose `operator=` fires an optional callback, so assigning to a status
  field has a side effect. Field comments state which protocols actually populate it.
- `rs485_interface.h`: abstract `RS485Interface` plus `ArduinoRS485Interface` (19200 8N1,
  DE pin toggled for half duplex). Non-Arduino targets supply their own implementation.
- `platform_specific.h`: `millis()` shims for ESP-IDF and Windows so the core can compile
  off-Arduino.

## Things that bite

- Frame sync has no start marker. `process()` slides a window until the header's two CRC
  bytes and cmd char check out, so garbage on the bus costs at most one byte per call.
- `push_settings()` sets `disable_cz_buf_clear_on_completion` so it can inspect the reply
  itself. If a code path returns without clearing that flag, every later frame is corrupted
  until the buffer overflows.
- Heat pump address defaults to `65 6F DE 02` and may differ per unit; `guess_heatpump_addr()`
  snoops for it but does not apply it, `set_heatpump_addr()` does.
- Adding a new decoded value means: add the field to `COMFORTZONE_STATUS`, locate its
  offset in the relevant packed status struct in `comfortzone_frame.h`, and assign it in the
  matching `reply_r_status_*` function. Do this per protocol; the layouts are unrelated.
- The 2.21 and 2.50 tables in `comfortzone_decoder.cpp` carry raw captured frames as
  comments above each row; those are the only "spec" for those protocols.
- In 2.21 and 2.50 the register number is a memory address: bytes 6..8 of `reg_num` are
  `{bank, addr_lo, addr_hi}` and the read command's parameter is the byte count. 2.50 is the
  2.21 map with a few bytes inserted, so the same frame appears at an address shifted by 0
  to 5 and one or two bytes longer. Map a new register by matching payload content against
  the 2.21 sample comments, not by address arithmetic alone.
- 2.50 status: heating setting, hot water setting, time/date, sensors, compressor frequency
  and running flag, extra hot water flag, powers, energies, runtimes, and filter days are
  decoded. `PROTOCOL_2_50.md` holds the register map,
  the identified flag bits, and the open leads; `tools/` holds the capture and analysis
  scripts that produced them. Writes confirmed from
  the panel: hot water setting, extra hot water on/off (bit 0x0040, values differ from the
  1.6 ones hardcoded in the setter, hence the `#if` there), a fan boost toggle (bit
  0x0200 at address 0x0E, exposed as `set_fan_boost()`), and cooling mode (bit 0x0400 at
  bank 0 address 0x12, exposed as `set_cooling_mode()`). Never seen on the bus: the 2.21
  style fan speed byte write and fireplace mode. Changing the room temperature on the panel
  produced no write, so `00 92 02` (room heating temperature) stays derived, not observed.
- Bit-toggle registers use bank byte 0x81/0x80 to set and 0x41/0x40 to clear bits of a
  16-bit word; the value is the mask (set) or its complement (clear). The heat pump's reply
  value is not always 0, so setters for these pass `header_check_only = true`.

## Host-side testing

No board is needed to check a decoder change. Stub `FastCRC.h` (CRC-8/MAXIM, poly 0x31
reflected) and a `Serial` object, provide `millis()`, then `g++ -fsyntax-only` every `.cpp`
with `-I.`. Feeding raw frame bytes into `comfortzone_heatpump::process()` through a fake
`RS485Interface` replays a capture through the real decoder; the `UNK:` lines that the
`DEBUG` frame dump prints are exactly those bytes.
