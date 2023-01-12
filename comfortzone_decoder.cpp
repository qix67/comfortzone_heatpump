#include "comfortzone_heatpump.h"
#include "comfortzone_config.h"

#include "comfortzone_frame.h"
#include "comfortzone_decoder.h"

static czdec::KNOWN_REGISTER kr_decoder[] =
	{
#if HP_PROTOCOL == HP_PROTOCOL_1_6
		// don't know why but extra hot water off does not use the same message as extra hot water on
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x41, 0x19, 0x00}, czcraft::KR_EXTRA_HOT_WATER_OFF, "Extra hot water - off", czdec::cmd_r_generic, czdec::cmd_w_extra_hot_water, czdec::reply_r_extra_hot_water, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x81, 0x19, 0x00}, czcraft::KR_EXTRA_HOT_WATER_ON, "Extra hot water - on", czdec::cmd_r_generic, czdec::cmd_w_extra_hot_water, czdec::reply_r_extra_hot_water, czdec::reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x81, 0x29, 0x00}, czcraft::KR_UNCRAFTABLE, "Clear alarm", czdec::cmd_r_generic, czdec::cmd_w_clr_alarm, czdec::reply_r_clr_alarm, czdec::reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x40, 0x00, 0x00}, czcraft::KR_AUTO_DAYLIGHT_SAVING_OFF, "Daylight saving - off", czdec::cmd_r_generic, czdec::cmd_w_daylight_saving, czdec::reply_r_daylight_saving, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x80, 0x00, 0x00}, czcraft::KR_AUTO_DAYLIGHT_SAVING_ON, "Daylight saving - on", czdec::cmd_r_generic, czdec::cmd_w_daylight_saving, czdec::reply_r_daylight_saving, czdec::reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x80, 0x0E, 0x00}, czcraft::KR_UNCRAFTABLE, "Sanitary priority (set)", czdec::empty, czdec::empty, czdec::empty, czdec::reply_w_generic},	// this frame itself is not enough to set priority

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x00, 0x00}, czcraft::KR_UNCRAFTABLE, "Status 09", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_09, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x00, 0x04}, czcraft::KR_FAN_SPEED, "Fan speed", czdec::cmd_r_generic, czdec::cmd_w_fan_speed, czdec::reply_r_fan_speed, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x05, 0x04}, czcraft::KR_UNCRAFTABLE, "Fan boost increase", czdec::cmd_r_generic, czdec::cmd_w_percentage, czdec::reply_r_percentage, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x0E, 0x00}, czcraft::KR_UNCRAFTABLE, "Sanitary priority (get)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_sanitary_priority, czdec::empty},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x16, 0x02}, czcraft::KR_UNCRAFTABLE, "Status 11", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_11, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x17, 0x04}, czcraft::KR_UNCRAFTABLE, "Supply fan T12 adjust", czdec::cmd_r_generic, czdec::cmd_w_percentage, czdec::reply_r_percentage, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x19, 0x00}, czcraft::KR_UNCRAFTABLE, "Status 24", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_24, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x2E, 0x03}, czcraft::KR_UNCRAFTABLE, "Minimal return temperature", czdec::cmd_r_generic, czdec::cmd_w_temp, czdec::reply_r_temp, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x29, 0x04}, czcraft::KR_UNCRAFTABLE, "Room Heating - compressor max frequency", czdec::cmd_r_generic, czdec::cmd_w_freq, czdec::reply_r_freq, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x2C, 0x04}, czcraft::KR_UNCRAFTABLE, "Status 12", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_12, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x35, 0x04}, czcraft::KR_UNCRAFTABLE, "Heatpump - compressor - blocked frequency 1", czdec::cmd_r_generic, czdec::cmd_w_freq, czdec::reply_r_freq, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x37, 0x04}, czcraft::KR_UNCRAFTABLE, "Heatpump - compressor - blocked frequency 2", czdec::cmd_r_generic, czdec::cmd_w_freq, czdec::reply_r_freq, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x39, 0x04}, czcraft::KR_UNCRAFTABLE, "Heatpump - compressor - blocked frequency 3", czdec::cmd_r_generic, czdec::cmd_w_freq, czdec::reply_r_freq, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x64, 0x01}, czcraft::KR_UNCRAFTABLE, "Status 25", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_25, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7A, 0x03}, czcraft::KR_UNCRAFTABLE, "Status 01", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_01, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7B, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR0, "Hardware Settings - Adjustments - TE0 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7C, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR1, "Hardware Settings - Adjustments - TE1 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7D, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR2, "Hardware Settings - Adjustments - TE2 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7D, 0x03}, czcraft::KR_HOT_WATER_TEMP, "Hot water calculated setting", czdec::cmd_r_generic, czdec::cmd_w_temp, czdec::reply_r_temp, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7E, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR3, "Hardware Settings - Adjustments - TE3 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7F, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR4, "Hardware Settings - Adjustments - TE4 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x80, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR5, "Hardware Settings - Adjustments - TE5 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x81, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR6, "Hardware Settings - Adjustments - TE6 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x82, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR7, "Hardware Settings - Adjustments - TE7 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xA6, 0x04}, czcraft::KR_UNCRAFTABLE, "Heatpump - defrost delay", czdec::cmd_r_generic, czdec::cmd_w_time, czdec::reply_r_time, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xA7, 0x02}, czcraft::KR_LED_LUMINOSITY, "LED luminosity", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xAC, 0x02}, czcraft::KR_UNCRAFTABLE, "Holiday reduction", czdec::cmd_r_generic, czdec::cmd_w_day_delay, czdec::reply_r_day_delay, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xC8, 0x02}, czcraft::KR_UNCRAFTABLE, "Status 10", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_10, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xB0, 0x02}, czcraft::KR_ROOM_HEATING_TEMP, "Heating calculated setting", czdec::cmd_r_generic, czdec::cmd_w_temp, czdec::reply_r_temp, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xB2, 0x00}, czcraft::KR_UNCRAFTABLE, "Status 23", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_23, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xDE, 0x04}, czcraft::KR_UNCRAFTABLE, "Status 13", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_13, czdec::reply_w_generic},	// 0x63 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x00, 0x00}, czcraft::KR_UNCRAFTABLE, "Status 02", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_02, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x04, 0x00}, czcraft::KR_HOUR, "Hour", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x05, 0x00}, czcraft::KR_MINUTE, "Minute", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x07, 0x00}, czcraft::KR_DAY, "Day of Month", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x08, 0x00}, czcraft::KR_MONTH, "Month (1=Jan)", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x09, 0x00}, czcraft::KR_YEAR, "Year (20xx)", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x16, 0x02}, czcraft::KR_UNCRAFTABLE, "Status 05", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_05, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x19, 0x00}, czcraft::KR_UNCRAFTABLE, "Status 22", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_22, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x2C, 0x04}, czcraft::KR_UNCRAFTABLE, "Status 14", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_14, czdec::reply_w_generic},	// 0x48 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x58, 0x04}, czcraft::KR_UNCRAFTABLE, "Language", czdec::cmd_r_generic, czdec::cmd_w_language, czdec::reply_r_language, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x64, 0x01}, czcraft::KR_UNCRAFTABLE, "Status 04", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_04, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x7A, 0x03}, czcraft::KR_UNCRAFTABLE, "Status 07", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_07, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0xB2, 0x00}, czcraft::KR_UNCRAFTABLE, "Status 03", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_03, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0xC8, 0x02}, czcraft::KR_UNCRAFTABLE, "Status 06", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_06, czdec::reply_w_generic},	// 0xC2 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0x15, 0x0D}, czcraft::KR_UNCRAFTABLE, "Status 15", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_15, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0x2B, 0x0F}, czcraft::KR_UNCRAFTABLE, "Status 18", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_18, czdec::reply_w_generic},	// 0x5D bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0x79, 0x0E}, czcraft::KR_UNCRAFTABLE, "Status 17", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_17, czdec::reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0xC7, 0x0D}, czcraft::KR_UNCRAFTABLE, "Status 16", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_16, czdec::reply_w_generic},	// 0xC2 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x04, 0x4E, 0x3F}, czcraft::KR_UNCRAFTABLE, "Status 20", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_20, czdec::reply_w_generic},	// 0x26 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x04, 0x9C, 0x3E}, czcraft::KR_UNCRAFTABLE, "Status 19", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_19, czdec::reply_w_generic},	// 0xC2 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x05, 0x00, 0x00}, czcraft::KR_UNCRAFTABLE, "Status 08", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_08, czdec::reply_w_generic},	// 0x50 bytes

