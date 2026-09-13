#!/usr/bin/env python3
"""Record the sketch's serial output (DEBUG mode) to a file without resetting the board.

usage: capture.py PORT SECONDS OUTFILE      e.g. capture.py /dev/ttyUSB0 300 capture.txt

DTR and RTS are both cleared BEFORE the port is opened (unopened Serial object, then open()).
Measured on a CP2102 ESP32 dev board: opening with pyserial's defaults (both asserted) holds the
board in reset and the port streams 0x00/0x80 line noise; clearing one line after opening
resets the board once. Clearing both first gives a clean start. While the board resets its RS485
driver is enabled, which the control panel reports as a communication error.
"""
import sys, time, serial

port, seconds, out = sys.argv[1], float(sys.argv[2]), sys.argv[3]
s = serial.Serial()
s.port = port; s.baudrate = 115200; s.timeout = 1
s.dtr = False; s.rts = False
s.open()
end = time.time() + seconds
with open(out, 'ab') as f:
    while time.time() < end:
        d = s.read(4096)
        if d:
            f.write(d); f.flush()
s.close()
