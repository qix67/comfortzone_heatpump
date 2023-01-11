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
//                                              41 44 44 52 07 8A 65 6F DE 02 2C 72 01 02 03 04 0B 09 00 00 00 CA 8E 72 42 AE 34 06 1F FF 00 00 11 03 C2 51 41 2B 01 40 61 00 00 F8  => 2C
//                                              41 44 44 52 07 8A 65 6F DE 02 2C 72 01 02 03 04 0B 09 00 00 00 CA 8E 72 42 AE 34 06 1F FF 00 00 11 03 C2 51 41 2B 01 40 61 00 00 F8  => 2C
//                                              41 44 44 52 07 8A 65 6F DE 02 2C 72 01 02 03 04 0B 09 00 00 00 CA 8E 72 42 AE 34 06 1F FF 00 00 11 03 C2 51 41 2B 01 40 61 00 00 F8  => 2C
//                                              41 44 44 52 07 8A 65 6F DE 02 2C 72 01 02 03 04 0B 09 00 00 00 CA 8E 72 42 AE 34 06 1F FF 00 00 11 03 C2 51 41 2B 01 40 61 00 00 F8  => 2C
//                                              41 44 44 52 07 8A 65 6F DE 02 2C 72 01 02 03 04 0B 09 00 00 00 CA 8E 72 42 AE 34 06 1F FF 00 00 11 03 C2 51 41 2B 01 40 61 00 00 F8  => 2C




// invalid frame query: 
// query 01 02 03 04 0B 09 00 00 00 B1 ..  => 17   ... no reply from heatpump



		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x0B, 0x04}, czcraft::KR_UNCRAFTABLE, "Hot water extra setting (not supported)", czdec::cmd_r_generic, czdec::cmd_w_temp, czdec::reply_r_temp, czdec::reply_w_generic},

// Write: 01 02 03 04 0B 09 00 0D 04 B4 00 ..  => 18  Reply: 01 02 03 04 0B 09 00 0D 04 00 ..  => 17
// means 18.0°C ?
// Read:  01 02 03 04 0B 09 00 0D 04 02 ..  => 17     Reply: 01 02 03 04 0B 09 00 0D 04 B4 00 ..  => 18
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x0D, 0x04}, czcraft::KR_ROOM_HEATING_TEMP, "Heating calculated setting", czdec::cmd_r_generic, czdec::cmd_w_temp, czdec::reply_r_temp, czdec::reply_w_generic},



		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x0E, 0x00}, czcraft::KR_UNCRAFTABLE, "Sanitary priority (get)", czdec::cmd_r_generic, czdec::empty, czdec::reply_r_sanitary_priority, czdec::empty},


		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x25, 0x01}, czcraft::KR_UNCRAFTABLE, "Hot water extra time (not supported)", czdec::cmd_r_generic, czdec::cmd_w_time, czdec::reply_r_time, czdec::reply_w_generic},
// write: 01 02 03 04 0B 09 00 25 01 04 ..  => 17   Reply: 01 02 03 04 0B 09 00 25 01 3C 00 00 00 ..  => 1A
// write: 01 02 03 04 0B 09 00 25 01 3D ..  => 17   Reply: 01 02 03 04 0B 09 00 25 01 3D ..  => 17
// read:  01 02 03 04 0B 09 00 25 01 01 ..  => 17   Reply: 01 02 03 04 0B 09 00 25 01 3D ..  => 17
// write: 01 02 03 04 0B 09 00 25 01 3C ..  => 17   Reply: 01 02 03 04 0B 09 00 25 01 3C ..  => 17



		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x1A, 0x04}, czcraft::KR_FAN_SPEED, "Fan speed", czdec::cmd_r_generic, czdec::cmd_w_fan_speed, czdec::reply_r_fan_speed, czdec::reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x39, 0x04}, czcraft::KR_UNCRAFTABLE, "Filter change countdown", czdec::cmd_r_generic, czdec::cmd_w_time_days, czdec::reply_r_time_days, czdec::reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x7D, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR0, "Hardware Settings - Adjustments - TE0 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x7E, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR1, "Hardware Settings - Adjustments - TE1 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x7F, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR2, "Hardware Settings - Adjustments - TE2 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x80, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR3, "Hardware Settings - Adjustments - TE3 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x81, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR4, "Hardware Settings - Adjustments - TE4 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x82, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR5, "Hardware Settings - Adjustments - TE5 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x83, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR6, "Hardware Settings - Adjustments - TE6 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x84, 0x00}, czcraft::KR_TEMP_OFFSET_SENSOR7, "Hardware Settings - Adjustments - TE7 Adjust", czdec::cmd_r_generic, czdec::cmd_w_temp_1byte, czdec::reply_r_temp_1byte, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x92, 0x03}, czcraft::KR_HOT_WATER_TEMP, "Hot water calculated setting", czdec::cmd_r_generic, czdec::cmd_w_temp, czdec::reply_r_temp, czdec::reply_w_generic},