#endif

#if HP_PROTOCOL == HP_PROTOCOL_1_8
// status frame
// not changed when hot water settings are modified
// query 01 02 03 04 0B 09 00 00 00 16 ..  => 17
// 01 02 03 04 0B 09 00 00 00 ?? Status: reply: 41 44 44 52 07 8A 65 6F DE 02 2C 72 01 02 03 04 0B 09 00 00 00 CA 8E 72 42 AE 34 06 1F FF 00 00 11 03 C2 51 41 2B 01 40 61 00 00 F8  => 2C
// always the same reply except 2 times:
/*
                                                41 44 44 52 07 8A 65 6F DE 02 2C 72 01 02 03 04 0B 09 00 00 00 CA 8E 72 42 AE 34 06 1F FF 00 00 11 03 C2 52 41 2B 01 40 61 00 00 3D (sanitary priority set to normal)
                                                41 44 44 52 07 8A 65 6F DE 02 2C 72 01 02 03 04 0B 09 00 00 00 CA 8E 72 42 AE 34 06 1F FF 00 00 11 03 C2 53 41 2B 01 40 61 00 00 7E (sanitary priority set to high)
*/


		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x00, 0x00}, czcraft::KR_UNCRAFTABLE, "Status 20 (not decoded)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_20, czdec::reply_w_generic},	// 0x26 bytes



// invalid frame query: 
// query 01 02 03 04 0B 09 00 00 00 B1 ..  => 17   ... no reply from heatpump



		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x0B, 0x04}, czcraft::KR_UNCRAFTABLE, "Hot water extra setting", czdec::cmd_r_generic, czdec::cmd_w_temp, czdec::reply_r_temp, czdec::reply_w_generic},

// Write: 01 02 03 04 0B 09 00 0D 04 B4 00 ..  => 18  Reply: 01 02 03 04 0B 09 00 0D 04 00 ..  => 17
// means 18.0°C ?
// Read:  01 02 03 04 0B 09 00 0D 04 02 ..  => 17     Reply: 01 02 03 04 0B 09 00 0D 04 B4 00 ..  => 18
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x0D, 0x04}, czcraft::KR_UNCRAFTABLE, "Hot water extra time", czdec::cmd_r_generic, czdec::cmd_w_time_minutes, czdec::reply_r_time_minutes, czdec::reply_w_generic},



		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x0E, 0x00}, czcraft::KR_UNCRAFTABLE, "Sanitary priority (get)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_sanitary_priority, czdec::empty},


		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x25, 0x01}, czcraft::KR_UNCRAFTABLE, "Hardware Settings - Adjustments - TE24 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x29, 0x04}, czcraft::KR_UNCRAFTABLE, "Junk packet, heatpump never replies to it", czdec::cmd_r_generic, czdec::empty, czdec::empty, czdec::reply_w_generic},
/* reply:
always:
41 44 44 52 07 8A 65 6F DE 02 C8 72 01 02 03 04 0B 09 00 2C 03 64 00 64 00 B4 00 BC 02 C8 00 F4 01 78 00 58 02 00 00 02 64 00 05 78 00 1E 00 52 03 2C 01 0C FE C8 00 0A 00 8C 0A 0C FE 1E 0C FE 58 02 D0 07 58 02 9C FF AC 0D FA 00 2C 01 5E 01 90 01 90 01 90 01 5E 01 2C 01 14 14 14 14 14 14 14 14 00 00 00 12 90 01 C8 00 14 00 DE 00 F9 00 10 01 27 01 3B 01 4E 01 00 00 FF 1C 02 0F 00 92 FF 8D FF 88 FF 83 FF 7E FF 79 FF 74 FF 6F FF 6A FF 65 FF 60 FF 5B FF 5B FF 5B FF 5B FF CE FF CE FF CE FF C9 FF C4 FF C4 FF BF FF BF FF BA FF BA FF B5 FF B5 FF B5 FF B5 FF B5 FF F1 FF EC FF E7 FF E2 FF DD FF D8 FF C3  => C8
except one time:
41 44 44 52 07 8A 65 6F DE 02 C8 72 01 02 03 04 0B 09 00 2C 03 64 00 64 00 B4 00 BC 02 C8 00 F4 01 78 00 58 02 00 00 02 64 00 05 78 00 1E 00 52 03 2C 01 0C FE C8 00 0A 00 8C 0A 0C FE 1E 0C FE 58 02 D0 07 58 02 9C FF AC 0D FA 00 2C 01 5E 01 90 01 90 01 90 01 5E 01 2C 01 14 14 14 14 14 14 14 14 00 00 00 12 90 01 C8 00 14 00 DE 00 F9 00 10 01 27 01 3B 01 4E 01 00 14 05 1C 02 0F 00 92 FF 8D FF 88 FF 83 FF 7E FF 79 FF 74 FF 6F FF 6A FF 65 FF 60 FF 5B FF 5B FF 5B FF 5B FF CE FF CE FF CE FF C9 FF C4 FF C4 FF BF FF BF FF BA FF BA FF B5 FF B5 FF B5 FF B5 FF B5 FF F1 FF EC FF E7 FF E2 FF DD FF D8 FF DD  => C8
*/

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x2C, 0x03}, czcraft::KR_UNCRAFTABLE, "Status xx (random mapping 1, not decoded)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_19, czdec::reply_w_generic},	// 0xC2 bytes

		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x1A, 0x04}, czcraft::KR_FAN_SPEED, "Fan speed", czdec::cmd_r_generic, czdec::cmd_w_fan_speed, czdec::reply_r_fan_speed, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x39, 0x04}, czcraft::KR_UNCRAFTABLE, "Filter change countdown", czdec::cmd_r_generic, czdec::cmd_w_time_days, czdec::reply_r_time_days, czdec::reply_w_generic},

// read: 01 02 03 04 0B 09 00 62 01 B2 68 => 17
// reply (always): 41 44 44 52 07 8A 65 6F DE 02 C8 72 01 02 03 04 0B 09 00 62 01 00 00 00 00 00 00 00 00 00 00 00 00 AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D AC 0D 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 E8 03 92 => C8
// looks like temperature history. AC 0D = 3500 (35.00°?), E8 03 = 1000 (10.00°?)

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x62, 0x01}, czcraft::KR_UNCRAFTABLE, "Status xx (random mapping 4, not decoded)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_19, czdec::reply_w_generic},	// 0xC2 bytes

		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x7D, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR0, "Hardware Settings - Adjustments - TE0 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x7E, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR1, "Hardware Settings - Adjustments - TE1 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x7F, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR2, "Hardware Settings - Adjustments - TE2 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x80, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR3, "Hardware Settings - Adjustments - TE3 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x81, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR4, "Hardware Settings - Adjustments - TE4 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x82, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR5, "Hardware Settings - Adjustments - TE5 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x83, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR6, "Hardware Settings - Adjustments - TE6 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x84, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR7, "Hardware Settings - Adjustments - TE7 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x92, 0x03}, czcraft::KR_HOT_WATER_TEMP, "Hot water calculated setting", czdec::cmd_r_generic, czdec::cmd_w_temp, czdec::reply_r_temp, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x9F, 0x02}, czcraft::KR_LED_LUMINOSITY, "LED luminosity", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},

		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0xA4, 0x02}, czcraft::KR_UNCRAFTABLE, "Holiday reduction", czdec::cmd_r_generic, czdec::cmd_w_day_delay, czdec::reply_r_day_delay, czdec::reply_w_generic},

// read: 01 02 03 04 0B 09 00 B1 00 => 17
// reply always 41 44 44 52 07 8A 65 6F DE 02 C7 72 01 02 03 04 0B 09 00 B1 00 00 00 00 00 FF 03 FF 03 99 03 99 03 FF 03 FF 03 FF 03 FF 03 FF 03 FF 03 FF 03 FF 03 FF 03 FF 03 FF 03 FF 03 00 00 00 00 0A 00 0A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 E8 03 E8 03 FF 03 E8 03 FE 01 6E 00 E8 03 E8 03 E8 03 E8 03 FF 03 E8 03 E8 03 E8 03 E8 03 E8 03 00 00 00 00 01 2E 3E 3E 2E 2E 2E 2E 40 2E 2E 2E 2E 2E 2E 2E 3C 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 48 => C7
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0xB1, 0x00}, czcraft::KR_UNCRAFTABLE, "Status xx (0xc72 bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_xc72, czdec::reply_w_generic},	// 0xc1 bytes

