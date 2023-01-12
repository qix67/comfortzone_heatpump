#ifndef _COMFORTZONE_FRAME_H
#define _COMFORTZONE_FRAME_H

#include <Arduino.h>

// This file contains the description of RS485 frames

/*
 Standard packets have the following format:

 xx xx xx xx pp pp yy yy yy yy ss tt ..... crc

    xx is source address on 4 bytes
    pp is packet type ?: D3 5E = command, 07 8A = reply
    yy is destination address on 4 bytes
    ss is packet size (from xx to crc, all included)
    tt can be either (W: write command, w: write reply, R: read command, r: read reply)

*/
typedef struct __attribute__ ((packed)) cz_packet_header
{
	byte destination[4];
	byte destination_crc;		// crc-maxim of source
	byte comp1_destination_crc;	// crc-maxim of comp1's of byte of source 
	byte source[4];
	byte packet_size;		// packet size in byte
	byte cmd;				// 'W': write command, 'w': write reply, 'R': read command, 'r': read reply)
	byte reg_num[9];
} CZ_PACKET_HEADER;

// =====================================
// == basic frames
// =====================================

// r cmd is: cz_header yy cc
// - yy is ?? (wanted reply size ?)
// - cc = crc
typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	byte wanted_reply_size;
	byte crc;
} R_CMD;

// w cmd is: cz_header yy yy cc
// - yy is reg value (2 bytes, little endian)
// - cc = crc
typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	byte reg_value[2];
	byte crc;
} W_CMD;

// w cmd is: cz_header yy yy cc
// - yy is reg value (1 byte)
// - cc = crc
typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	byte reg_value;
	byte crc;
} W_SMALL_CMD;

// r reply is: cz_header yy yy cc
// - yy is reg value (2 bytes, little endian)
// - cc = crc
typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	byte reg_value[2];
	byte crc;
} R_REPLY;

// r reply is: cz_header yy cc
// - yy is reg value (1 byte)
// - cc = crc
typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	byte reg_value;
	byte crc;
} R_SMALL_REPLY;

// w reply is: cz_header yy cc
//  - yy = status code: 00 = ok
//  - cc = crc
typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	byte return_code;
	byte crc;
} W_REPLY;

// =====================================
// == status frames (protocol version 1.6)
// =====================================

typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[2];

	byte extra_hot_water;					// 0x00 = on, 0xFF = off
	byte hot_water_user_setting[2];			// °C, LSB, 2 bytes, * 10 (it is the temperature selected by user)
	byte hot_water_hysteresis[2];		// °C, LSB, 2 bytes, * 10

	//  +3 comes from 3 non displayed steps
#define STATUS_01_NB_HW_NORMAL_STEPS (12+3)
	byte hot_water_normal_steps[STATUS_01_NB_HW_NORMAL_STEPS][2];	// °C, LSB, 2 bytes, signed, * 10

	//  +3 comes from 3 non displayed steps
#define STATUS_01_NB_HW_HIGH_STEPS (12+3)
	byte hot_water_high_steps[STATUS_01_NB_HW_HIGH_STEPS][2];	// °C, LSB, 2 bytes, signed, * 10

	//  +3 comes from 3 non displayed steps
