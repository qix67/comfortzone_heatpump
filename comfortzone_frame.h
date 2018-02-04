#ifndef _COMFORTZONE_FRAME_H
#define _COMFORTZONE_FRAME_H

#include <Arduino.h>

// This file contains the description of RS485 frames

#define FRAME_TYPE_01 0x01
#define FRAME_TYPE_02 0x02
#define FRAME_TYPE_02_CMD_p1 0x4302
#define FRAME_TYPE_02_CMD_p2 0x6302
#define FRAME_TYPE_02_REPLY 0x5202
#define FRAME_TYPE_15 0x15

// =====================================
// == basic frames
// =====================================

// r cmd is
// 65 6F DE 02 D3 5E 41 44 44 52 17 52 xx xx xx xx xx xx xx xx xx yy cc 41 44 44 52 07 8A
// xx is register num
// yy is ?? (wanted reply size ?)
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte unkown1[2];		// D3 5E
	byte addr1[4];			// 41 44 44 52 (=ADDR)
	byte packet_size;		// cz_size-6  = 0x17  (BCD?)
	byte cmd;				// 52 (=r)
	byte reg_num[9];
	byte wanted_reply_size;
	byte crc;
	byte addr2[4];			// 41 44 44 52 (=ADDR)
	byte unknown2[2];		// 07 8A
} R_CMD;

// w cmd is
// 65 6F DE 02 D3 5E 41 44 44 52 18 57 xx xx xx xx xx xx xx xx xx yy yy cc 41 44 44 52 07 8A
// xx is register num
// yy is reg value (little endian)
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte unkown1[2];		// D3 5E
	byte addr1[4];			// 41 44 44 52 (=ADDR)
	byte packet_size;		// cz_size-6  = 0x18  (BCD?)
	byte cmd;				// 57 (=W)
	byte reg_num[9];
	byte reg_value[2];
	byte crc;
	byte addr2[4];			// 41 44 44 52 (=ADDR)
	byte unknown2[2];		// 07 8A
} W_CMD;

// w cmd is
// 65 6F DE 02 D3 5E 41 44 44 52 17 57 xx xx xx xx xx xx xx xx xx yy yy cc 41 44 44 52 07 8A
// xx is register num
// yy is reg value (little endian)
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte unkown1[2];		// D3 5E
	byte addr1[4];			// 41 44 44 52 (=ADDR)
	byte packet_size;		// cz_size-6  = 0x17  (BCD?)
	byte cmd;				// 57 (=W)
	byte reg_num[9];
	byte reg_value;
	byte crc;
	byte addr2[4];			// 41 44 44 52 (=ADDR)
	byte unknown2[2];		// 07 8A
} W_SMALL_CMD;

// r reply is
// 65 6F DE 02 18 52 xx xx xx xx xx xx xx xx xx yy yy cc
// xx is register num
// yy is reg value (little endian)
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte packet_size;		// cz_size+6  = 0x18
	byte cmd;				// 52 (=r)
	byte reg_num[9];
	byte reg_value[2];
	byte crc;
} R_REPLY;

// r reply is
// 65 6F DE 02 17 52 xx xx xx xx xx xx xx xx xx yy cc
// xx is register num
// yy is reg value (little endian)
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte packet_size;		// cz_size+6  = 0x17
	byte cmd;				// 52 (=r)
	byte reg_num[9];
	byte reg_value;
	byte crc;
} R_SMALL_REPLY;

// w reply is
// 65 6F DE 02 17 77 xx xx xx xx xx xx xx xx xx yy cc
// xx is register num
// yy = status code: 00 = ok
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte packet_size;		// cz_size+6  = 0x17
	byte cmd;				// 77 (=w)
	byte reg_num[9];
	byte return_code;
	byte crc;
} W_REPLY;

// =====================================
// == status frames
// =====================================

typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[2];

	byte extra_hot_water;					// 0x00 = on, 0xFF = off
	byte hot_water_calculated_setting[2];			// °C, LSB, 2 bytes, * 10
	byte hot_water_hysteresis[2];		// °C, LSB, 2 bytes, * 10

	// le +3 correspond visiblement à 3 steps non affichés
