#include "comfortzone_heatpump.h"
#include "comfortzone_config.h"
#include "comfortzone_frame.h"
#include "comfortzone_status.h"

#include "comfortzone_decoder_basic.h"
#include "comfortzone_decoder_status.h"

static byte cz_buf[256];
static int cz_size = 0;
static int cz_frame_type = -1;
static int cz_full_frame_size = -1;

COMFORTZONE_STATUS comfortzone_status;

static KNOWN_REGISTER kr_decoder[] =
	{
		// don't know why but extra hot water off does not use the same message as extra hot water on
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x41, 0x19, 0x00}, KR_EXTRA_HOT_WATER_ON, "Extra hot water - off", czdec_cmd_r_generic, czdec_cmd_w_extra_hot_water, czdec_reply_r_extra_hot_water, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x81, 0x19, 0x00}, KR_EXTRA_HOT_WATER_OFF, "Extra hot water - on", czdec_cmd_r_generic, czdec_cmd_w_extra_hot_water, czdec_reply_r_extra_hot_water, czdec_reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x40, 0x00, 0x00}, KR_UNCRAFTABLE, "Daylight saving - on", czdec_cmd_r_generic, czdec_cmd_w_daylight_saving, czdec_reply_r_daylight_saving, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x80, 0x00, 0x00}, KR_UNCRAFTABLE, "Daylight saving - off", czdec_cmd_r_generic, czdec_cmd_w_daylight_saving, czdec_reply_r_daylight_saving, czdec_reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x80, 0x0E, 0x00}, KR_UNCRAFTABLE, "Sanitary priority - bug: same code as minimal return temperature", czdec_cmd_r_generic, czdec_cmd_w_sanitary_priority, czdec_reply_r_sanitary_priority, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x80, 0x0E, 0x00}, KR_UNCRAFTABLE, "Minimal return temperature", czdec_cmd_r_generic, czdec_cmd_w_temp, czdec_reply_r_temp, czdec_reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x00, 0x00}, KR_UNCRAFTABLE, "Status 09", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_09, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x00, 0x04}, KR_FAN_SPEED, "Fan speed", czdec_cmd_r_generic, czdec_cmd_w_fan_speed, czdec_reply_r_fan_speed, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x05, 0x04}, KR_UNCRAFTABLE, "Fan boost increase", czdec_cmd_r_generic, czdec_cmd_w_percentage, czdec_reply_r_percentage, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x16, 0x02}, KR_UNCRAFTABLE, "Status 11", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_11, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x17, 0x04}, KR_UNCRAFTABLE, "Supply fan T12 adjust", czdec_cmd_r_generic, czdec_cmd_w_percentage, czdec_reply_r_percentage, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x19, 0x00}, KR_UNCRAFTABLE, "Status 24", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_24, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x29, 0x04}, KR_UNCRAFTABLE, "Chauffage - compressor max frequency", czdec_cmd_r_generic, czdec_cmd_w_freq, czdec_reply_r_freq, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x2C, 0x04}, KR_UNCRAFTABLE, "Status 12", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_12, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x35, 0x04}, KR_UNCRAFTABLE, "Heatpump - compressor - blocked frequency 1", czdec_cmd_r_generic, czdec_cmd_w_freq, czdec_reply_r_freq, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x37, 0x04}, KR_UNCRAFTABLE, "Heatpump - compressor - blocked frequency 2", czdec_cmd_r_generic, czdec_cmd_w_freq, czdec_reply_r_freq, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x39, 0x04}, KR_UNCRAFTABLE, "Heatpump - compressor - blocked frequency 3", czdec_cmd_r_generic, czdec_cmd_w_freq, czdec_reply_r_freq, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x64, 0x01}, KR_UNCRAFTABLE, "Status 25", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_25, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7A, 0x03}, KR_UNCRAFTABLE, "Status 01", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_01, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7B, 0x00}, KR_UNCRAFTABLE, "Hardware Settings - Adjustments - TE0 Adjust", czdec_cmd_r_generic, czdec_cmd_w_temp_1byte, czdec_reply_r_temp_1byte, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7C, 0x00}, KR_UNCRAFTABLE, "Hardware Settings - Adjustments - TE1 Adjust", czdec_cmd_r_generic, czdec_cmd_w_temp_1byte, czdec_reply_r_temp_1byte, czdec_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7D, 0x00}, KR_UNCRAFTABLE, "Hardware Settings - Adjustments - TE2 Adjust", czdec_cmd_r_generic, czdec_cmd_w_temp_1byte, czdec_reply_r_temp_1byte, czdec_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7D, 0x03}, KR_HOT_WATER_TEMP, "Hot water calculated setting", czdec_cmd_r_generic, czdec_cmd_w_temp, czdec_reply_r_temp, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7E, 0x00}, KR_UNCRAFTABLE, "Hardware Settings - Adjustments - TE3 Adjust", czdec_cmd_r_generic, czdec_cmd_w_temp_1byte, czdec_reply_r_temp_1byte, czdec_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7F, 0x00}, KR_UNCRAFTABLE, "Hardware Settings - Adjustments - TE4 Adjust", czdec_cmd_r_generic, czdec_cmd_w_temp_1byte, czdec_reply_r_temp_1byte, czdec_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x80, 0x00}, KR_UNCRAFTABLE, "Hardware Settings - Adjustments - TE5 Adjust", czdec_cmd_r_generic, czdec_cmd_w_temp_1byte, czdec_reply_r_temp_1byte, czdec_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x81, 0x00}, KR_UNCRAFTABLE, "Hardware Settings - Adjustments - TE6 Adjust", czdec_cmd_r_generic, czdec_cmd_w_temp_1byte, czdec_reply_r_temp_1byte, czdec_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x82, 0x00}, KR_UNCRAFTABLE, "Hardware Settings - Adjustments - TE7 Adjust", czdec_cmd_r_generic, czdec_cmd_w_temp_1byte, czdec_reply_r_temp_1byte, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xA6, 0x04}, KR_UNCRAFTABLE, "Chauffage - delai dégivrage", czdec_cmd_r_generic, czdec_cmd_w_time, czdec_reply_r_time, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xA7, 0x02}, KR_LED_LUMINOSITY, "LED luminosity", czdec_cmd_r_generic, czdec_cmd_w_digit, czdec_reply_r_digit, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xAC, 0x02}, KR_UNCRAFTABLE, "Holiday reduction", czdec_cmd_r_generic, czdec_cmd_w_day_delay, czdec_reply_r_day_delay, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xC8, 0x02}, KR_UNCRAFTABLE, "Status 10", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_10, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xB0, 0x02}, KR_ROOM_HEATING_TEMP, "Heating calculated setting", czdec_cmd_r_generic, czdec_cmd_w_temp, czdec_reply_r_temp, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xB2, 0x00}, KR_UNCRAFTABLE, "Status 23", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_23, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xDE, 0x04}, KR_UNCRAFTABLE, "Status 13", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_13, czdec_reply_w_generic},	// 0x63 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x00, 0x00}, KR_UNCRAFTABLE, "Status 02", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_02, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x04, 0x00}, KR_UNCRAFTABLE, "Hour", czdec_cmd_r_generic, czdec_cmd_w_digit, czdec_reply_r_digit, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x05, 0x00}, KR_UNCRAFTABLE, "Minute", czdec_cmd_r_generic, czdec_cmd_w_digit, czdec_reply_r_digit, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x07, 0x00}, KR_UNCRAFTABLE, "Day of Month", czdec_cmd_r_generic, czdec_cmd_w_digit, czdec_reply_r_digit, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x08, 0x00}, KR_UNCRAFTABLE, "Month (1=Jan)", czdec_cmd_r_generic, czdec_cmd_w_digit, czdec_reply_r_digit, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x09, 0x00}, KR_UNCRAFTABLE, "Year (20xx)", czdec_cmd_r_generic, czdec_cmd_w_digit, czdec_reply_r_digit, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x16, 0x02}, KR_UNCRAFTABLE, "Status 05", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_05, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x19, 0x00}, KR_UNCRAFTABLE, "Status 22", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_22, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x2C, 0x04}, KR_UNCRAFTABLE, "Status 14", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_14, czdec_reply_w_generic},	// 0x48 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x58, 0x04}, KR_UNCRAFTABLE, "Language", czdec_cmd_r_generic, czdec_cmd_w_language, czdec_reply_r_language, czdec_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x64, 0x01}, KR_UNCRAFTABLE, "Status 04", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_04, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x7A, 0x03}, KR_UNCRAFTABLE, "Status 07", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_07, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0xB2, 0x00}, KR_UNCRAFTABLE, "Status 03", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_03, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0xC8, 0x02}, KR_UNCRAFTABLE, "Status 06", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_06, czdec_reply_w_generic},	// 0xC2 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0x15, 0x0D}, KR_UNCRAFTABLE, "Status 15", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_15, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0x2B, 0x0F}, KR_UNCRAFTABLE, "Status 18", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_18, czdec_reply_w_generic},	// 0x5D bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0x79, 0x0E}, KR_UNCRAFTABLE, "Status 17", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_17, czdec_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0xC7, 0x0D}, KR_UNCRAFTABLE, "Status 16", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_16, czdec_reply_w_generic},	// 0xC2 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x04, 0x4E, 0x3F}, KR_UNCRAFTABLE, "Status 20", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_20, czdec_reply_w_generic},	// 0x26 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x04, 0x9C, 0x3E}, KR_UNCRAFTABLE, "Status 19", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_19, czdec_reply_w_generic},	// 0xC2 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x05, 0x00, 0x00}, KR_UNCRAFTABLE, "Status 08", czdec_cmd_r_generic, czdec_empty, czdec_reply_r_status_08, czdec_reply_w_generic},	// 0x50 bytes

		{ {0}, KR_UNCRAFTABLE, NULL, NULL, NULL, NULL, NULL}
	};

