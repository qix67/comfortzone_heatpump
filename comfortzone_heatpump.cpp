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
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x41, 0x19, 0x00}, "Extra hot water - off", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_extra_hot_water, comfortzone_decoder_reply_r_extra_hot_water, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x81, 0x19, 0x00}, "Extra hot water - on", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_extra_hot_water, comfortzone_decoder_reply_r_extra_hot_water, comfortzone_decoder_reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x40, 0x00, 0x00}, "Daylight saving - on", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_daylight_saving, comfortzone_decoder_reply_r_daylight_saving, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x80, 0x00, 0x00}, "Daylight saving - off", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_daylight_saving, comfortzone_decoder_reply_r_daylight_saving, comfortzone_decoder_reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x80, 0x0E, 0x00}, "Sanitary priority - bug: same code as minimal return temperature", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_sanitary_priority, comfortzone_decoder_reply_r_sanitary_priority, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x80, 0x0E, 0x00}, "Minimal return temperature", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_temp, comfortzone_decoder_reply_r_temp, comfortzone_decoder_reply_w_generic},

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x00, 0x00}, "Status 09", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_09, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x00, 0x04}, "Fan speed", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_fan_speed, comfortzone_decoder_reply_r_fan_speed, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x05, 0x04}, "Fan boost increase", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_percentage, comfortzone_decoder_reply_r_percentage, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x16, 0x02}, "Status 11", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_11, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x17, 0x04}, "Supply fan T12 adjust", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_percentage, comfortzone_decoder_reply_r_percentage, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x19, 0x00}, "Status 24", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_24, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x29, 0x04}, "Chauffage - compressor max frequency", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_freq, comfortzone_decoder_reply_r_freq, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x2C, 0x04}, "Status 12", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_12, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x35, 0x04}, "Heatpump - compressor - blocked frequency 1", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_freq, comfortzone_decoder_reply_r_freq, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x37, 0x04}, "Heatpump - compressor - blocked frequency 2", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_freq, comfortzone_decoder_reply_r_freq, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x39, 0x04}, "Heatpump - compressor - blocked frequency 3", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_freq, comfortzone_decoder_reply_r_freq, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x64, 0x01}, "Status 25", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_25, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7A, 0x03}, "Status 01", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_01, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7B, 0x00}, "Hardware Settings - Adjustments - TE0 Adjust", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_temp_1byte, comfortzone_decoder_reply_r_temp_1byte, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7C, 0x00}, "Hardware Settings - Adjustments - TE1 Adjust", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_temp_1byte, comfortzone_decoder_reply_r_temp_1byte, comfortzone_decoder_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7D, 0x00}, "Hardware Settings - Adjustments - TE2 Adjust", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_temp_1byte, comfortzone_decoder_reply_r_temp_1byte, comfortzone_decoder_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7D, 0x03}, "Hot water calculated setting", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_temp, comfortzone_decoder_reply_r_temp, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7E, 0x00}, "Hardware Settings - Adjustments - TE3 Adjust", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_temp_1byte, comfortzone_decoder_reply_r_temp_1byte, comfortzone_decoder_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x7F, 0x00}, "Hardware Settings - Adjustments - TE4 Adjust", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_temp_1byte, comfortzone_decoder_reply_r_temp_1byte, comfortzone_decoder_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x80, 0x00}, "Hardware Settings - Adjustments - TE5 Adjust", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_temp_1byte, comfortzone_decoder_reply_r_temp_1byte, comfortzone_decoder_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x81, 0x00}, "Hardware Settings - Adjustments - TE6 Adjust", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_temp_1byte, comfortzone_decoder_reply_r_temp_1byte, comfortzone_decoder_reply_w_generic},	// not tested
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0x82, 0x00}, "Hardware Settings - Adjustments - TE7 Adjust", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_temp_1byte, comfortzone_decoder_reply_r_temp_1byte, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xA6, 0x04}, "Chauffage - delai dégivrage", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_time, comfortzone_decoder_reply_r_time, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xA7, 0x02}, "LED luminosity", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_digit, comfortzone_decoder_reply_r_digit, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xAC, 0x02}, "Holiday reduction", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_day_delay, comfortzone_decoder_reply_r_day_delay, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xC8, 0x02}, "Status 10", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_10, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xB0, 0x02}, "Heating calculated setting", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_temp, comfortzone_decoder_reply_r_temp, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xB2, 0x00}, "Status 23", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_23, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x00, 0xDE, 0x04}, "Status 13", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_13, comfortzone_decoder_reply_w_generic},	// 0x63 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x00, 0x00}, "Status 02", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_02, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x04, 0x00}, "Hour", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_digit, comfortzone_decoder_reply_r_digit, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x05, 0x00}, "Minute", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_digit, comfortzone_decoder_reply_r_digit, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x07, 0x00}, "Day of Month", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_digit, comfortzone_decoder_reply_r_digit, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x08, 0x00}, "Month (1=Jan)", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_digit, comfortzone_decoder_reply_r_digit, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x09, 0x00}, "Year (20xx)", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_digit, comfortzone_decoder_reply_r_digit, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x16, 0x02}, "Status 05", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_05, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x19, 0x00}, "Status 22", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_22, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x2C, 0x04}, "Status 14", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_14, comfortzone_decoder_reply_w_generic},	// 0x48 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x58, 0x04}, "Language", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_cmd_w_language, comfortzone_decoder_reply_r_language, comfortzone_decoder_reply_w_generic},
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x64, 0x01}, "Status 04", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_04, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0x7A, 0x03}, "Status 07", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_07, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0xB2, 0x00}, "Status 03", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_03, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x01, 0xC8, 0x02}, "Status 06", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_06, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0x15, 0x0D}, "Status 15", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_15, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0x2B, 0x0F}, "Status 18", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_18, comfortzone_decoder_reply_w_generic},	// 0x5D bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0x79, 0x0E}, "Status 17", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_17, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x03, 0xC7, 0x0D}, "Status 16", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_16, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x04, 0x4E, 0x3F}, "Status 20", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_20, comfortzone_decoder_reply_w_generic},	// 0x26 bytes
		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x04, 0x9C, 0x3E}, "Status 19", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_19, comfortzone_decoder_reply_w_generic},	// 0xC2 bytes

		{ {0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x05, 0x00, 0x00}, "Status 08", comfortzone_decoder_cmd_r_generic, comfortzone_decoder_empty, comfortzone_decoder_reply_r_status_08, comfortzone_decoder_reply_w_generic},	// 0x50 bytes

		{ {0}, NULL, NULL, NULL, NULL, NULL}
	};

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
													NPRINT(kr_decoder[i].reg_name);
													NPRINTLN(" (get): ");

													kr_decoder[i].cmd_r(&kr_decoder[i], (R_CMD*)cz_buf);
													return;
												}
												else if(reg_num[0] == 'W')
												{
													NPRINT(kr_decoder[i].reg_name);
													NPRINTLN(" (set): ");

													kr_decoder[i].cmd_w(&kr_decoder[i], (W_CMD*)cz_buf);
													return;
												}
												break;
												
				case FRAME_TYPE_02_REPLY:
												if(reg_num[0] == 'r')
												{
													NPRINT(kr_decoder[i].reg_name);
													NPRINTLN(" (reply get): ");

													kr_decoder[i].reply_r(&kr_decoder[i], (R_REPLY*)cz_buf);
													return;
												}
												else if(reg_num[0] == 'w')
												{
													NPRINT(kr_decoder[i].reg_name);
													NPRINTLN(" (reply set): ");

													kr_decoder[i].reply_w(&kr_decoder[i], (W_REPLY*)cz_buf);
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

