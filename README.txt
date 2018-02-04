comfortzone heatpump library
by Eric PREVOTEAU

1) Quick start
==============

Place this folder in your libraries directory
Start or restart the Arduino IDE

2) Wiring
=========

Comfortzone EX50 heatpump control board comes with 2 RS485 ports connected to
RJ11 6P6C. 1 port is connected to the onboard control panel, the other port is
free to use.

RJ11 wiring is
- pin 1: 24V (25V without load)
- pin 2: GND
- pin 3: RS485 B
- pin 4: RS485 A
- pin 5: 24V (25V without load)
- pin 6: GND

Connect GND, A and B pin to RS485 module. Serial port must be configure in
8N1 19200.


