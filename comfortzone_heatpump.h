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

// process 1 byte from input stream (RS485)
// output: true = full frame received and processed
bool comfortzone_receive(byte input_byte);

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