// read:  01 02 03 04 0B 09 00 BF 04 8B 8C  => 17
// always the same reply: 41 44 44 52 07 8A 65 6F DE 02 A1 72 01 02 03 04 0B 09 00 BF 04 B0 04 F4 01 96 00 58 02 00 00 50 00 19 00 C8 00 07 00 C8 00 40 01 B4 00 5E 01 05 05 F0 0A 09 01 60 09 AA 00 00 00 0A 0A 0A 01 C8 00 E8 03 E8 03 58 02 87 00 64 00 96 00 48 03 32 1E 00 14 14 14 00 E8 03 E8 03 00 05 01 64 00 00 2D 00 BB 03 05 1E 2C 01 0A F4 01 3C 00 05 46 00 FA 00 74 04 26 02 78 00 0A D2 00 F4 01 32 00 A0 00 D0 07 58 02 1A FF 2C 01 DA 02 05 50 00 78 B0 04 78 E8 03 E8 03 14 01 84 03 08 07 08 07 5A 00 D3 => A1
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0xBF, 0x04}, czcraft::KR_UNCRAFTABLE, "Status xx (0xa1 bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_xa1, czdec::reply_w_generic},	// 0x9b bytes

// read:  01 02 03 04 0B 09 00 C6 02 B2 65  => 17
// always the same reply: 41 44 44 52 07 8A 65 6F DE 02 C8 72 01 02 03 04 0B 09 00 C6 02 1E 00 64 00 32 00 B0 04 5A 00 B0 04 5A 00 46 00 B4 00 50 00 78 00 FA 00 32 00 12 FD 1E 00 A8 FD 80 0C E0 FC A0 0F DC 05 12 FD 1E 00 A8 FD 80 0C E0 FC A0 0F DC 05 3C 05 0F 0F 0A 06 05 00 05 00 05 00 2D 00 1E 00 19 00 19 00 19 00 05 00 06 00 32 00 00 00 00 00 00 00 00 00 00 60 6A 47 00 00 00 00 00 00 00 00 64 00 64 00 B4 00 BC 02 C8 00 F4 01 78 00 58 02 00 00 02 64 00 05 78 00 1E 00 52 03 2C 01 0C FE C8 00 0A 00 8C 0A 0C FE 1E 0C FE 58 02 D0 07 58 02 9C FF AC 0D FA 00 2C 01 5E 01 90 01 90 01 90 01 5E 01 2C 01 14 14 14 14 14 14 14 92 => C8
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0xC6, 0x02}, czcraft::KR_UNCRAFTABLE, "Status xx (random mapping 5, not decoded)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_19, czdec::reply_w_generic},	// 0xC2 bytes

// read:  01 02 03 04 0B 09 00 DB 04 8D 6A  => 17
// always the same reply: 41 44 44 52 07 8A 65 6F DE 02 A3 72 01 02 03 04 0B 09 00 DB 04 F0 0A 09 01 60 09 AA 00 00 00 0A 0A 0A 01 C8 00 E8 03 E8 03 58 02 87 00 64 00 96 00 48 03 32 1E 00 14 14 14 00 E8 03 E8 03 00 05 01 64 00 00 2D 00 BB 03 05 1E 2C 01 0A F4 01 3C 00 05 46 00 FA 00 74 04 26 02 78 00 0A D2 00 F4 01 32 00 A0 00 D0 07 58 02 1A FF 2C 01 DA 02 05 50 00 78 B0 04 78 E8 03 E8 03 14 01 84 03 08 07 08 07 5A 00 C0 03 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 D2 04 56 55 4D => A3
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0xDB, 0x04}, czcraft::KR_UNCRAFTABLE, "Status xx (0xa3 bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_xa3, czdec::reply_w_generic},	// 0x9d bytes

// read:  01 02 03 04 0B 09 00 DE 03 77 3B  => 17 
/* replies:
41 44 44 52 07 8A 65 6F DE 02 8D 72 01 02 03 04 0B 09 00 DE 03 D3 FF CE FF C9 FF C4 FF BF FF BA FF BA FF BA FF BA FF 05 1E 14 7E FF 74 FF 92 FF 88 FF CE FF C4 FF F1 FF E7 FF 2C 01 26 02 26 02 1E 00 8A 02 B4 00 DA 02 0A 0A 1E 26 02 32 00 58 02 04 0C 03 D4 FE 2C 01 00 00 2C 01 48 03 8A 02 EE 02 48 03 B6 03 E8 03 D4 FE BC 02 00 00 32 00 58 00 A4 06 10 0E 88 13 D2 00 34 03 26 02 BC 02 34 03 FC 03 1A 04 00 00 00 00 00 00 59 => 8D

 always:
except 1 time:
41 44 44 52 07 8A 65 6F DE 02 8D 72 01 02 03 04 0B 09 00 DE 03 D3 FF CE FF C9 FF C4 FF BF FF BA FF BA FF BA FF BA FF 05 1E 14 7E FF 74 FF 92 FF 88 FF CE FF C4 FF F1 FF E7 FF 2C 01 26 02 26 02 1E 00 8A 02 B4 00 DA 02 0A 0A 1E 26 02 32 00 58 02 04 0C 03 D4 FE 2C 01 00 00 2C 01 48 03 8A 02 EE 02 48 03 B6 03 E8 03 D4 FE BC 02 00 00 32 00 5A 00 A4 06 10 0E 88 13 D2 00 34 03 26 02 BC 02 34 03 FC 03 1A 04 00 00 00 00 00 00 31 => 8D
*/
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0xDE, 0x03}, czcraft::KR_UNCRAFTABLE, "Status xx (0x8d bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_x8d, czdec::reply_w_generic},	// 0x87 bytes


		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x04, 0x00}, czcraft::KR_HOUR, "Hour", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x05, 0x00}, czcraft::KR_MINUTE, "Minute", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x06, 0x00}, czcraft::KR_UNCRAFTABLE, "Second", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x07, 0x00}, czcraft::KR_DAY, "Day of Month", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x08, 0x00}, czcraft::KR_MONTH, "Month (1=Jan)", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x09, 0x00}, czcraft::KR_YEAR, "Year (20xx)", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},

// read: 01 02 03 04 0B 09 01 17 00 => 17
/* replies => 18
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 17 00 08 4D 11
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 17 00 08 4C 4F
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 17 00 0A 5C 43
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 17 00 0B 6C 39
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 17 00 0A 6C FD
*/
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x17, 0x00}, czcraft::KR_UNCRAFTABLE, "Status HP (short 2)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_short2, czdec::reply_w_generic},	// 0x18 bytes

// read:  01 02 03 04 0B 09 01 24 04 => 17
/* replies => BF
41 44 44 52 07 8A 65 6F DE 02 BF 72 01 02 03 04 0B 09 01 24 04 00 00 00 00 10 12 00 01 00 00 0A 00 00 00 14 14 03 00 01 00 01 00 01 00 B4 00 14 00 0A 00 0A 00 0A 00 0A E0 2E 35 0D 2C 01 05 78 78 14 08 07 08 07 18 15 05 B0 04 8C 0A 1E 00 10 0E 10 0E 3C 00 3C 00 3C 00 3C 00 8C 0A 23 2C 01 80 16 C1 A8 0A C7 FF FF FF 00 C0 A8 0A 01 64 01 83 00 00 2C 01 18 01 C8 00 6C 02 2C 01 6A FF 78 00 2C 01 C8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 26 02 00 02 06 00 05 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 68
41 44 44 52 07 8A 65 6F DE 02 BF 72 01 02 03 04 0B 09 01 24 04 00 00 00 00 10 14 00 00 00 00 0A 00 00 00 14 14 03 00 01 00 01 00 01 00 B4 00 14 00 0A 00 0A 00 0A 00 0A E0 2E 10 0E 2C 01 05 78 78 14 08 07 08 07 18 15 05 B0 04 8C 0A 1E 00 10 0E 10 0E 3C 00 3C 00 3C 00 3C 00 8C 0A 1F 2C 01 80 16 C1 A8 0A C7 FF FF FF 00 C0 A8 0A 01 64 01 83 00 00 2C 01 18 01 C8 00 6C 02 2C 01 6A FF 78 00 2C 01 C8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 26 02 00 02 06 00 05 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 94
*/
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x24, 0x04}, czcraft::KR_UNCRAFTABLE, "Status xx (0xbf bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_xbf, czdec::reply_w_generic},	// 0xb9 bytes