#define STATUS_01_NB_HW_EXTRA_STEPS (12+3)
	byte hot_water_extra_steps[STATUS_01_NB_HW_EXTRA_STEPS][2];	// °C, LSB, 2 bytes, signed, * 10

	byte unknown5a[1];

	byte hot_water_max_runtime;								// minute
	byte hot_water_pause_time;							// minute

	byte unknown5b[8][2];		// looks like 8 signed int16

	byte hot_water_compressor_min_frequency[2];		// Hz, LSB, 2bytes, * 10
	byte hot_water_compressor_max_frequency[2];		// Hz, LSB, 2bytes, * 10

	byte unknown6[4];

	byte hot_water_extra_setting[2];					// °C, LSB, 2bytes, * 10
	byte hot_water_extra_time[2];				// minute, LSB, 2bytes

	byte unknown7[7];
	
	byte normal_fan_speed[2];						// %, LSB, 2 bytes, * 10
	byte reduce_fan_speed[2];						// %, LSB, 2 bytes, * 10, signed
	byte fan_boost_increase[2];					// %, LSB, 2 bytes, * 10

	byte unknown8[16];

	byte supply_fan_t12_adjust[2];				// %, LSB, 2 bytes, * 10

	byte unknown8a[6];

	byte fan_time_to_filter_change[2];	// days, LSB, 2 bytes

	byte unknown9[6];

	byte heating_compressor_min_frequency[2];		// Hz, LSB, 2 bytes, * 10
	byte heating_compressor_max_frequency[2];		// Hz, LSB, 2 bytes, * 10
	byte unknown9a[1];

	byte crc;
} R_REPLY_STATUS_01;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[4];

	byte hour1;
	byte minute1;
	byte second1;
	byte day;
	byte month;
	byte year;
	byte day_of_week;		// 1 = monday, 7 = sunday

	byte unknown2;			// maybe daylight saving?

	byte hour2;
	byte minute2;
	byte second2;

	byte unknown3[10];

	byte general_status[5];
								// 0x88 0A 00 00 10    // heatpump stopped, no defrost, no heating, no hot water, no add
								// 0x88 0E 00 00 10    // heatpump stopped, no defrost, no heating, no hot water, no add
								// 0x8B 0E 00 00 10    // heatpump stopped, no defrost, no heating, no hot water, no add
								// 0x88 08 00 00 10    // heatpump stopped, no defrost, no heating, no hot water, no add
								// 0x88 2E 00 00 10    // heatpump running, no defrost, no heating,    hot water, no add
								// 0x8B AE 00 00 10    // heatpump running, no defrost, no heating,    hot water, no add
								// 0xA8 1A 00 00 14    // heatpump stopped,    defrost,    heating or hot water,     add
								// 0xA8 2A 00 00 10    // heatpump running, no defrost,    heating, no hot water,    add
								// 0xA8 3A 00 00 10    // heatpump running, no defrost,    heating, no hot water,    add
								// 0x88 2A 00 00 10    // heatpump running, no defrost,    heating, no hot water, no add
								// 0x8B 2E 00 00 10    // heatpump running, no defrost, no heating,    hot water, no add
								// 0x88 AA 00 00 10    // heatpump running, no defrost,    heating, no hot water, no add
								// 0x80 08 00 00 10    // heatpump stopped, no defrost, no heating, no hot water, no add
								// 0xA0 08 00 00 10    // heatpump stopped, no defrost, no heating, no hot water, no add
								// 0x80 2A 00 00 10    // heatpump running, no defrost,    heating, no hot water, no add

								// byte 0:  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
								//           =1 ----------------------------------> always
								//               =0 ------------------------------> always
								//                   =x --------------------------> 1 = add energy on, 0 = add energy off
								//                       =0 ----------------------> always
								//                           =x ------------------> ?
								//                               =0 --------------> always
								//                                    x---y-------> xy = 11 (hot water mode), 00 (heating mode, default when stopped)
	
								// byte 1:  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
								//           =0 ----------------------------------> always
								//               =0 ------------------------------> always
								//                    x---y-----------------------> xy = 10 (heatpump is running), 11 (heatpump is powering down), 01 (heatpump stopped), 00 ( heatpump stopped?)
								//                           =1 ------------------> always
								//                               =x---y-----------> xy = 11 (hot water mode), 00 (idle mode), 01 (heating mode, default mode)
								//                                       =0 ------> always

								// byte 2:  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
								//            always 0

								// byte 3:  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
								//            always 0 or rarely changing

								// byte 4:  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
								//           =0 ----------------------------------> always
								//               =0 ------------------------------> always
								//                   =0 --------------------------> always
								//                       =0 ----------------------> always
								//                           =0 ------------------> always
								//                               =x --------------> x = 1 (defrost in progress), 0 (no defrost in progress)
								//                                   =0 ----------> always
								//                                       =0 ------> always

	byte unknown3b[3];

	byte pending_alarm[2];		// bitmask of current alarms
										// byte 0: | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
										//                                  =1 -----> filter replacement alarm

	byte unknown3c[6];

	byte acknowledged_alarm[2];
										// bitmask of acknowledged alarms
										// before the corresponding bits are cleared from pending_alarm[], is it set in acknowledged_alarm[] using
										// an access to register 0x01, 0x02, 0x03, 0x04, 0x0B, 0x07, 0x81, 0x29, 0x00. Register value will be loaded in this array

	byte unknown3d[11];

	// Note: at least 24, not sure above
