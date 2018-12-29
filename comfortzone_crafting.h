#ifndef _COMFORTZONE_CRAFTING_H
#define _COMFORTZONE_CRAFTING_H

#include "comfortzone_heatpump.h"
#include "comfortzone_frame.h"

class czcraft
{
	public:
	// list of craftable command packets
	typedef enum known_register_craft_name
	{
		KR_UNCRAFTABLE = 0,		// uncraftable packet
		KR_FAN_SPEED,				// set fan speed, parameter => 1=slow, 2=normal, 3=fast
		KR_LED_LUMINOSITY,		// set led luminosity, parameter => 0=off to 6=full power
		KR_ROOM_HEATING_TEMP,	// set room heating temperature, parameter => 120 (=12.0°) to 240 (=24.0°)
		KR_HOT_WATER_TEMP,		// set hot water temperature, parameter => 120 (=12.0°) to 800 (=80.0°)
		KR_EXTRA_HOT_WATER_ON,  // enable extra hot water, no parameter
		KR_EXTRA_HOT_WATER_OFF, // disable extra hot water, no parameter
	} KNOWN_REGISTER_CRAFT_NAME;

	// craft a W_CMD paquet
	// input: pointer to output buffer
	//		  9 byte array contaning register number
	//		 16bit value (it will be automatically stored into little endian)
	static int craft_w_cmd(comfortzone_heatpump *czhp, byte *output_buffer, byte *reg_num, uint16_t value);

	// craft a W_SMALL_CMD paquet
	// input: pointer to output buffer
	//		  9 byte array contaning register number
	//		  8bit value
	static int craft_w_small_cmd(comfortzone_heatpump *czhp, byte *output_buffer, byte *reg_num, byte value);
};


#endif