// convert a craftname into index into kr_decoder_array
static int kr_craft_name_to_index(KNOWN_REGISTER_CRAFT_NAME reg_cname)
{
	int i;

	while(kr_decoder[i].reg_name != NULL)
	{
		if(reg_cname == kr_decoder[i].reg_cname)
		{
			return i;
		}

		i++;
	}

	DPRINT("kr_craft_name_to_index failed to find craftname ");
	DPRINTLN(reg_cname);

	return -1;
}

static void dump_frame(const char *prefix)
{
	int i;

	DPRINTLN("==============================");
	DPRINT(prefix);

	for(i = 0; i < cz_size; i++)
	{
		if(cz_buf[i] < 0x10)
			DPRINT("0");
		DPRINT(cz_buf[i], HEX);

		DPRINT(" ");
	}

	DPRINT(" => ");
	DPRINT(cz_size, HEX);
}



static void comfortzone_process_frame(int frame_type, byte *reg_num)
{
	int i = 0;

	while(kr_decoder[i].reg_name != NULL)
	{
		if(!memcmp(reg_num + 1, kr_decoder[i].reg_num, 9))
		{

			switch(frame_type)
			{
				case FRAME_TYPE_02_CMD_p2:
												if(reg_num[0] == 'R')
												{
#ifdef DEBUG
													NPRINT(kr_decoder[i].reg_name);
													NPRINTLN(" (get): ");
#endif

													kr_decoder[i].cmd_r(&kr_decoder[i], (R_CMD*)cz_buf);

													DPRINTLN("====================================================");
													return;
												}
												else if(reg_num[0] == 'W')
												{
#ifdef DEBUG
													NPRINT(kr_decoder[i].reg_name);
													NPRINTLN(" (set): ");
#endif

													kr_decoder[i].cmd_w(&kr_decoder[i], (W_CMD*)cz_buf);

													DPRINTLN("====================================================");
													return;
												}
												break;
												
				case FRAME_TYPE_02_REPLY:
												if(reg_num[0] == 'r')
												{
#ifdef DEBUG
													NPRINT(kr_decoder[i].reg_name);
													NPRINTLN(" (reply get): ");
#endif

													kr_decoder[i].reply_r(&kr_decoder[i], (R_REPLY*)cz_buf);

													DPRINTLN("====================================================");
													return;
												}
												else if(reg_num[0] == 'w')
												{
#ifdef DEBUG
													NPRINT(kr_decoder[i].reg_name);
													NPRINTLN(" (reply set): ");
#endif

													kr_decoder[i].reply_w(&kr_decoder[i], (W_REPLY*)cz_buf);

													DPRINTLN("====================================================");
													return;
												}
												break;
												
			}
		}

		i++;
	}

	NPRINTLN("unknown register");
	dump_frame("UNK:");
}

