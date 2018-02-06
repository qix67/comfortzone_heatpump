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

(Note: you can check GND location by powering off the heatpump and do a
continuity test between pin and led strip GND pad).

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

The library only decodes data sent to comfortzone_receive() function, it does
not retrieve data itself.

The library does not send commands (at least currently) on RS485 bus. Data
have to be obtained using bus snooping. Heatpump control panel already sends
periodic status query (every 5 seconds for each status packet).

Example heatpump_dump.ino is a basic example showing how to use the library.
During the first 5 seconds, output variable "comfortzone_status" is not 
initialized or partially initialized, it is normal.

5) Note
=======

This library is a work in progress. 

Output variable "comfortzone_status" (see comfortzone_status.h) returns
basic status which is avaible on the digital control panel in standard mode.

Few additional status are available when digital control panel is switched to
advanced mode.

Heatpump controller sends a huge amount of data and most of them are not 
displayed by digital control panel, even in advanced mode. To see all
outputs, enable library DEBUG mode. You can also take a look at
comfortzone_frame.h file.

A bunch of status variables are available. Variable names should be the same as
the name shown on digital control panel in advanced mode.

It still remains a great quantity of unknown data (unknown? variables). Some
of them should be related to option not enabled on my machine (I have no chimney
for example), other are never changing data which make it harder to guess their
meaning. Finally some of them should be related to error or warning.

Currently crc is not checked mainly because I was unable to find used CRC
algorithm. The missing crc algorithm also prevents command crafting even if I
have a way to bypass this problem.