#define STATUS_02_NB_SENSORS 62
	byte sensors[STATUS_02_NB_SENSORS][2];	// bunch of sensors (TEx)

	byte crc;
} R_REPLY_STATUS_02;

typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_03;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[170];
	
	byte chauffage_puissance_consommee1[2];	// W, LSB, 2 bytes

	byte unknown8[2];
	
	byte chauffage_puissance_consommee2[2];	// W, LSB, 2 bytes

	byte hot_water_production;						// 0x00 = off, 0x78 or 0x77 (rare) = production in progress

	byte unknown9[1];	

	// 1 time, hot_water_production + unknown9 was 0x44 0x44. At the same time, unknown8 was 0x02 0xD3 else, was always 00 00
	byte crc;
} R_REPLY_STATUS_04;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte hot_water_production;						// 0x00 = off, 0x78 or 0x77 (rare) = production in progress
	byte unknown[1];

	byte heating_calculated_setting[2];	// °C, LSB, 2 bytes, * 10

	byte unknown0[30];

#define STATUS_05_TE3_INDOOR_TEMP_HISTORY_NB 21
	byte te3_indoor_temp_history[STATUS_05_TE3_INDOOR_TEMP_HISTORY_NB][2];				// °C, LSB, 2 bytes, signed, TE3 (indoor temp.) history, new values first, 1 value/10s