static void comfortzone_process_frame15()
{
	// une frame de ce type commence toujours par
	// 65 6F DE 15 FF FF FF FF 00 F4 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 14 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	// peut-être s'agit-il de la remontée des alarmes
}

bool comfortzone_receive(byte input_byte)
{
	// all frame start by 0x65 0x6F 0xDE
	switch(cz_size)
	{
		case 0:
					if(input_byte != 0x65)
						return false;

					cz_buf[cz_size++] = input_byte;
					return false;
				
		case 1:
					if(input_byte != 0x6F)
						return false;

					cz_buf[cz_size++] = input_byte;
					return false;

		case 2:
					if(input_byte != 0xDE)
						return false;

					cz_buf[cz_size++] = input_byte;
					return false;

		case 3:	
					switch(input_byte)
					{
						case FRAME_TYPE_01:		// empty frame ?
								cz_size = 0;
								break;

						case FRAME_TYPE_02:		// either a command frame or a command response frame
								cz_buf[cz_size++] = input_byte;
								cz_frame_type = FRAME_TYPE_02;
								break;

						case FRAME_TYPE_15:		// broadcast frame ?
								cz_buf[cz_size++] = input_byte;
								cz_frame_type = FRAME_TYPE_15;
								cz_full_frame_size = 0x74;			// size of frame
								break;
					}
					return false;

		default:
					switch(cz_frame_type)
					{
						case FRAME_TYPE_02:
								cz_buf[cz_size++] = input_byte;

								if(input_byte == 0xD3)
								{	// it a type 2 command frame, 6 additionnal bytes are required to have frame size
									cz_frame_type = FRAME_TYPE_02_CMD_p1;
								}
								else
								{
									cz_frame_type = FRAME_TYPE_02_REPLY;
									cz_full_frame_size = input_byte - 6;
								}
								return false;

						case FRAME_TYPE_02_CMD_p1:
								cz_buf[cz_size++] = input_byte;

								if(cz_size == 11)
								{
									cz_frame_type = FRAME_TYPE_02_CMD_p2;
									cz_full_frame_size = input_byte + 6;
								}

								return false;

						case FRAME_TYPE_02_CMD_p2:
								cz_buf[cz_size++] = input_byte;

								if(cz_size == cz_full_frame_size)
								{
									comfortzone_process_frame(FRAME_TYPE_02_CMD_p2, cz_buf + 11);;
									cz_size = 0;
									return true;
								}

								return false;

							case FRAME_TYPE_02_REPLY:
								cz_buf[cz_size++] = input_byte;

								if(cz_size == cz_full_frame_size)
								{
									comfortzone_process_frame(FRAME_TYPE_02_REPLY, cz_buf + 5);
									cz_size = 0;
									return true;
								}

								return false;

						case FRAME_TYPE_15:
								cz_buf[cz_size++] = input_byte;

								if(cz_size == cz_full_frame_size)
								{
									comfortzone_process_frame15();
									cz_size = 0;
									return true;
								}

								return false;

					}

					// we should never come here
					cz_size = 0;
					return false;
	}
}

