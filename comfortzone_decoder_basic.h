#ifndef _COMFORTZONE_DECODER_BASIC_H
#define _COMFORTZONE_DECODER_BASIC_H

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

#endif