// read                                             reply: 01 02 03 04 0B 09 00 92 03 1D 02 ..  => 18 

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0x9F, 0x02}, czcraft::KR_LED_LUMINOSITY, "LED luminosity", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x00, 0xA4, 0x02}, czcraft::KR_UNCRAFTABLE, "Holiday reduction", czdec::cmd_r_generic, czdec::cmd_w_day_delay, czdec::reply_r_day_delay, czdec::reply_w_generic},

// read:  01 02 03 04 0B 09 00 DE 03 77 3B  => 17 ???

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x04, 0x00}, czcraft::KR_HOUR, "Hour", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x05, 0x00}, czcraft::KR_MINUTE, "Minute", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
// 01 02 03 04 0B 09 01 06 00 ?? KR_SECOND ?
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x07, 0x00}, czcraft::KR_DAY, "Day of Month", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x08, 0x00}, czcraft::KR_MONTH, "Month (1=Jan)", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x01, 0x09, 0x00}, czcraft::KR_YEAR, "Year (20xx)", czdec::cmd_r_generic, czdec::cmd_w_digit, czdec::reply_r_digit, czdec::reply_w_generic},
// 01 02 03 04 0B 09 01 47 01 ??

// 01 02 03 04 0B 09 03 6F 0E ??
// read:  01 02 03 04 0B 09 03 BD 0D B2 DB  => 17 ??? no reply

// read:  01 02 03 04 0B 09 00 39 04 00 00 EA  => 18    reply: no reply
// read:  no query                                      reply: 41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 00 39 04 00 00 EA  => 18



// 01 02 03 04 0B 09 04 9C 3E ??

//  Hot water extra: on/off
//        01 02 03 04 0B 09 41 17 00 FE FF ..  => 18    reply: 41 44 44 52 07 8A 65 6F DE 02 17 77 01 02 03 04 0B 09 41 17 00 6E ..  => 17
// write: 01 02 03 04 0B 09 41 17 00 FF FE ..  => 18    reply: 41 44 44 52 07 8A 65 6F DE 02 17 77 01 02 03 04 0B 09 41 17 00 4A ..  => 17
// read:  01 02 03 04 0B 09 01 17 00 02 ..  => 17       reply: 41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 0B 09 01 17 00 08 4A ..  => 18
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x09, 0x41, 0x17, 0x00}, czcraft::KR_EXTRA_HOT_WATER_OFF, "Extra hot water - off", czdec::cmd_r_generic, czdec::cmd_w_extra_hot_water, czdec::reply_r_extra_hot_water, czdec::reply_w_generic},

// read:  01 02 04 0B 09 00 2C 03 B2 0C 41  => 17 ??? no reply
// read:  01 90 01 C8 00 14 00 DE 00 F9 00  => 17 ??? no reply

// status frame ???
// reply: 41 44 44 52 07 8A 65 6F DE 02 78 72 -- 01 01 01 01 01 01 01 01 01 02 03 41 44 44 52 07 8A 65 6F DE 02 18 72 01 02 03 04 44 52 17 52 01 02 03 04 0B 09 01 F5 01 2A 56 41 09 01 CC 02 97 6F DE 02 FF 00 3B 01 FE 01 00 00 00 00 00 00 B4 14 00 00 02 03 04 0B 09 01 85 03 52 12 41 44 44 52 07 8A 03 04 0B 09 01 B3 04 0A 6A 41 44 44 52 07 8A 65 0B 09 03 0B 0D 17 1C 0A -- 16  => 78


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

