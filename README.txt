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

Connect GND, A and B pins to RS485 module. Serial port must be configured in
8N1 19200.

Warning: if A and B pins are inverted on RS485, heatpump control panel will
display a "connection lost" message. I don't know if it may break something but
I accidentaly misconnect my module during ~2 minutes and everything still works
fine.

3) Library configuration
========================

It is possible to configure the library using comfortzone_config.h file.

Defining DEBUG macro will enable the very verbose mode. All debug messages
will be printed on the serial console set in OUTSER define.

4) Using library
================

The library only decodes data sent using comfortzone_receive() function, it
does not retrieve data itself.

The library does not send commands (at least currently) on RS485 bus. Data
have to be obtain using bus snooping. Heatpump control panel already sends
periodic status query (every 5 seconds for each status packet).