// read:  01 02 03 04 0B 09 01 47 01 02 44  => 17      reply: 41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 47 01 21 21 D7  => 18
		// related to led luminosity.
		// Luminosity = 0 => 21 01
		// Luminosity = 1 => 21 21
		// Luminosity = 2 => 21 31
		// Luminosity = 3 => 21 41
		// Luminosity = 4 => 21 51
		// Luminosity = 5 => 21 61
		// Luminosity = 6 => 21 71 (guessed, packet lost)
/* replies:
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 47 01 20 20 4D
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 47 01 21 01 F4
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 47 01 21 03 48
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 47 01 21 0B 8A
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 47 01 21 21 D7
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 47 01 21 31 4A
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 47 01 21 41 B2
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 47 01 21 51 2F
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 47 01 21 61 91
41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 47 01 A1 87 8A  <== occurs when filter countdown read 0. bit 7 of A1 is related to global alarm or filter alarm
*/
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x47, 0x01}, czcraft::KR_UNCRAFTABLE, "Status HP (short)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_02, czdec::reply_w_generic},	// 0x18 bytes

// read: 01 02 03 04 0B 09 01 85 03 52 12  => 17
// reply:
/*
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 39 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FB  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 01 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 04 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 96  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 09 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 26  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 16 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 B1  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 1B 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 20 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 D8  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 26 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 6C  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C4 00 00 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 14 5B 04 29 00 00 86 1E 00 00 00 00 00 00 2B 35 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 D3  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C5 00 04 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 40 6E 04 29 00 00 B2 31 00 00 00 00 00 00 2D 37 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 31  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C5 00 01 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 02 8B 04 29 00 00 74 4E 00 00 00 00 00 00 30 3A 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 71  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C6 00 00 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 B6 37 04 29 00 00 C8 87 00 00 00 00 00 00 16 20 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0B  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C3 00 00 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 4A 83 04 29 00 00 BC 46 00 00 00 00 00 00 00 05 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 39  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C3 00 00 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 C4 39 04 29 00 00 D6 89 00 00 00 00 00 00 07 0C 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 27  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 1C 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 74  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 22 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 43  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 27 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 AD  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 2A 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 1D  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 31 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 A5  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 35 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 8A  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 3B 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 60  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 06 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0D  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 0E 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 53  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 12 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 9E  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 17 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 70  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 1B 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 00 00 00 1E 00 00 00 05 C4 0A FF 00 87 00 48 03 00 00 95 79 04 29 00 00 07 3D 00 00 00 00 00 00 27 22 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 43  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CE 00 05 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 96 1E 04 29 00 00 A8 6E 00 00 00 00 00 00 36 04 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 42  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CE 00 02 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 58 3B 04 29 00 00 6A 8B 00 00 00 00 00 00 39 07 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 BA  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CD 00 05 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 84 4E 04 29 00 00 F6 11 00 00 00 00 00 00 3B 09 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 AC  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CD 00 02 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 46 6B 04 29 00 00 B8 2E 00 00 00 00 00 00 02 0C 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0D  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CD 00 00 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 08 88 04 29 00 00 7A 4B 00 00 00 00 00 00 05 0F 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 AE  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C3 00 04 15 00 00 00 00 70 0A EB 00 87 00 48 03 00 00 62 21 04 29 00 00 74 71 00 00 00 00 00 00 2E 38 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 47  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 D1 00 05 11 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 96 45 04 29 00 00 08 09 00 00 00 00 00 00 32 00 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 A1  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 D1 00 02 0E 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 58 62 04 29 00 00 CA 25 00 00 00 00 00 00 35 03 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 9E  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 D1 00 00 0B 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 1A 7F 04 29 00 00 8C 42 00 00 00 00 00 00 38 06 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FB  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 D1 00 00 09 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 A6 05 04 29 00 00 B8 55 00 00 00 00 00 00 3A 08 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 5D  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 D1 00 00 06 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 68 22 04 29 00 00 7A 72 00 00 00 00 00 00 01 0B 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 87  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 D1 00 00 03 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 2A 3F 04 29 00 00 9C 02 00 00 00 00 00 00 04 0E 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 56  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 D0 00 05 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 EC 5B 04 29 00 00 5E 1F 00 00 00 00 00 00 07 11 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 2D  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 D0 00 02 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 AE 78 04 29 00 00 20 3C 00 00 00 00 00 00 0A 14 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 1D  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CA 00 04 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 D0 08 04 29 00 00 E2 58 00 00 00 00 00 00 0D 17 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FE  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C7 00 05 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 28 2F 04 29 00 00 3A 7F 00 00 00 00 00 00 11 1B 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 5B  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C7 00 02 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 EA 4B 04 29 00 00 5C 0F 00 00 00 00 00 00 14 1E 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 45  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C7 00 05 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 64 02 04 29 00 00 76 52 00 00 00 00 00 00 1B 25 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 F6  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C6 00 05 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 52 32 04 29 00 00 64 82 00 00 00 00 00 00 20 2A 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 F2  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C6 00 02 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 14 4F 04 29 00 00 86 12 00 00 00 00 00 00 23 2D 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 8D  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C5 00 05 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 6C 75 04 29 00 00 DE 38 00 00 00 00 00 00 27 31 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 73  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C5 00 01 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 24 0F 04 29 00 00 36 5F 00 00 00 00 00 00 2B 35 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 15  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CA 00 02 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 48 15 04 29 00 00 5A 65 00 00 00 00 00 00 29 33 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 92  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CA 00 00 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 74 28 04 29 00 00 86 78 00 00 00 00 00 00 2B 35 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 C1  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CA 00 00 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 36 45 04 29 00 00 A8 08 00 00 00 00 00 00 2E 38 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 8D  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CC 00 03 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 46 05 04 29 00 00 58 55 00 00 00 00 00 00 36 04 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 08  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CC 00 00 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 08 22 04 29 00 00 1A 72 00 00 00 00 00 00 39 07 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 79  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CD 00 02 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 CA 3E 04 29 00 00 3C 02 00 00 00 00 00 00 00 0A 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 18  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CD 00 00 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 22 65 04 29 00 00 94 28 00 00 00 00 00 00 04 0E 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 39  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CE 00 04 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 E4 81 04 29 00 00 56 45 00 00 00 00 00 00 07 11 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 B4  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 CE 00 01 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 06 12 04 29 00 00 18 62 00 00 00 00 00 00 0A 14 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 34  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C3 00 04 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 82 5D 04 29 00 00 F4 20 00 00 00 00 00 00 20 25 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 F9  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C3 00 00 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 D0 00 04 29 00 00 E2 50 00 00 00 00 00 00 25 2A 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 7F  => 68
41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C3 00 00 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 28 27 04 29 00 00 3A 77 00 00 00 00 00 00 29 2E 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 57  => 68
*/
//       41 44 44 52 07 8A 65 6F DE 02 68 72 01 02 03 04 0B 09 01 85 03 C6 00 02 00 00 00 00 00 3C 0A DF 00 87 00 48 03 00 00 14 4F 04 29 00 00 86 12 00 00 00 00 00 00 23 2D 00 00 00 00 00 22 FC FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 8D  => 68

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x85, 0x03}, czcraft::KR_UNCRAFTABLE, "Status xx (0x68 bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_x68, czdec::reply_w_generic},	// 0x62 bytes