#define STATUS_05_TE2_RETURN_WATER_HISTORY_NB 21
	byte te2_return_water_history[STATUS_05_TE2_RETURN_WATER_HISTORY_NB][2];			// °C, LSB, 2 bytes, signed, TE2 (return water) history, new values first, 1 value/10s

	byte room_heating_in_progress[2];					// LSB, 2bytes, 0x012C = room heating off, 0x0000 = room heating on

	byte unknown1[35];							// always seems to be 84 03 84 03 00 00 22 FC 22 FC 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF 19 FC FF FF F1 D8 F1 D8 FF
														// rarely starts by   83 03 83 03

	byte hot_water_calculated_setting[2];				// °C, LSB, 2bytes (it is the current hot water the heatpump tries to reach)
																	// it is either the user one or a different one if extra hot water is enabled
	byte unknown2[2];							// always 0x00 00 (previous field may be 4 bytes length)
	byte unknown_count_down[2];				// second, LSB, 2 bytes, * 10
	byte unknown2b[2];						// always 0x00 00 (previous field may be 4 bytes length)
	byte extra_hot_water;						// 0x00 = off, 0x0F = on
	byte unknown2c[1];						// always 0x03

	byte fan_speed;							// 1 = slow, 2 = normal, 3 = fast
	byte fan_speed_duty[2];					// %, LSB, 2 bytes, * 10

	byte unknown3[6];

	byte condensing_temperature[2];					// °C, LSB, 2 bytes, * 10
	byte condensing_pressure[2];						// bar, LSB, 2 bytes, * 10
	byte crc;
} R_REPLY_STATUS_05;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte evaporator_pressure[2];				// bar, LSB, 2 bytes, * 10

	byte pressure_ratio[2];				// LSB, 2 bytes, * 10

	byte unknown0a[2];		// 2bytes, LSB, either a pressure or a temperature. Goes down when PAC is off but increase when PAC is started

	byte heatpump_compressor_max_frequency1[2];

	byte hot_water_active_max_frequency[2];	// Hz, LSB, 2bytes, * 10. forced to 0hz when hot water production is off (may be target frequency)

	byte heatpump_active_max_frequency1[2];		// Hz, LSB, 2bytes, * 10. During defrost, forced to 0Hz else set to heating compressor max frequency

	byte unknown0c[10];

	byte heatpump_active_max_frequency2[2];		// Hz, LSB, 2bytes, * 10. During defrost, forced to 0Hz else set to heating compressor max frequency
	byte heatpump_active_max_frequency3[2];		// Hz, LSB, 2bytes, * 10. During defrost, forced to 0Hz else set to heating compressor max frequency
	byte heatpump_current_compressor_frequency[2];	// Hz, LSB, 2bytes, * 10.
	byte chauffage_compressor_max_frequency3[2];

	byte unknown0d[2];

	byte heating_compressor_min_frequency[2];		// Hz, LSB, 2 bytes, * 10. Compressor min frequency when running in room heating mode
	byte heating_compressor_max_frequency[2];		// Hz, LSB, 2 bytes, * 10. Compressor max frequency when running in room heating mode
	
	byte unknown0[30];

	byte heatpump_current_compressor_power[2];		// W, LSB, 2 bytes
	byte heatpump_current_add_power[2];			// W, LSB, 2 bytes
	byte heatpump_current_total_power1[2];		// W, LSB, 2 bytes
	byte heatpump_current_total_power2[2];		// W, LSB, 2 bytes
	byte heatpump_compressor_input_power[2];					// W, LSB, 2 bytes

	byte unknown1a[4];

	byte unknown_count_down[2];				// = 00 00 when heatpump is stopped else countdown from approximately 0x0383

	byte unknown1b[6];

	byte heatpump_defrost_delay[2];	// minute, 2 bytes, LSB, *10 (erroneous)

	byte unknown2[12];

	byte expansion_valve_calculated_setting[2];	// K, LSB, 2 bytes, * 10
	byte vanne_expansion_xxx[2];						// negative number, expansion_valve_calculated_setting + xxx = temperature difference
	byte expansion_valve_temperature_difference1[2];		// K, LSB, 2 bytes, * 10
	byte expansion_valve_temperature_difference2[2];		// K, LSB, 2 bytes, * 10

	byte unknown2a[30];

	byte expansion_valve_valve_position1[2];		// %, LSB, 2 bytes, * 10
	byte expansion_valve_valve_position2[2];		// %, LSB, 2 bytes, * 10

	byte unknown2b[34];

	byte crc;
} R_REPLY_STATUS_06;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[113];
	byte input_power_limit[2];	// W, 2 bytes, LSB

	byte unknown2[63];
	byte crc;
} R_REPLY_STATUS_07;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;

	byte bcd_second;
	byte bcd_minute;
	byte bcd_hour;

	byte unknown0;		// padding ?

	byte bcd_day;
	byte bcd_month;
	byte bcd_year;

	byte unknown0a[5];	// byte 0: padding ?
	byte unknown[32];

	byte compressor_energy[4];		// kWh, 4 bytes, LSB, * 100
	byte add_energy[4];				// kWh, 4 bytes, LSB, * 100
	byte hot_water_energy[4];		// kWh, 4 bytes, LSB, * 100
	byte compressor_runtime[4];	// minutes, 4 bytes LSB
	byte total_runtime[4];			// minutes, 4 bytes LSB
	byte crc;
} R_REPLY_STATUS_08;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[14];
	byte hotwater_priority[2];

	byte unknown2[107];
#define STATUS_09_NB_AJUSTMENT 8
	byte hardware_settings_adjustments_teX_adjust[STATUS_09_NB_AJUSTMENT];

	byte unknown6[47];

	byte crc;
} R_REPLY_STATUS_09;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[98];
	byte holiday_temperature_reduction[2];			// °C, LSB, 2 bytes, * 10
	byte holiday_minimal_room_temperature[2];		// °C, LSB, 2 bytes, * 10
	byte cw_minimal_temperature[2];				// °C, LSB, 2 bytes, * 10
	byte cw_maximal_temperature[2];				// °C, LSB, 2 bytes, * 10
	byte cw_overheat_hysteresis[2];				// °C, LSB, 2 bytes, * 10

	byte unknown2a[4];

	byte cw_return_maximal_temperature[2];	// °C, LSB, 2 bytes, * 10

	byte unknown2b[10];

	byte cw_pump[2];									// %, LSB, 2 bytes, * 10

	byte unknown2[25];