extern "C"
{
	static W_CMD empty_w_cmd_packet =
										{
											.header = { 0x65, 0x6F, 0xDE },
											.frame_type = 0x02,
											.unknown1 = { 0xD3, 0x5E },
											.addr1 = { 0x41, 0x44, 0x44, 0x52 },
											.packet_size = 0x18,
											.cmd = 'W',
											.reg_num = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
											.reg_value = { 0x00, 0x00 }, 
											.crc = 0x00,
											.addr2 = { 0x41, 0x44, 0x44, 0x52 },
											.unknown2 = { 0x07, 0x8A }
										};
}

// craft a W_CMD paquet
// input: pointer to output buffer
//        9 byte array contaning register number
//       16bit value (it will be automatically stored into little endian)
//        1 CRC byte
static int cz_craft_w_cmd(byte *output_buffer, byte *reg_num, uint16_t value, byte crc)
{
	W_CMD *q = (W_CMD*)output_buffer;

	memcpy(output_buffer, (byte*)&empty_w_cmd_packet, sizeof(W_CMD));

	memcpy(q->reg_num, reg_num, 9);
	q->reg_value[0] = value & 0xFF;
	q->reg_value[1] = (value >> 8) & 0xFF;
	q->crc = crc;
	return sizeof(W_CMD);
}