// read: 01 02 03 04 0B 09 01 B2 00 => 17
/* replies:  => C7
41 44 44 52 07 8A 65 6F DE 02 C7 72 01 02 03 04 0B 09 01 B2 00 22 FC 22 FC 22 FC 22 FC 22 FC 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 04 00 00 03 02 00 48 80 61 42 ED 22 6F 43 BC 88 D9 43 DC 98 61 43 5C 7D D4 43 23 4E DB 43 C8 7C 34 42 48 C1 EA 43 6A 6C 5F 42 00 00 00 00 00 00 00 00 00 00 82 43 EF F4 61 42 8C 3B 6F 43 12 68 DA 43 91 6F 61 43 F7 30 D6 43 A8 97 DB 43 0D C0 2B 42 B8 93 EA 43 C6 33 3F 42 00 00 00 00 00 00 00 00 00 00 82 43 0D 00 7F C0 20 20 20 20 00 00 47 00 83 00 A0 00 00 00 00 00 FF 03 B1 03 87 13 1F 16 FC 18 EC 15 1A
41 44 44 52 07 8A 65 6F DE 02 C7 72 01 02 03 04 0B 09 01 B2 00 22 FC 22 FC 22 FC 22 FC 22 FC 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 04 04 00 00 09 02 01 00 CF 7C 62 42 11 00 6F 43 73 90 D3 43 EF FF 61 43 1D FE B1 43 2A B3 D5 43 A9 A7 C4 42 08 79 EC 43 FA B5 14 43 00 00 00 00 00 00 00 00 00 00 82 43 33 8A 62 42 11 00 6F 43 05 C6 D3 43 D4 FF 61 43 0E 50 B2 43 66 DC D5 43 66 97 BF 42 2E 65 EC 43 BF 32 13 43 00 00 00 00 00 00 00 00 00 00 82 43 0D 00 00 C0 20 20 20 20 00 00 44 00 70 00 89 00 00 00 00 00 FF 03 B0 03 88 13 1F 16 D0 18 EC 15 27
41 44 44 52 07 8A 65 6F DE 02 C7 72 01 02 03 04 0B 09 01 B2 00 22 FC 22 FC 22 FC 22 FC 22 FC 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 11 00 64 42 11 00 70 43 EC FB D8 43 EF FF 60 43 33 40 2B 44 D3 8D E8 43 3C 00 A0 C0 4F F6 E4 43 61 00 E0 C0 00 00 00 00 00 00 00 00 00 00 82 43 11 00 64 42 11 00 70 43 EE ED D8 43 D4 FF 60 43 4C 41 2B 44 D8 85 E8 43 9D 02 A0 C0 A8 DD E4 43 08 04 E0 C0 00 00 00 00 00 00 00 00 00 00 82 43 0D 00 7F C0 21 21 21 21 00 00 4B 00 65 00 A0 00 BF 00 00 00 FF 03 AC 03 89 13 21 16 FD 18 EA 15 90
*/
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0xB2, 0x00}, czcraft::KR_UNCRAFTABLE, "Status xx (0xc7 bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_xc7, czdec::reply_w_generic},	// 0xc1 bytes

// read: 01 02 03 04 0B 09 01 B3 04 => 17
// reply always: 41 44 44 52 07 8A 65 6F DE 02 20 72 01 02 03 04 0B 09 01 B3 04 02 06 00 05 00 00 00 00 00 00 C7 => 20
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0xB3, 0x04}, czcraft::KR_UNCRAFTABLE, "Status xx (0x20 bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_x20, czdec::reply_w_generic},	// 0x1A bytes

// read: 01 02 03 04 0B 09 01 C4 02 => 17
/* replies:  => C5
41 44 44 52 07 8A 65 6F DE 02 C5 72 01 02 03 04 0B 09 01 C4 02 00 00 00 00 22 FC C8 00 1C 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 0A 22 02 00 00 00 00 26 02 AC 01 04 01 6B 00 18 00 CD 00 60 00 00 00 3C 01 00 00 CC FC CC FC CC FC CC FC 00 00 00 00 00 00 00 00 D2 00 00 00 D2 00 34 03 A4 06 88 13 EB 00 1D 01 E0 01 00 00 00 00 00 00 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 5C 00 00 00 00 00 04 00 00 3C 00 00 B8 0B 00 00 00 00 00 00 30 2A 2C 00 5A 00 3C 01 00 00 00 00 B9 FF B9 FF 22 FC 22 FC 22 FC 22 FC 22 FC 22 FC 22 FC 88
41 44 44 52 07 8A 65 6F DE 02 C5 72 01 02 03 04 0B 09 01 C4 02 00 00 00 00 22 FC C8 00 1C 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 0A 22 02 00 00 00 00 26 02 B7 01 0B 01 5B 00 1D 00 D2 00 2A 00 00 00 3C 01 00 00 CC FC CC FC CC FC CC FC 00 00 00 00 00 00 00 00 D2 00 00 00 D2 00 34 03 A4 06 88 13 EB 00 1D 01 E0 01 00 00 00 00 00 00 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 D4 00 00 00 00 00 04 00 00 3C 00 00 B8 0B 00 00 00 00 00 00 30 2A 85 00 5A 00 3C 01 00 00 00 00 F4 FF F4 FF 22 FC 22 FC 22 FC 22 FC 22 FC 22 FC 22 FC A5
41 44 44 52 07 8A 65 6F DE 02 C5 72 01 02 03 04 0B 09 01 C4 02 00 00 00 00 22 FC C8 00 1C 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 0A 22 02 00 00 00 00 26 02 DD 01 24 01 4F 00 24 00 E2 00 FB FF 00 00 3C 01 3C 01 4E FF 94 FF B8 FF FA FD 00 00 3C 01 3C 01 3C 01 D2 00 00 00 E2 00 34 03 1F 07 88 13 EB 00 1D 01 E0 01 00 00 00 00 00 00 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 96 09 00 00 00 00 96 09 00 00 96 09 00 00 73 04 00 00 00 00 00 00 08 00 00 3C 00 00 0A 01 00 00 00 00 00 00 30 2A 00 00 E7 03 00 00 DC 00 02 00 DC 00 DC 00 22 FC 22 FC 22 FC 22 FC 22 FC 22 FC 22 FC 0C
*/
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0xC4, 0x02}, czcraft::KR_UNCRAFTABLE, "Status xx (0xc5 bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_xc5, czdec::reply_w_generic},	// 0xbf bytes

