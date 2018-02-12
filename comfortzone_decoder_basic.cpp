#include "comfortzone_config.h"
#include "comfortzone_frame.h"
#include "comfortzone_decoder_basic.h"
#include "comfortzone_tools.h"

// this file contains basic decoding function for commands and replies.
// Unless debug mode is set, nothing is done here

void czdec_cmd_r_generic(KNOWN_REGISTER *kr, R_CMD *p)
{
#ifdef DEBUG
	NPRINT("Wanted reply size: ");
	if(p->wanted_reply_size < 0x10)
		NPRINT("0");
	NPRINTLN(p->wanted_reply_size, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_generic_2byte(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	reg_v = p->reg_value[0];

	if(reg_v < 0x10)
		NPRINT("0");
	NPRINT(reg_v, HEX);

	NPRINT(" ");

	reg_v = p->reg_value[1];

	if(reg_v < 0x10)
		NPRINT("0");
	NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
#endif
}

void czdec_cmd_w_temp(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	reg_v = get_uint16(p->reg_value);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT(reg_v_f);
	NPRINTLN("°C");

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_temp_1byte(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	reg_v = ((W_SMALL_CMD*)p)->reg_value;

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT(reg_v_f);
	NPRINTLN("°C");

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_freq(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	reg_v = get_uint16(p->reg_value);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_time(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	reg_v = get_uint16(p->reg_value);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT(reg_v_f);
	NPRINTLN("min");

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_percentage(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	reg_v = get_int16(p->reg_value);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT(reg_v_f);
	NPRINTLN("%");

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_extra_hot_water(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = get_uint16(p->reg_value);

	if(reg_v == 0x0001)
		NPRINTLN("on");
	else if(reg_v == 0xFFFE)		// -2 (???)
		NPRINTLN("off");
	else
		NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_daylight_saving(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = get_uint16(p->reg_value);

	if(reg_v == 0xFFBF)		// == ~0x0040
		NPRINTLN("on");
	else if(reg_v == 0x0040)
		NPRINTLN("off");
	else
		NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_sanitary_priority(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = get_uint16(p->reg_value);

	if(reg_v == 0x0001)
		NPRINTLN("low");
	else if(reg_v == 0x0002)
		NPRINTLN("normal");
	else if(reg_v == 0x0003)
		NPRINTLN("high");
	else
		NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_day_delay(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = get_uint16(p->reg_value);

	if(reg_v == 0)
		NPRINTLN("off");
	else
	{
		NPRINT(reg_v);
		NPRINTLN(" day(s)");
	}

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_fan_speed(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = ((W_SMALL_CMD*)p)->reg_value;

	if(reg_v == 0x01)
		NPRINTLN("low");
	else if(reg_v == 0x02)
		NPRINTLN("normal");
	else if(reg_v == 0x03)
		NPRINTLN("high");
	else
		NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_language(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = ((W_SMALL_CMD*)p)->reg_value;

	if(reg_v == 0x01)
		NPRINTLN("English");
	else if(reg_v == 0x04)
		NPRINTLN("Français");
	else
		NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_cmd_w_digit(KNOWN_REGISTER *kr, W_CMD *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = ((W_SMALL_CMD*)p)->reg_value;

	NPRINT("Value: ");
	NPRINT(reg_v);
	NPRINT(" = 0x");

	if(reg_v < 0x10)
		NPRINT("0");
	NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_generic_2byte(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = p->reg_value[0];

	if(reg_v < 0x10)
		NPRINT("0");
	NPRINT(reg_v, HEX);

	NPRINT(" ");

	reg_v = p->reg_value[1];

	if(reg_v < 0x10)
		NPRINT("0");
	NPRINTLN(reg_v, HEX);


	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_temp(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	reg_v = get_uint16(p->reg_value);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT(reg_v_f);
	NPRINTLN("°C");

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_temp_1byte(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	reg_v = ((R_SMALL_REPLY*)p)->reg_value;

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT(reg_v_f);
	NPRINTLN("°C");

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_freq(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	reg_v = get_uint16(p->reg_value);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_time(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	reg_v = get_uint16(p->reg_value);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT(reg_v_f);
	NPRINTLN("min");

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_percentage(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	reg_v = get_int16(p->reg_value);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT(reg_v_f);
	NPRINTLN("%");

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_extra_hot_water(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = get_uint16(p->reg_value);

	if(reg_v == 0x0C8B)
		NPRINTLN("on");
	else if(reg_v == 0x0C8A)		// -1 in complement 1's
		NPRINTLN("off");
	else
		NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_daylight_saving(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	// not tested
	int reg_v;

	reg_v = get_uint16(p->reg_value);

	if(reg_v == 0xFFBF)		// == ~0x0040
		NPRINTLN("on");
	else if(reg_v == 0x0040)
		NPRINTLN("off");
	else
		NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_sanitary_priority(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = get_uint16(p->reg_value);

	if(reg_v == 0x4151)
		NPRINTLN("low");
	else if(reg_v == 0x4152)
		NPRINTLN("normal");
	else if(reg_v == 0x4153)
		NPRINTLN("high");
	else
		NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_day_delay(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = get_uint16(p->reg_value);

	if(reg_v == 0)
		NPRINTLN("off");
	else
	{
		NPRINT(reg_v);
		NPRINTLN(" day(s)");
	}

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_fan_speed(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = ((R_SMALL_REPLY*)p)->reg_value;

	if(reg_v == 0x01)
		NPRINTLN("low");
	else if(reg_v == 0x02)
		NPRINTLN("normal");
	else if(reg_v == 0x03)
		NPRINTLN("high");
	else
		NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_r_language(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = ((R_SMALL_REPLY*)p)->reg_value;

	if(reg_v == 0x01)
		NPRINTLN("English");
	else if(reg_v == 0x04)
		NPRINTLN("Français");
	else
		NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}


void czdec_reply_r_digit(KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	int reg_v;

	reg_v = ((R_SMALL_REPLY*)p)->reg_value;

	if(reg_v < 0x10)
		NPRINT("0");
	NPRINTLN(reg_v, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_reply_w_generic(KNOWN_REGISTER *kr, W_REPLY *p)
{
#ifdef DEBUG
	NPRINT("Value: ");

	NPRINT(p->return_code);
	NPRINT(" = 0x");

	if(p->return_code < 0x10)
		NPRINT("0");
	NPRINTLN(p->return_code, HEX);

	NPRINT("crc: ");
	if(p->crc < 0x10)
		NPRINT("0");
	NPRINTLN(p->crc, HEX);
	return;
#endif
}

void czdec_empty(KNOWN_REGISTER *kr, W_CMD *p)
{
}

