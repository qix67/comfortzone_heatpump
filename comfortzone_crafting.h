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
		KR_EXTRA_HOT_WATER_ON,  // enable extra hot water, parameter => always 0xFFFE
		KR_EXTRA_HOT_WATER_OFF, // disable extra hot water, parameter => always 0x0001
		KR_HOUR,						// set hour, parameter 0-23
		KR_MINUTE,					// set minute, parameter 0-59
		KR_DAY,						// set day of month, parameter 1-31
		KR_MONTH,					// set day of month, parameter 1-12
		KR_YEAR,						// set day of month, parameter 2000-2255
		KR_AUTO_DAYLIGHT_SAVING_ON,  // enable automatic daylight saving, parameter => always 0xFFBF
		KR_AUTO_DAYLIGHT_SAVING_OFF, // disable automatic daylight saving, parameter => always 0x0040
		KR_HOT_WATER_PRIORITY,	// set hot water priority, parameter => 1=low, 2=normal, 3=high
		KR_TEMP_OFFSET_SENSOR0,	// set sensor 0 temperature offset, parameter => -100 (=-10°) to 100 (=10.0°)
		KR_TEMP_OFFSET_SENSOR1,	// set sensor 1 temperature offset, parameter => -100 (=-10°) to 100 (=10.0°)
		KR_TEMP_OFFSET_SENSOR2,	// set sensor 2 temperature offset, parameter => -100 (=-10°) to 100 (=10.0°)
		KR_TEMP_OFFSET_SENSOR3,	// set sensor 3 temperature offset, parameter => -100 (=-10°) to 100 (=10.0°)
		KR_TEMP_OFFSET_SENSOR4,	// set sensor 4 temperature offset, parameter => -100 (=-10°) to 100 (=10.0°)
		KR_TEMP_OFFSET_SENSOR5,	// set sensor 5 temperature offset, parameter => -100 (=-10°) to 100 (=10.0°)
		KR_TEMP_OFFSET_SENSOR6,	// set sensor 6 temperature offset, parameter => -100 (=-10°) to 100 (=10.0°)
		KR_TEMP_OFFSET_SENSOR7,	// set sensor 7 temperature offset, parameter => -100 (=-10°) to 100 (=10.0°)
	} KNOWN_REGISTER_CRAFT_NAME;

	// craft a W_CMD packet
	// input: pointer to output buffer
	//		  9 byte array contaning register number
	//		 16bit value (it will be automatically stored into little endian)
	static void craft_w_cmd(comfortzone_heatpump *czhp, W_CMD *output_buffer, byte *reg_num, uint16_t value);

	// craft a W_SMALL_CMD packet
	// input: pointer to output buffer
	//		  9 byte array contaning register number
	//		  8bit value
	static void craft_w_small_cmd(comfortzone_heatpump *czhp, W_SMALL_CMD *output_buffer, byte *reg_num, byte value);

	// craft a W_REPLY packet
	// input: pointer to output buffer
	//		  9 byte array contaning register number
	//		  expected 8bit value
	static void craft_w_reply(comfortzone_heatpump *czhp, W_REPLY *output_buffer, byte *reg_num, byte value);
};


#endif