// read: 01 02 03 04 0B 09 01 CC 02 => 17
/* replies:  => AD
41 44 44 52 07 8A 65 6F DE 02 AD 72 01 02 03 04 0B 09 01 CC 02 1C 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 0A 22 02 00 00 00 00 26 02 AB 01 04 01 6C 00 18 00 CD 00 62 00 00 00 3C 01 00 00 CC FC CC FC CC FC CC FC 00 00 00 00 00 00 00 00 D2 00 00 00 D2 00 34 03 A4 06 88 13 EB 00 1D 01 E0 01 00 00 00 00 00 00 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 54 00 00 00 00 00 0B 00 00 3C 00 00 B8 0B 00 00 00 00 00 00 30 2A 24 00 5A 00 3C 01 00 00 00 00 B7 FF 81
41 44 44 52 07 8A 65 6F DE 02 AD 72 01 02 03 04 0B 09 01 CC 02 1C 02 00 00 05 05 A8 02 00 00 00 00 00 00 00 00 00 03 0A 22 02 00 00 00 00 26 02 F0 01 31 01 52 00 25 00 EA 00 07 00 00 00 3C 01 3C 01 65 FF 8B FF BA FF E1 FD 00 00 3C 01 3C 01 3C 01 00 00 00 00 EA 00 34 03 5C 07 88 13 EB 00 1D 01 E0 01 00 00 00 00 00 00 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 96 09 00 00 00 00 96 09 00 00 96 09 00 00 9D 04 00 00 00 00 D4 01 0F 00 00 3C 00 00 07 0A 00 00 00 00 00 00 30 2A 00 00 5A 00 00 00 DC 00 D9 FF B5 00 C1
41 44 44 52 07 8A 65 6F DE 02 AD 72 01 02 03 04 0B 09 01 CC 02 1C 02 00 00 6C 05 0F 03 00 00 00 00 00 00 00 00 00 03 0A 22 02 00 00 00 00 26 02 EB 01 2D 01 54 00 23 00 E8 00 10 00 00 00 3C 01 3C 01 5F FF 84 FF B5 FF DC FD 00 00 3C 01 3C 01 3C 01 00 00 00 00 E8 00 34 03 4D 07 88 13 EB 00 1D 01 E0 01 00 00 00 00 00 00 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 96 09 00 00 00 00 96 09 00 00 96 09 00 00 92 04 00 00 00 00 3B 02 0D 00 00 3C 00 00 6E 0A 00 00 00 00 00 00 30 2A 00 00 5A 00 00 00 DC 00 E0 FF B6 00 75
41 44 44 52 07 8A 65 6F DE 02 AD 72 01 02 03 04 0B 09 01 CC 02 1C 02 00 00 7D 05 20 03 00 00 00 00 00 00 00 00 00 03 0A 22 02 00 00 00 00 26 02 EA 01 2D 01 55 00 23 00 E8 00 11 00 00 00 3C 01 3C 01 5E FF 83 FF B5 FF E2 FD 00 00 3C 01 3C 01 3C 01 00 00 00 00 E8 00 34 03 4D 07 88 13 EB 00 1D 01 E0 01 00 00 00 00 00 00 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 96 09 00 00 00 00 96 09 00 00 96 09 00 00 8F 04 00 00 00 00 4C 02 0F 00 00 3C 00 00 7F 0A 00 00 00 00 00 00 30 2A 00 00 5A 00 00 00 DC 00 E7 FF BC 00 ED
41 44 44 52 07 8A 65 6F DE 02 AD 72 01 02 03 04 0B 09 01 CC 02 1C 02 00 00 88 06 2B 04 00 00 00 00 00 00 00 00 00 03 0A 22 02 00 00 00 00 26 02 CD 01 19 01 74 00 18 00 DA 00 78 00 00 00 3C 01 3C 01 3A FF 36 FF 9A FF 2F FD 00 00 3C 01 3C 01 3C 01 00 00 00 00 DA 00 34 03 E2 06 88 13 EB 00 1D 01 E0 01 00 00 00 00 00 00 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 96 09 00 00 00 00 96 09 00 00 96 09 00 00 4F 04 00 00 00 00 57 03 0C 00 00 3C 00 00 8A 0B 00 00 00 00 00 00 30 2A 00 00 5A 00 00 00 DC 00 6E FF 4A 00 4E
41 44 44 52 07 8A 65 6F DE 02 AD 72 01 02 03 04 0B 09 01 CC 02 1C 02 00 00 A6 04 49 02 00 00 00 00 00 00 00 00 00 03 0A 22 02 00 00 00 00 26 02 F1 01 31 01 50 00 26 00 EB 00 03 00 00 00 3C 01 3C 01 67 FF 8E FF BD FF F8 FD 00 00 3C 01 3C 01 3C 01 00 00 00 00 00 01 34 03 FD 07 88 13 1D 01 E0 01 00 00 00 00 00 00 00 00 3B 01 FE 01 00 00 00 00 00 00 00 00 B4 14 00 00 96 09 00 00 00 00 96 09 00 00 96 09 00 00 9F 04 00 00 00 00 75 01 0A 00 00 3C 00 00 A8 09 00 00 00 00 00 00 30 2A 00 00 E7 03 00 00 DC 00 EB FF C7 00 9D
41 44 44 52 07 8A 65 6F DE 02 AD 72 01 02 03 04 0B 09 01 CC 02 1C 02 00 00 F5 06 98 04 00 00 00 00 00 00 00 00 00 03 0A 22 02 00 00 00 00 26 02 74 01 E3 00 6F 00 14 00 B6 00 6A 00 00 00 3C 01 00 00 CC FC CC FC CC FC CC FC 00 00 00 00 00 00 00 00 00 00 00 00 D2 00 34 03 A4 06 88 13 EB 00 1D 01 E0 01 00 00 00 00 00 00 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 3B 00 00 00 00 00 01 00 00 3C 00 00 B8 0B 00 00 00 00 00 00 30 2A 00 00 5A 00 00 00 00 00 00 00 EE FF 56
41 44 44 52 07 8A 65 6F DE 02 AD 72 01 02 03 04 0B 09 01 CC 02 1C 02 00 00 F7 05 9A 03 00 00 00 00 00 00 00 00 00 03 0A 22 02 00 00 00 00 26 02 E0 01 26 01 57 00 21 00 E3 00 19 00 00 00 3C 01 3C 01 51 FF 7D FF B0 FF AD FD 00 00 3C 01 3C 01 3C 01 00 00 00 00 E3 00 34 03 27 07 88 13 EB 00 1D 01 E0 01 00 00 00 00 00 00 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 96 09 00 00 00 00 96 09 00 00 96 09 00 00 79 04 00 00 00 00 C6 02 02 00 00 3C 00 00 F9 0A 00 00 00 00 00 00 30 2A 00 00 5A 00 00 00 DC 00 C0 FF 9C 00 ED
*/
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0xCC, 0x02}, czcraft::KR_UNCRAFTABLE, "Status xx (0xad bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_xad, czdec::reply_w_generic},	// 0xa7 bytes

// read: 01 02 03 04 0B 09 01 F5 01 => 18
/* replies:  => 40
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 46 01 00 00 00 00 00 00 00 00 28 00 E1 00 31 00 CC 02 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 07
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 46 01 00 00 00 00 00 00 00 00 28 00 E1 00 39 00 C4 02 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 AD
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 46 01 00 00 00 00 00 00 00 00 28 00 E2 00 87 05 76 FD 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 D5
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 46 01 00 00 00 00 00 00 00 00 28 00 E2 00 8F 05 6E FD 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 59
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 46 01 00 00 00 00 00 00 00 00 28 00 E2 00 97 05 66 FD 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 98
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 46 01 00 00 00 00 00 00 00 00 28 00 E2 00 A0 05 5D FD 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 79
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E1 00 20 00 DD 02 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 A6
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E2 00 1A 05 E3 FD 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 C2
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E2 00 20 00 DD 02 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 77 00 77 00 E0 01 D2 00 42
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E2 00 20 00 DD 02 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 9C
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E2 00 23 05 DA FD 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 EC
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E2 00 2B 05 D2 FD 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 77 00 77 00 DF 01 D2 00 4A
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E2 00 2B 05 D2 FD 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 46
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E2 00 52 04 AB FE 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 55
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E2 00 8D 04 70 FE 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 E0
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E2 00 B6 00 47 02 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 EE
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E2 00 CF 04 2E FE 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 DB
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 47 01 00 00 00 00 00 00 00 00 28 00 E2 00 E2 01 1B 01 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 D8
41 44 44 52 07 8A 65 6F DE 02 40 72 01 02 03 04 0B 09 01 F5 01 E2 00 9C FF 00 00 00 00 00 00 28 00 E1 00 71 03 8C FF 00 00 FD 02 51 03 00 00 00 00 00 00 51 03 00 00 78 00 78 00 E0 01 D2 00 44
*/
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0xF5, 0x01}, czcraft::KR_UNCRAFTABLE, "Status xx (0x40 bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_x40, czdec::reply_w_generic},	// 0x38 bytes


// read: 01 02 03 04 0B 09 03 14 0F => 17 ?? Always the same reply:
// 41 44 44 52 07 8A 65 6F DE 02 26 72 01 02 03 04 0B 09 03 14 0F 09 00 00 08 09 17 32 07 00 00 00 08 00 00 00 07 2D => 26
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x03, 0x14, 0x0F}, czcraft::KR_UNCRAFTABLE, "Status xx (0x26 bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_x26, czdec::reply_w_generic},	// 0x1E bytes

// read: 01 02 03 04 0B 09 03 21 0F => 17
// reply: always 41 44 44 52 07 8A 65 6F DE 02 6D 72 01 02 03 04 0B 09 03 21 0F 00 00 07 01 02 00 06 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 77 => 6D
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x03, 0x21, 0x0F}, czcraft::KR_UNCRAFTABLE, "Status xx (0x6d bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_x6d, czdec::reply_w_generic},	// 0x69 bytes

// 01 02 03 04 0B 09 03 6F 0E ?? Always the same reply:
// 41 44 44 52 07 8A 65 6F DE 02 58 72 01 02 03 04 0B 09 03 6F 0E 11 03 12 00 00 00 00 00 00 1D 0C 16 0C 00 00 01 12 00 00 00 00 00 00 07 01 17 12 30 15 02 12 00 00 00 00 00 00 07 01 17 12 30 19 03 12 00 00 00 00 00 00 08 01 17 0B 39 17 00 00 00 00 00 00 00 00 00 E1 => 58
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x03, 0x6F, 0x0E}, czcraft::KR_UNCRAFTABLE, "Status xx (0x58 bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_x58, czdec::reply_w_generic},	// 0x52 bytes


// read:  01 02 03 04 0B 09 03 BD 0D B2 DB  => 17 ??? no reply

// read:  01 02 03 04 0B 09 00 39 04 00 00 EA  => 18    reply: no reply
// read:  no query                                      reply: 41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 00 39 04 00 00 EA  => 18

// read: 01 02 03 04 0B 09 04 4E 3F => 17
// reply always : 41 44 44 52 07 8A 65 6F DE 02 2C 72 01 02 03 04 0B 09 04 4E 3F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 6A => 2C
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x04, 0x4E, 0x3F}, czcraft::KR_UNCRAFTABLE, "Status xx (0x2c bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_x2c, czdec::reply_w_generic},	// 0x26 bytes