extern "C"
{
	static W_SMALL_CMD empty_w_small_cmd_packet =
										{
											.header = { 0x65, 0x6F, 0xDE },
											.frame_type = 0x02,
											.unknown1 = { 0xD3, 0x5E },
											.addr1 = { 0x41, 0x44, 0x44, 0x52 },
											.packet_size = 0x17,
											.cmd = 'W',
											.reg_num = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
											.reg_value = 0x00,
											.crc = 0x00,
											.addr2 = { 0x41, 0x44, 0x44, 0x52 },
											.unknown2 = { 0x07, 0x8A }
										};
}

// craft a W_SMALL_CMD paquet
// input: pointer to output buffer
//        9 byte array contaning register number
//        8bit value
//        1 CRC byte
static int cz_craft_w_small_cmd(byte *output_buffer, byte *reg_num, byte value, byte crc)
{
	W_SMALL_CMD *q = (W_SMALL_CMD*)output_buffer;

	memcpy(output_buffer, (byte*)&empty_w_small_cmd_packet, sizeof(W_SMALL_CMD));

	memcpy(q->reg_num, reg_num, 9);
	q->reg_value = value;
	q->crc = crc;
	return sizeof(W_SMALL_CMD);
}



// craft one command frame
// input: pointer to output buffer, min size is sizeof(W_CMD) = 30 bytes
//        name of the command to craft
//        parameter of the command (depend on crafted command, see KNOWN_REGISTER_CRAFT_NAME enum)
// output: 0 = uncraftable packet or crafting error else number of bytes used in buffer
uint16_t comfortzone_craft(byte *output_buffer, KNOWN_REGISTER_CRAFT_NAME reg_cname, uint16_t parameter)
{
	int kr_idx;

	kr_idx = kr_craft_name_to_index(reg_cname);

	switch(reg_cname)
	{
		case KR_UNCRAFTABLE:				// uncraftable packet
									break;

		case KR_FAN_SPEED:				// set fan speed, parameter => 1=slow, 2=normal, 3=fast
									break;

		case KR_LED_LUMINOSITY:			// set led luminosity, parameter => 0=off to 6=full power
									break;

		case KR_ROOM_HEATING_TEMP:		// set room heating temperature, parameter => 120 (=12.0°) to 240 (=24.0°)
									break;

		case KR_HOT_WATER_TEMP:			// set hot water temperature, parameter => 120 (=12.0°) to 800 (=80.0°)
									break;

		case KR_EXTRA_HOT_WATER_ON:	// enable extra hot water, no parameter
									break;

		case KR_EXTRA_HOT_WATER_OFF:	// disable extra hot water, no parameter
									break;

	}

	return 0;
}

