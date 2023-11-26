comfortzone heatpump library
by Eric PREVOTEAU

1) Quick start
==============

- Place this folder in your libraries directory

- Install FrankBoesing's FastCRC library (https://github.com/FrankBoesing/FastCRC)
  in your libraries directory. (Note: this step is not necessary if you use
  teensyduino).

- Start or restart the Arduino IDE

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

Connect GND, A and B pins to RS485 module.

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

By default, the library only performs bus snooping assuming a control panel
periodically requests heatpump status (roughly every 5 seconds on mine).

The library can send commands on RS485 bus.

Example heatpump_dump.ino is a basic example showing how to use the library.
During the first 5 seconds, output variable "comfortzone_status" is not 
initialized or partially initialized, it is normal.

5) library API
==============

The library exposes few methods to make it easily usable. All methods are inside
comfortzone_heatpump object.

 * constructor: comfortzone_heatpump(RS485Interface* rs485);

 The constructor takes a pointer to a RS485Interface object. An Arduino-specific
 implementation (ArduinoRS485Interface) is provided in rs485_interface.h.
 It is initalized with HardwareSerial object and number of the pin connected
 to RS485 module DE pin.

 * void begin();

 First function to call before anything else.

 * PROCESSED_FRAME_TYPE process();

 This function must be called regularly to process bytes received by serial port

 returned value notifies the type of received frame
	- PFT_NONE: no frame received
	- PFT_CORRUPTED: a corrupted frame was received
   - PFT_QUERY: a query frame was received
   - PFT_REPLY: a reply frame was received
   - PFT_UNKNOWN: a frame of unknown type was encountered

 * void set_grab_buffer(byte *buffer, uint16_t buffer_size, uint16_t *frame_size);

 For debug purpose, when process() return value is not PFT_NONE, it can be useful
 to obtain the received frame. The function can be called at any time.

 If buffer is NULL, all other parameters are unused and frame grabber is disabled.

 If buffer is not NULL, buffer_size is the size in byte of the buffer. *frame_size
 will be updated by process() to report the number of byte used in buffer
 (=frame_size).

 * void enable_debug_mode(bool debug_flag);

 enable (true) or disable (false) debug mode. 

 When debug mode is enabled, set_* methods include additionnal debug data into 
 last_message[] buffer.

 The function can be called at any time.

 Heatpump control
 ----------------

 All set_* methods return true on success and false on error.

 On error, last_message[] contains a string describing the problem.

 When debug mode is enabled. last_message[] also contains debug information on
 both error and success.

 The default timeout is the maximum number of seconds before giving up.

 * bool set_fan_speed(uint8_t fan_speed, int timeout = 5);

 update heatpump fan speed. Possible speeds are:
	- 1 = low
	- 2 = normal
	- 3 = fast


 * bool set_room_temperature(float room_temp, int timeout = 5);

 update heatpump room heating setting. Possible temperature goes from
 10.0° to 50.0°, with step of 0.1°.

 Temperature is in °C.


 * bool set_hot_water_temperature(float room_temp, int timeout = 5);

 update heatpump hot water setting. Possible temperature goes from
 10.0° to 60.0°, with step of 0.1°)

 Temperature is in °C.


 * bool set_led_luminosity(uint8_t led_level, int timeout = 5);

 update heatpump led luminosity. Values goes from 0 (off) to 6 (highest level).


 * bool set_hour(uint8_t hour, int timeout = 5);

 Update heatpump internal hour. Range: [0:23]


 * bool set_minute(uint8_t minute, int timeout = 5);

 Update heatpump internal minute. Range: [0:59]


 * bool set_day(uint8_t day, int timeout = 5);

 Update heatpump internal day of month. Range: [1:31]


 * bool set_month(uint8_t month, int timeout = 5);

 Update heatpump internal month. Range: [1:12]


 * bool set_year(uint16_t year, int timeout = 5);

 Update heatpump internal year. Range: [2000:2255]


 * bool set_extra_hot_water(bool enable, int timeout = 5);

 Update heatpump extra hot water. True = enable extra hot water. False = disable


 * bool set_automatic_daylight_saving(bool enable, int timeout = 5);

 Update heatpump automatic daylight saving. True = automatic. False = manual


 * bool set_sensor_offset(int16_t sensor_num, float temp_offset, int timeout = 5);

 Set temperature sensor offset.
 
 Possible sensor number goes from 0 to 7.
 Possible temperature offset goes from -10.0° to 10.0°, with step of 0.1°


 * bool set_fireplace_mode(bool enable, int timeout = 5);

 Enable/disable temporary mode to start the fireplace
 (disables fan for 5 minutes, runs completely on electricity during that period)


 Special methods
 ---------------

 * void set_heatpump_addr(byte new_heatpump_addr[4]);

 The default heatpump address (0x65, 0x6F, 0xDE, 0x02) may not be the correct
 one for everybody. Using this function, it is possible to change it.


 * bool guess_heatpump_addr(byte guessed_addr[4], int timeout = 5);

 The method will try to guess heatpump address using bus snooping and packets
 sent by command panel. It will try during at most "timeout" seconds. 

 At end, true means a successful guess (guessed_addr is populated) and false
 on no luck.

 Unless set_heatpump_addr() is called with the guessed address, the library will
 not automatically used the guessed address.


6) Library variables
====================

 * char last_message[];

 Contains debug messages and set_* methods return status


 * COMFORTZONE_STATUS comfortzone_status;

 Last status heatpump. Updated automatically when a STATUS frame is received.

 See comfortzone_status.h for more information.


7) Note
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

