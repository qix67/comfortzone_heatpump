#ifndef _COMFORTZONE_DECODER_BASIC_H
#define _COMFORTZONE_DECODER_BASIC_H

#include "comfortzone_frame.h"

typedef struct known_register
{
	byte reg_num[9];
	const char *reg_name;
	void (*cmd_r)(struct known_register *kr, R_CMD *p);		// FRAME_TYPE_02_CMD_p2, R command
	void (*cmd_w)(struct known_register *kr, W_CMD *p);		// FRAME_TYPE_02_CMD_p2, W command
	void (*reply_r)(struct known_register *kr, R_REPLY *p);	// FRAME_TYPE_02_REPLY, r reply
	void (*reply_w)(struct known_register *kr, W_REPLY *p);	// FRAME_TYPE_P2_REPLY, w reply
} KNOWN_REGISTER;

void comfortzone_decoder_cmd_r_generic(KNOWN_REGISTER *kr, R_CMD *p);
void comfortzone_decoder_cmd_w_generic_2byte(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_temp(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_temp_1byte(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_freq(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_time(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_percentage(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_extra_hot_water(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_daylight_saving(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_sanitary_priority(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_day_delay(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_fan_speed(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_language(KNOWN_REGISTER *kr, W_CMD *p);
void comfortzone_decoder_cmd_w_digit(KNOWN_REGISTER *kr, W_CMD *p);

void comfortzone_decoder_reply_r_generic_2byte(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_temp(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_temp_1byte(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_freq(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_time(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_percentage(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_extra_hot_water(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_daylight_saving(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_sanitary_priority(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_day_delay(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_fan_speed(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_language(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_r_digit(KNOWN_REGISTER *kr, R_REPLY *p);
void comfortzone_decoder_reply_w_generic(KNOWN_REGISTER *kr, W_REPLY *p);

void comfortzone_decoder_empty(KNOWN_REGISTER *kr, W_CMD *p);

#endif