#define STATUS_10_STEP_WISE_HEATING_NB 8

	byte step_wise_heating_temperature[STATUS_10_STEP_WISE_HEATING_NB][2];		// °C, LSB, 2 bytes, * 10
	byte step_wise_heating_days[STATUS_10_STEP_WISE_HEATING_NB];				// day, 1 byte, * 10
	
	byte unknown3[5];
	byte crc;
} R_REPLY_STATUS_10;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[145];
	byte led_luminosity;
	byte unknown2[4];
	byte holiday_reduction[2];		// in day

	byte unknown3[26];
	byte crc;
} R_REPLY_STATUS_11;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[9];
	
	byte heatpump_compressor_blocked_frequency1[2];			// Hz, 2 bytes, LSB, * 10
	byte heatpump_compressor_blocked_frequency2[2];			// Hz, 2 bytes, LSB, * 10
	byte heatpump_compressor_blocked_frequency3[2];			// Hz, 2 bytes, LSB, * 10

	byte unknown3[107];

	byte heatpump_defrost_delay[2];	// min, 2 bytes, LSB, * 10;

	byte unknown8[54];
	byte crc;
} R_REPLY_STATUS_12;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[83];
	byte crc;
} R_REPLY_STATUS_13;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[44];

	byte language;
	
	byte unknownd[11];
	byte crc;
} R_REPLY_STATUS_14;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_15;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_16;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_17;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[77];
	byte crc;
} R_REPLY_STATUS_18;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_19;

typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[22];
	byte crc;
} R_REPLY_STATUS_20;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_22;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
#define STATUS_23_UNKNOWN1_NB 16
	byte unknown1[STATUS_23_UNKNOWN1_NB][2];

	byte unknown[28];

#define STATUS_23_UNKNOWN2_NB 16
	byte unknown2[STATUS_23_UNKNOWN2_NB][2];

	byte unknown3[86];
	byte crc;
} R_REPLY_STATUS_23;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_24;


typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_25;

// =====================================
// == status frames (protocol version 1.8)
// =====================================

typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[2];
	byte unknown1[2];
	byte unknown2[4];
	byte unknown3[2];
	byte unknown4[2];
	byte unknown5[2];
	byte unknown6[2];
	byte unknown7[2];
	byte unknown8[2];
	byte unknown9[2];
	byte unknown10[2];
	byte unknown11[2];
	byte unknown12[2];
	byte unknown13[2];
	byte unknown14[2];
	byte unknown15[2];
	byte unknown16[48];
	byte crc;
} R_REPLY_STATUS_V180_STATUS_x68;

typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[66];
	byte crc;
} R_REPLY_STATUS_V180_STATUS_x58;

typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
#define STATUS_V180_x40_NB_TEMP 21
	byte temp[STATUS_V180_x40_NB_TEMP][2];				// ? °C, LSB, 2 bytes, signed
	byte crc;
} R_REPLY_STATUS_V180_STATUS_x40;

typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[16];
	byte crc;
} R_REPLY_STATUS_V180_STATUS_x26;

typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
#define STATUS_V180_x8d_NB_TEMP 9
	byte temp[STATUS_V180_x8d_NB_TEMP][2];				// ? °C, LSB, 2 bytes, signed
	byte unknown[3];
#define STATUS_V180_x8d_NB_TEMP1 15
	byte temp1[STATUS_V180_x8d_NB_TEMP1][2];				// ? °C, LSB, 2 bytes, signed
	byte unknown1[3];
	byte unknown2[65];
	byte crc;
} R_REPLY_STATUS_V180_STATUS_x8d;

typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown1[4];
	byte total_runtime[4];
	byte compressor_runtime[4];
	byte compressor_energy[4];
	byte add_energy[4];
	byte hot_water_energy[4];
	byte crc;
} R_REPLY_STATUS_V180_STATUS_x2e;

typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte unknown[139];
	byte crc;
} R_REPLY_STATUS_V180_STATUS_xa1;

typedef struct __attribute__ ((packed))
{
	CZ_PACKET_HEADER cz_head;
	
	byte heatpump_status[2];
	byte crc;
} R_REPLY_STATUS_V180_02;


#endif