// 01 02 03 04 0B 09 05 28 00  => 17
/* replies: => 2E
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 33 FC 3A 00 77 71 05 00 39 42 0F 00 30 87 0B 00 85 56 1D 00 A5
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 33 FC 3A 00 77 71 05 00 39 42 0F 00 30 87 0B 00 86 56 1D 00 2D
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 50 FC 3A 00 77 71 05 00 56 42 0F 00 37 87 0B 00 8E 56 1D 00 41
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 51 FC 3A 00 77 71 05 00 56 42 0F 00 37 87 0B 00 8E 56 1D 00 8A
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 51 FC 3A 00 77 71 05 00 56 42 0F 00 37 87 0B 00 8F 56 1D 00 05
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 57 FC 3A 00 77 71 05 00 5D 42 0F 00 39 87 0B 00 90 56 1D 00 45
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 20 FC 3A 00 77 71 05 00 26 42 0F 00 2C 87 0B 00 7E 56 1D 00 CF
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 21 FC 3A 00 77 71 05 00 27 42 0F 00 2C 87 0B 00 7E 56 1D 00 6C
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 33 FC 3A 00 77 71 05 00 39 42 0F 00 30 87 0B 00 83 56 1D 00 AC
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 33 FC 3A 00 77 71 05 00 39 42 0F 00 30 87 0B 00 84 56 1D 00 2A
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 49 FC 3A 00 77 71 05 00 4E 42 0F 00 35 87 0B 00 8D 56 1D 00 2B
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 49 FC 3A 00 77 71 05 00 4F 42 0F 00 36 87 0B 00 8D 56 1D 00 86
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 4A FC 3A 00 77 71 05 00 4F 42 0F 00 36 87 0B 00 8D 56 1D 00 C2
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 34 FC 3A 00 77 71 05 00 39 42 0F 00 30 87 0B 00 87 56 1D 00 E1
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 34 FC 3A 00 77 71 05 00 3A 42 0F 00 30 87 0B 00 88 56 1D 00 C3
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 35 FC 3A 00 77 71 05 00 3A 42 0F 00 30 87 0B 00 88 56 1D 00 08
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 35 FC 3A 00 77 71 05 00 3A 42 0F 00 31 87 0B 00 88 56 1D 00 4B
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 35 FC 3A 00 77 71 05 00 3B 42 0F 00 31 87 0B 00 88 56 1D 00 23
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 36 FC 3A 00 77 71 05 00 3B 42 0F 00 31 87 0B 00 88 56 1D 00 67
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 36 FC 3A 00 77 71 05 00 3C 42 0F 00 31 87 0B 00 88 56 1D 00 66
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 37 FC 3A 00 77 71 05 00 3C 42 0F 00 31 87 0B 00 88 56 1D 00 AD
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 37 FC 3A 00 77 71 05 00 3D 42 0F 00 31 87 0B 00 88 56 1D 00 C5
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 38 FC 3A 00 77 71 05 00 3D 42 0F 00 31 87 0B 00 88 56 1D 00 88
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 40 FC 3A 00 77 71 05 00 45 42 0F 00 33 87 0B 00 8A 56 1D 00 28
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 40 FC 3A 00 77 71 05 00 46 42 0F 00 33 87 0B 00 8A 56 1D 00 90
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 41 FC 3A 00 77 71 05 00 46 42 0F 00 33 87 0B 00 8B 56 1D 00 D4
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 41 FC 3A 00 77 71 05 00 47 42 0F 00 34 87 0B 00 8B 56 1D 00 6C
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 42 FC 3A 00 77 71 05 00 47 42 0F 00 34 87 0B 00 8B 56 1D 00 28
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 0E FC 3A 00 77 71 05 00 14 42 0F 00 27 87 0B 00 79 56 1D 00 AB
41 44 44 52 07 8A 65 6F DE 02 2E 72 01 02 03 04 0B 09 05 28 00 00 00 00 00 0F FC 3A 00 77 71 05 00 14 42 0F 00 27 87 0B 00 79 56 1D 00 60
*/
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x05, 0x28, 0x00}, czcraft::KR_UNCRAFTABLE, "Status xx (0x2e bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_x2e, czdec::reply_w_generic},	// 0x28 bytes

// 01 02 03 04 0B 09 04 9C 3E ??

//  Hot water extra: on/off
//        01 02 03 04 0B 09 41 17 00 FE FF ..  => 18    reply: 41 44 44 52 07 8A 65 6F DE 02 17 77 01 02 03 04 0B 09 41 17 00 6E ..  => 17
// write: 01 02 03 04 0B 09 41 17 00 FF FE ..  => 18    reply: 41 44 44 52 07 8A 65 6F DE 02 17 77 01 02 03 04 0B 09 41 17 00 4A ..  => 17
// read:  01 02 03 04 0B 09 01 17 00 02 ..  => 17       reply: 41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 17 00 08 4A ..  => 18
		/* @OK */ { {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x41, 0x17, 0x00}, czcraft::KR_EXTRA_HOT_WATER_OFF, "Extra hot water - off", czdec::cmd_r_generic, czdec::cmd_w_extra_hot_water, czdec::reply_r_extra_hot_water, czdec::reply_w_generic},

/* read:  01 02 04 0B 09 00 2C 03 B2 0C 41  => 17 ???   reply:
41 44 44 52 07 8A 65 6F DE 02 C8 72 01 02 03 04 0B 09 00 2C 03 64 00 64 00 B4 00 BC 02 C8 00 F4 01 78 00 58 02 00 00 02 64 00 05 78 00 1E 00 52 03 2C 01 0C FE C8 00 0A 00 8C 0A 0C FE 1E 0C FE 58 02 D0 07 58 02 9C FF AC 0D FA 00 2C 01 5E 01 90 01 90 01 90 01 5E 01 2C 01 14 14 14 14 14 14 14 14 00 00 00 12 90 01 C8 00 14 00 DE 00 F9 00 10 01 27 01 3B 01 4E 01 00 00 FF 1C 02 0F 00 92 FF 8D FF 88 FF 83 FF 7E FF 79 FF 74 FF 6F FF 6A FF 65 FF 60 FF 5B FF 5B FF 5B FF 5B FF CE FF CE FF CE FF C9 FF C4 FF C4 FF BF FF BF FF BA FF BA FF B5 FF B5 FF B5 FF B5 FF B5 FF F1 FF EC FF E7 FF E2 FF DD FF D8 FF C3  => C8
41 44 44 52 07 8A 65 6F DE 02 C8 72 01 02 03 04 0B 09 00 2C 03 64 00 64 00 B4 00 BC 02 C8 00 F4 01 78 00 58 02 00 00 02 64 00 05 78 00 1E 00 52 03 2C 01 0C FE C8 00 0A 00 8C 0A 0C FE 1E 0C FE 58 02 D0 07 58 02 9C FF AC 0D FA 00 2C 01 5E 01 90 01 90 01 90 01 5E 01 2C 01 14 14 14 14 14 14 14 14 00 00 00 12 90 01 C8 00 14 00 DE 00 F9 00 10 01 27 01 3B 01 4E 01 00 00 FF 1C 02 0F 00 92 FF 8D FF 88 FF 83 FF 7E FF 79 FF 74 FF 6F FF 6A FF 65 FF 60 FF 5B FF 5B FF 5B FF 5B FF CE FF CE FF CE FF C9 FF C4 FF C4 FF BF FF BF FF BA FF BA FF B5 FF B5 FF B5 FF B5 FF B5 FF F1 FF EC FF E7 FF E2 FF DD FF D8 FF C3  => C8

*/

// 01 02 03 04 0B 09 03 0B 0D => 17
// always same reply: 41 44 44 52 07 8A 65 6F DE 02 C8 72 01 02 03 04 0B 09 03 0B 0D 0C 08 01 17 0C 00 00 01 12 00 00 00 00 00 00 09 01 17 00 02 0F 02 12 00 00 00 00 00 00 09 01 17 00 02 11 03 12 00 00 00 00 00 00 09 01 17 00 02 32 01 12 00 00 00 00 00 00 09 01 17 06 0D 39 02 12 00 00 00 00 00 00 09 01 17 06 0E 00 03 12 00 00 00 00 00 00 09 01 17 0C 00 00 01 12 00 00 00 00 00 00 09 01 17 0C 00 26 02 12 00 00 00 00 00 00 09 01 17 0C 00 2A 03 12 00 00 00 00 00 00 09 01 17 0C 33 17 01 12 00 00 00 00 00 00 09 01 17 0C 33 21 02 12 00 00 00 00 00 00 09 01 17 0C 33 26 03 12 00 00 00 00 00 00 0E 0C 16 0C 38 09 00 00 00 A6 => C8

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x03, 0x0B, 0x0D}, czcraft::KR_UNCRAFTABLE, "Status xx (random mapping 3, not decoded)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_19, czdec::reply_w_generic},	// 0xC2 bytes