#define STATUS_01_NB_HW_NORMAL_STEPS (12+3)
	byte hot_water_normal_steps[STATUS_01_NB_HW_NORMAL_STEPS][2];	// °C, LSB, 2 bytes, signed, * 10

	// le +3 correspond visiblement à 3 steps non affichés
#define STATUS_01_NB_HW_HIGH_STEPS (12+3)
	byte hot_water_high_steps[STATUS_01_NB_HW_HIGH_STEPS][2];	// °C, LSB, 2 bytes, signed, * 10

	// le +3 correspond visiblement à 3 steps non affichés
#define STATUS_01_NB_HW_EXTRA_STEPS (12+3)
	byte hot_water_extra_steps[STATUS_01_NB_HW_EXTRA_STEPS][2];	// °C, LSB, 2 bytes, signed, * 10

	byte unknown5a[1];

	byte hot_water_max_runtime;								// minute
	byte hot_water_pause_time;							// minute

	byte unknown5b[8][2];		// visiblement, ce sont 8 int16 signé

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

	byte pac_compressor_min_frequency[2];		// Hz, LSB, 2 bytes, * 10
	byte pac_compressor_max_frequency[2];		// Hz, LSB, 2 bytes, * 10
	byte unknown9a[1];

	byte crc;
} R_REPLY_STATUS_01;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
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

	byte unknown3[15];

	byte unknown3b[24];

	// Note: at least 24, not sure above
#define STATUS_02_NB_SENSORS 62
	byte sensors[STATUS_02_NB_SENSORS][2];	// bunch of sensors (TEx)

	byte crc;
} R_REPLY_STATUS_02;

typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_03;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
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
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
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

	byte hot_water_calculated_setting[2];				// °C, LSB, 2bytes
	byte unknown2[2];							// always 0x00 00 (previous field may be 4 bytes length)
	byte unknown_count_down[2];				// second, LSB, 2 bytes, * 10
	byte unknown2b[2];						// always 0x00 00 (previous field may be 4 bytes length)
	byte extra_hot_water;						// 0x00 = off, 0x0F = on
	byte unknown2c[1];						// always 0x03

	byte fan_speed;

	byte unknown3[8];

	byte condensing_temperature[2];					// °C, LSB, 2 bytes, * 10
	byte condensing_pressure[2];						// bar, LSB, 2 bytes, * 10
	byte crc;
} R_REPLY_STATUS_05;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
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

	byte unknown_count_down[2];				// = 00 00 si pac à l'arret, sinon, décompte à partir de~0x0383

	byte unknown1b[6];

	byte heatpump_defrost_delay[2];	// minute, 2 bytes, LSB, *10 (FAUX)

	byte unknown2[12];

	byte expansion_valve_calculated_setting[2];	// K, LSB, 2 bytes, * 10
	byte vanne_expansion_xxx[2];						// nombre négatif. parametre_calculé + xxx = difference de temp1
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
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[113];
	byte input_power_limit[2];	// W, 2 bytes, LSB

	byte unknown2[63];
	byte crc;
} R_REPLY_STATUS_07;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];

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
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
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
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
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
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[145];
	byte led_luminosity;
	byte unknown2[4];
	byte holiday_reduction[2];		// in day

	byte unknown3[26];
	byte crc;
} R_REPLY_STATUS_11;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
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
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0x69
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[83];
	byte crc;
} R_REPLY_STATUS_13;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0x50
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[44];

	byte language;
	
	byte unknownd[11];
	byte crc;
} R_REPLY_STATUS_14;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_15;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_16;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_17;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0x63
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[77];
	byte crc;
} R_REPLY_STATUS_18;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_19;

typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0x2C
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[22];
	byte crc;
} R_REPLY_STATUS_20;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_22;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
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
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_24;


typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte packet_format;	// 02
	byte packet_size;		// cz_size+6  = 0xC8
	byte cmd;				// 72 (=r)
	byte reg_num[9];
	
	byte unknown[178];
	byte crc;
} R_REPLY_STATUS_25;


#endif
