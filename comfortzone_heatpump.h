/*
   This file is part of comfortzone_heatpump library.

   comfortzone_heatpump library is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   comfortzone_heatpump library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with comfortzone_heatpump library.  If not, see <http://www.gnu.org/licenses/>.

   Copyright (C) 2017 Eric PREVOTEAU

   Original Author: Eric PREVOTEAU <digital.or@gmail.com>
*/

#ifndef _COMFORTZONE_HEATPUMP_H
#define _COMFORTZONE_HEATPUMP_H

#include <Arduino.h>

#include <HardwareSerial.h>
#include <FastCRC.h>

#include <comfortzone_status.h>

class comfortzone_heatpump
{
	public:
	typedef enum processed_frame_type
	{
		PFT_NONE,		// no full frame received and process
		PFT_CORRUPTED,	// received frame was corrupted (CRC error)
		PFT_QUERY,		// received frame was a command
		PFT_REPLY,		// received frame was a reply
		PFT_UNKNOWN,	// received frame has an unknown type
	} PROCESSED_FRAME_TYPE;

	comfortzone_heatpump() : rs485(Serial1) {};

	void begin(HardwareSerial &rs485_serial, int de_pin);

	// Function to call periodically to manage rs485 serial input
	PROCESSED_FRAME_TYPE process();
	
	// for debug purpose, it can be useful to get full frame
	// input: pointer on buffer where last full frame will be copied
	//        max size of buffer
	//        size of the last full frame received
	// If buffer is set to NULL, frame grabber is disabled
	// If buffer is not NULL, each time comfortzone_receive() reply is not PFT_NONE, the received frame will
	// be copied into buffer and *frame_size will be updated
	// recommended buffer_size is 256 bytes
	void set_grab_buffer(byte *buffer, uint16_t buffer_size, uint16_t *frame_size);

	// Functions to modify heatpump settings
	// timeout (in second) is the maximum duration before giving up (RS485 bus always busy)
	// output: true = ok, false = failed to process
	bool set_fan_speed(uint8_t fan_speed, int timeout = 5);					// 1 = low, 2 = normal, 3 = fast
	bool set_room_temperature(float room_temp, int timeout = 5);			// temperature in °C
	bool set_hot_water_temperature(float room_temp, int timeout = 5);	// temperature in °C
	const char *set_led_luminosity(uint8_t led_level, int timeout = 5);			// 0 = off -> 6 = highest level

	// current status
	COMFORTZONE_STATUS comfortzone_status;

	private:
	friend class czdec;
	friend class czcraft;

	HardwareSerial &rs485;
	int rs485_de_pin;

	FastCRC8 CRC8;

	// incoming buffer
	byte cz_buf[256];							// incoming RS485 bytes
	uint16_t cz_size = 0;					// #bytes in cz_buf
	uint16_t cz_full_frame_size = -1;	// #bytes in the current frame

	// frame timestamp
	unsigned long last_frame_timestamp = 0;
	unsigned long last_reply_frame_timestamp = 0;

	// for debug purpose (see set_grab_buffer() )
	byte *grab_buffer = NULL;
	uint16_t grab_buffer_size = 0;
	uint16_t *grab_buffer_frame_size = NULL;

	// when a packet is processed, input buffer is automatically cleared at end
	// however this action must be disabled when sending a command because we must catch the reply internally
	// WARNING: if this flag is set to true, due to the buffer not being cleared on frame completion
	// all following frames until cz_buf overflow will be corrupted
	bool disable_cz_buf_clear_on_completion = false;

	// RS485 "address of heatpump"
	byte heatpump_addr[4] = { 0x65, 0x6F, 0xDE, 0x02 };
	
	// RS485 "address of this controller"
	byte controller_addr[4] = { 0x45, 0x72, 0x69, 0x63 };		// can be anything but must be uniq on this RS485 bus

	// send a command to the heatpump and wait for the given reply
	// on error, several retries may occur and the command may take up to "timeout" seconds
	const char *push_settings(byte *cmd, int cmd_length, byte *expected_reply, int expected_reply_length, int timeout);
};

#endif