// 01 02 03 04 0B 09 03 BD 0D B2 DB  => 17
//   reply: 41 44 44 52 07 8A 65 6F DE 02 C8 72 01 02 03 04 0B 09 03 BD 0D 00 00 00 00 00 0B 01 17 16 10 30 02 12 00 00 00 00 00 00 0B 01 17 16 10 32 03 12 00 00 00 00 00 00 0B 01 17 16 11 15 01 12 00 00 00 00 00 00 0B 01 17 16 11 21 02 12 00 00 00 00 00 00 1A ** 0C 16 0C 00 00 01 12 ** 00 00 00 00 00 00 1A 0C 16 10 37 37 02 12 00 00 00 00 00 00 1A 0C 16 10 3A 21 03 12 00 00 00 00 00 00 1B 0C 16 0C 00 00 01 12 00 00 00 00 00 00 1B 0C 16 0D 16 26 02 12 00 00 00 00 00 00 1B 0C 16 0D 16 2A 03 12 00 00 00 00 00 00 1C 0C 16 0C 00 00 01 12 00 00 00 00 00 00 1C 0C 16 0D 2F 0E 02 12 00 00 00 00 00 00 1C 0C 16 0D 2F 2A  => C8
// after reseting filter countdown
//   reply: 41 44 44 52 07 8A 65 6F DE 02 C8 72 01 02 03 04 0B 09 03 BD 0D 00 00 00 00 00 0B 01 17 16 10 30 02 12 00 00 00 00 00 00 0B 01 17 16 10 32 03 12 00 00 00 00 00 00 0B 01 17 16 11 15 01 12 00 00 00 00 00 00 0B 01 17 16 11 21 02 12 00 00 00 00 00 00 0B ** 01 17 16 11 25 03 12 ** 00 00 00 00 00 00 1A 0C 16 10 37 37 02 12 00 00 00 00 00 00 1A 0C 16 10 3A 21 03 12 00 00 00 00 00 00 1B 0C 16 0C 00 00 01 12 00 00 00 00 00 00 1B 0C 16 0D 16 26 02 12 00 00 00 00 00 00 1B 0C 16 0D 16 2A 03 12 00 00 00 00 00 00 1C 0C 16 0C 00 00 01 12 00 00 00 00 00 00 1C 0C 16 0D 2F 0E 02 12 00 00 00 00 00 00 1C 0C 16 0D 2F 18  => C8



		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x03, 0xBD, 0x0D}, czcraft::KR_UNCRAFTABLE, "Status xx (random mapping 2, not decoded)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_19, czdec::reply_w_generic},	// 0xC2 bytes
// read:  01 90 01 C8 00 14 00 DE 00 F9 00  => 17 ??? no reply

// status frame ???
// reply: 41 44 44 52 07 8A 65 6F DE 02 78 72 -- 01 01 01 01 01 01 01 01 01 02 03 41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 44 52 17 52 01 02 03 04 0B 09 01 F5 01 2A 56 41 09 01 CC 02 97 6F DE 02 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 02 03 04 0B 09 01 85 03 52 12 41 44 44 52 07 8A 03 04 0B 09 01 B3 04 0A 6A 41 44 44 52 07 8A 65 0B 09 03 0B 0D 17 1C 0A -- 16  => 78

// read: 01 02 03 04 0B 09 05 00 00  => 17
/* replies => 56
41 44 44 52 07 8A 65 6F DE 02 56 72 01 02 03 04 0B 09 05 00 00 37 07 22 01 11 01 23 00 00 00 00 00 01 01 01 01 56 14 00 00 17 0F 00 00 9F 0A 00 00 65 06 00 00 00 00 00 00 BC 34 00 00 00 00 00 00 33 FC 3A 00 77 71 05 00 39 42 0F 00 30 87 0B 00 86 56 1D 00 0E
41 44 44 52 07 8A 65 6F DE 02 56 72 01 02 03 04 0B 09 05 00 00 33 59 21 01 11 01 23 00 00 00 00 00 00 00 00 00 56 14 00 00 17 0F 00 00 9F 0A 00 00 65 06 00 00 00 00 00 00 BC 34 00 00 00 00 00 00 21 FC 3A 00 77 71 05 00 26 42 0F 00 2C 87 0B 00 7E 56 1D 00 A3
*/
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x05, 0x00, 0x00}, czcraft::KR_UNCRAFTABLE, "Status xx (0x56 bytes long frame)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_status_v180_x56, czdec::reply_w_generic},	// 0x50 bytes

// write:  01 02 03 04 0B 09 81 17 00 ???
/*
write 65 6F DE 02 D3 5E 41 44 44 52 18 57 01 02 03 04 0B 09 81 17 00 00 01 9A  => 18
reply 41 44 44 52 07 8A 65 6F DE 02 17 77 01 02 03 04 0B 09 81 17 00 4D BD  => 17
write 65 6F DE 02 D3 5E 41 44 44 52 18 57 01 02 03 04 0B 09 81 17 00 01 00 00  => 18
reply 41 44 44 52 07 8A 65 6F DE 02 17 77 01 02 03 04 0B 09 81 17 00 5C 7E  => 17
write 65 6F DE 02 D3 5E 41 44 44 52 18 57 01 02 03 04 0B 09 81 17 00 01 00 00  => 18
reply 41 44 44 52 07 8A 65 6F DE 02 17 77 01 02 03 04 0B 09 81 17 00 6C C0  => 17
*/

#endif
		{ {0}, czcraft::KR_UNCRAFTABLE, NULL, NULL, NULL, NULL, NULL}
	};

// convert a craftname into index into kr_decoder_array
czdec::KNOWN_REGISTER *czdec::kr_craft_name_to_index(czcraft::KNOWN_REGISTER_CRAFT_NAME reg_cname)
{
	int i = 0;

	while(kr_decoder[i].reg_name != NULL)
	{
		if(reg_cname == kr_decoder[i].reg_cname)
		{
			return &(kr_decoder[i]);
		}

		i++;
	}

	DPRINT("kr_craft_name_to_index failed to find craftname ");
	DPRINTLN(reg_cname);

	return NULL;
}

// process a received frame and adjust comfortzone_status accordingly
comfortzone_heatpump::PROCESSED_FRAME_TYPE czdec::process_frame(comfortzone_heatpump *cz_class, CZ_PACKET_HEADER *czph)
{
	int i = 0;

	while(kr_decoder[i].reg_name != NULL)
	{
		if(!memcmp(czph->reg_num, kr_decoder[i].reg_num, 9))
		{
			switch(czph->cmd)
			{
				case 'R':
							DPRINT(kr_decoder[i].reg_name);
							DPRINTLN(" (get): ");

							kr_decoder[i].cmd_r(cz_class, &kr_decoder[i], (R_CMD*)czph);

							DPRINTLN("====================================================");
							return comfortzone_heatpump::PFT_QUERY;

				case 'W':
							DPRINT(kr_decoder[i].reg_name);
							DPRINTLN(" (set): ");

							kr_decoder[i].cmd_w(cz_class, &kr_decoder[i], (W_CMD*)czph);

							DPRINTLN("====================================================");
							return comfortzone_heatpump::PFT_QUERY;
												
				case 'r':
							DPRINT(kr_decoder[i].reg_name);
							DPRINTLN(" (reply get): ");

							kr_decoder[i].reply_r(cz_class, &kr_decoder[i], (R_REPLY*)czph);

							DPRINTLN("====================================================");
							return comfortzone_heatpump::PFT_REPLY;

				case 'w':
							DPRINT(kr_decoder[i].reg_name);
							DPRINTLN(" (reply set): ");

							kr_decoder[i].reply_w(cz_class, &kr_decoder[i], (W_REPLY*)czph);

							DPRINTLN("====================================================");
							return comfortzone_heatpump::PFT_REPLY;
			}
		}

		i++;
	}

	DPRINTLN("unknown register");
	dump_frame(cz_class, "UNK:");
	return comfortzone_heatpump::PFT_UNKNOWN;
}

void czdec::dump_frame(comfortzone_heatpump *cz_class, const char *prefix)
{
	int i;

	DPRINTLN("==============================");
	DPRINT(prefix);

	for(i = 0; i < cz_class->cz_size; i++)
	{
		if(cz_class->cz_buf[i] < 0x10)
			DPRINT("0");
		DPRINT(cz_class->cz_buf[i], HEX);

		DPRINT(" ");
	}

	DPRINT(" => ");
	DPRINTLN(cz_class->cz_size, HEX);
}

