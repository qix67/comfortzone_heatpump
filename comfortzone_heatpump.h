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

	COMFORTZONE_STATUS comfortzone_status;

	private:
	HardwareSerial &rs485;
	int rs485_de_pin;

	FastCRC8 CRC8;

	// incoming buffer
	byte cz_buf[256];							// incoming RS485 bytes
	uint16_t cz_size = 0;					// #bytes in cz_buf
	uint16_t cz_full_frame_size = -1;	// #bytes in the current frame

	// for debug purpose (see set_grab_buffer() )
	byte *grab_buffer = NULL;
	uint16_t grab_buffer_size = 0;
	uint16_t *grab_buffer_frame_size = NULL;

	PROCESSED_FRAME_TYPE comfortzone_process_frame(struct cz_packet_header *czph);

	void dump_frame(const char *prefix);
};

// list of craftable command packets
typedef enum known_register_craft_name
{
	KR_UNCRAFTABLE = 0,		// uncraftable packet
	KR_FAN_SPEED,				// set fan speed, parameter => 1=slow, 2=normal, 3=fast
	KR_LED_LUMINOSITY,		// set led luminosity, parameter => 0=off to 6=full power
	KR_ROOM_HEATING_TEMP,	// set room heating temperature, parameter => 120 (=12.0°) to 240 (=24.0°)
	KR_HOT_WATER_TEMP,		// set hot water temperature, parameter => 120 (=12.0°) to 800 (=80.0°)
	KR_EXTRA_HOT_WATER_ON,	// enable extra hot water, no parameter
	KR_EXTRA_HOT_WATER_OFF,	// disable extra hot water, no parameter
} KNOWN_REGISTER_CRAFT_NAME;


// craft one command frame
// input: pointer to output buffer, min size is sizeof(W_CMD) = 30 bytes
//        name of the command to craft
//        parameter of the command (depend on crafted command, see KNOWN_REGISTER_CRAFT_NAME enum)
// output: 0 = uncraftable packet or crafting error else number of bytes used in buffer
uint16_t comfortzone_craft(byte *output_buffer, KNOWN_REGISTER_CRAFT_NAME reg_cname, uint16_t parameter);

#endif
