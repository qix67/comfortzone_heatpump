#include "comfortzone_config.h"
#include "comfortzone_frame.h"
#include "comfortzone_decoder.h"
#include "comfortzone_tools.h"
#include "comfortzone_status.h"

#ifdef DEBUG
static void dump_unknown(const char *prefix, byte *start, int length)
{
	NPRINT(prefix);

	NPRINT(": ");
	while(length > 0)
	{
		if(*start < 0x10)
			NPRINT("0");
		NPRINT(*start, HEX);

		NPRINT(" ");

		start++;
		length--;
	}

	NPRINTLN("");
}
#endif

void czdec::reply_r_status_01(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
	R_REPLY_STATUS_01 *q = (R_REPLY_STATUS_01 *)p;

	czhp->comfortzone_status.fan_time_to_filter_change = get_uint16(q->fan_time_to_filter_change);

	czhp->comfortzone_status.hot_water_setting = get_uint16(q->hot_water_user_setting);
#ifdef DEBUG
	int reg_v;
	float reg_v_f;
	int i;

	// ===
	dump_unknown("unknown_s01", q->unknown, sizeof(q->unknown));

	// ===
	reg_v = q->extra_hot_water;

	NPRINT("Extra hot water: ");
	if(reg_v == 0xFF)
		NPRINT("off");
	else
		NPRINT("on - ");			// 0xFF = off mais on = 0x02 ou autre chose
		NPRINT(reg_v, HEX);
	NPRINTLN("");

	// ===
	reg_v = get_uint16(q->hot_water_user_setting);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Hot water User setting: ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");

	// ===
	reg_v = get_uint16(q->hot_water_hysteresis);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Hot water - hysteresis: ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");

	// ===
	for(i = 0; i < STATUS_01_NB_HW_NORMAL_STEPS; i++)
	{
		reg_v = get_int16(q->hot_water_normal_steps[i]);

		reg_v_f = reg_v;
		reg_v_f /= 10.0;

		NPRINT("Hot water - Normal step ");
		NPRINT(i);
		NPRINT(": ");
		NPRINT(reg_v_f);
		NPRINTLN("°C");
	}

	// ===
	for(i = 0; i < STATUS_01_NB_HW_HIGH_STEPS; i++)
	{
		reg_v = get_int16(q->hot_water_high_steps[i]);

		reg_v_f = reg_v;
		reg_v_f /= 10.0;

		NPRINT("Hot water - High step ");
		NPRINT(i);
		NPRINT(": ");
		NPRINT(reg_v_f);
		NPRINTLN("°C");
	}
	
	// ===
	for(i = 0; i < STATUS_01_NB_HW_EXTRA_STEPS; i++)
	{
		reg_v = get_int16(q->hot_water_extra_steps[i]);

		reg_v_f = reg_v;
		reg_v_f /= 10.0;

		NPRINT("Hot water - Additional step ");
		NPRINT(i);
		NPRINT(": ");
		NPRINT(reg_v_f);
		NPRINTLN("°C");
	}
	
	// ===
	dump_unknown("unknown_s01_5a", q->unknown5a, sizeof(q->unknown5a));

	// ===
	reg_v = q->hot_water_max_runtime;

	NPRINT("Hot water max runtime: ");
	NPRINT(reg_v);
	NPRINTLN("min");

	// ===
	reg_v = q->hot_water_pause_time;

	NPRINT("Hot water pause duration: ");
	NPRINT(reg_v);
	NPRINTLN("min");
	
	// ===
	for(i = 0; i < 8; i++)
	{
		reg_v = get_int16(q->unknown5b[i]);

		reg_v_f = reg_v;

		NPRINT("unknown_s01_5b[");
		NPRINT(i);
		NPRINT("]: ");
		NPRINT(reg_v_f);
		NPRINTLN("?");
	}


	// ===
	reg_v = get_uint16(q->hot_water_compressor_min_frequency);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Hot water - compressor min freq.: ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	reg_v = get_uint16(q->hot_water_compressor_max_frequency);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Hot water - compressor max freq.: ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");
	
	// ===
	dump_unknown("unknown_s01_6", q->unknown6, sizeof(q->unknown6));

	// ===
	reg_v = get_uint16(q->hot_water_extra_setting);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Hot water - extra setting: ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");

	// ===
	reg_v = get_uint16(q->hot_water_extra_time);

	NPRINT("Hot water - extra time: ");
	NPRINT(reg_v);
	NPRINTLN("min");

	// ===
	dump_unknown("unknown_s01_7", q->unknown7, sizeof(q->unknown7));

	// ===
	reg_v = get_uint16(q->normal_fan_speed);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Normal fan speed: ");
	NPRINT(reg_v_f);
	NPRINTLN("%");

	// ===
	reg_v = get_int16(q->reduce_fan_speed);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Reduce fan speed: ");
	NPRINT(reg_v_f);
	NPRINTLN("%");

	// ===
	reg_v = get_int16(q->fan_boost_increase);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Fan boost increase: ");
	NPRINT(reg_v_f);
	NPRINTLN("%");

	// ===
	dump_unknown("unknown_s01_8", q->unknown8, sizeof(q->unknown8));

	// ===
	reg_v = get_int16(q->supply_fan_t12_adjust);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Supply fan T12 adjust: ");
	NPRINT(reg_v_f);
	NPRINTLN("%");

	// ===
	dump_unknown("unknown_s01_8a", q->unknown8a, sizeof(q->unknown8a));

	// ===
	reg_v = get_uint16(q->fan_time_to_filter_change);

	NPRINT("Fan - Time to filter change: ");
	NPRINT(reg_v);
	NPRINTLN("d");

	// ===
	dump_unknown("unknown_s01_9", q->unknown9, sizeof(q->unknown9));

	// ===
	reg_v = get_uint16(q->heating_compressor_min_frequency);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heating compressor min freq.: ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	reg_v = get_uint16(q->heating_compressor_max_frequency);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heating compressor max freq.: ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	dump_unknown("unknown_s01_9a", q->unknown9a, sizeof(q->unknown9a));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_02(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
	R_REPLY_STATUS_02 *q = (R_REPLY_STATUS_02 *)p;
	uint16_t active_alarm;

	czhp->comfortzone_status.sensors_te1_flow_water = get_int16(q->sensors[0]);
	czhp->comfortzone_status.sensors_te2_return_water = get_int16(q->sensors[1]);
	czhp->comfortzone_status.sensors_te3_indoor_temp= get_int16(q->sensors[2]);
	czhp->comfortzone_status.sensors_te4_hot_gas_temp = get_int16(q->sensors[3]);
	czhp->comfortzone_status.sensors_te5_exchanger_out = get_int16(q->sensors[4]);
	czhp->comfortzone_status.sensors_te6_evaporator_in = get_int16(q->sensors[5]);
	czhp->comfortzone_status.sensors_te7_exhaust_air = get_int16(q->sensors[6]);
	czhp->comfortzone_status.sensors_te24_hot_water_temp = get_int16(q->sensors[23]);

	czhp->comfortzone_status.additional_power_enabled = (q->general_status[0] & 0x20) ? true : false;
	czhp->comfortzone_status.defrost_enabled = (q->general_status[4] & 0x04) ? true : false;

	switch((q->general_status[1]>>4) & 0x3)
	{
		case 0:	czhp->comfortzone_status.compressor_activity = CZCMP_UNKNOWN;
					break;

		case 1:	czhp->comfortzone_status.compressor_activity = CZCMP_STOPPED;
					break;

		case 2:	czhp->comfortzone_status.compressor_activity = CZCMP_RUNNING;
					break;

		case 3:	czhp->comfortzone_status.compressor_activity = CZCMP_STOPPING;
					break;
	}

	switch((q->general_status[1]>>1) & 0x3)
	{
		case 0:	czhp->comfortzone_status.mode = CZMD_IDLE;
					break;

		case 1:	czhp->comfortzone_status.mode = CZMD_ROOM_HEATING;
					break;

		case 2:	czhp->comfortzone_status.mode = CZMD_UNKNOWN;
					break;

		case 3:	czhp->comfortzone_status.mode = CZMD_HOT_WATER;
					break;
	}

	active_alarm = get_uint16(q->pending_alarm) ^ get_uint16(q->acknowledged_alarm);

	czhp->comfortzone_status.filter_alarm = (active_alarm & 0x0002) ? true : false ;

	czhp->comfortzone_status.hour = q->hour1;
	czhp->comfortzone_status.minute = q->minute1;
	czhp->comfortzone_status.second = q->second1;

	czhp->comfortzone_status.day = q->day;
	czhp->comfortzone_status.month = q->month;
	czhp->comfortzone_status.year = 2000 + q->year;
	czhp->comfortzone_status.day_of_week = q->day_of_week;

#ifdef DEBUG
	int reg_v;
	float reg_v_f;
	int i;

	static const char *sensor_names[STATUS_02_NB_SENSORS] =
						{	"TE1 Flow water",
							"TE2 Return water",
							"TE3 Indoor temp. = Heating - Room temperature",
							"TE4 Hot gas temp.",
							"TE5 Exchanger out",
							"TE6 Evaporator in",
							"TE7 Exhaust air",
							"TE8 ?",			// no value
							"TE9 ?",			// no value
							"TE10 ?",
							"TE11 ?",		// no value
							"TE12 ?",		// no value
							"TE13 ?",		// no value
							"TE14 ?",		// no value
							"TE15 ?",		// no value
							"TE16 ?",		// no value
							"TE17 ?",		// no value
							"TE18 ?",		// no value
							"TE19 ?",		// no value
							"TE20 ?",		// no value
							"TE21 ?",		// no value
							"TE22 ?",		// no value
							"TE23 ?",		// no value
							"TE24 Hot water = Hot water Measured temperature",	
							"TE25 ?",
							"TE26 ?",
							"TE27 ?",
							"TE28 ?",
							"TE29 ?",		// always 0.0°C
							"TE30 ?",		// always 100.0°C
							"TE31 ?",		// always 100.0°C
							"TE32 ?",		// always 0.0°C
							"TE33 ?",		// seems to always be close to TE25
							"TE34 ?",
							"TE35 ?",
							"TE36 ?",
							"TE37 ?",		// always 0.0°C
							"TE38 ?",		// always 0.0°C
							"TE39 ?",		// always 0.0°C
							"TE40 ?",
							"TE41 ?",		// seems to always be close to TE1
							"TE42 ?",		// seems to always be close to TE2
							"TE43 ?",		// seems to always be close to TE3
							"TE44 ?",		// seems to always be close to TE4
							"TE45 ?",		// seems to always be close to TE5
							"TE46 ?",		// seems to always be close to TE6
							"TE47 ?",
							"TE48 ?",		// seems to always be close to TE7
							"TE49 ?",		// no value
							"TE50 ?",		// no value
							"TE51 ?",
							"TE52 ?",		// no value
							"TE53 ?",		// no value
							"TE54 ?",		// no value
							"TE55 ?",		// no value
							"TE56 ?",		// no value
							"TE57 ?",		// no value
							"TE58 ?",		// no value
							"TE59 ?",		// no value
							"TE60 ?",		// no value
							"TE61 ?",		// no value
							"TE62 ?"			// no value
						};

	// ===
	dump_unknown("unknown_s02", q->unknown, sizeof(q->unknown));

	// ===
	NPRINT("Time1: ");
	NPRINT(q->hour1);
	NPRINT(":");
	NPRINT(q->minute1);
	NPRINT(":");
	NPRINT(q->second1);
	NPRINTLN();

	// ===
	NPRINT("Day: ");
	NPRINT(q->day);
	NPRINT("/");
	NPRINT(q->month);
	NPRINT("/");
	NPRINT(q->year + 2000);
	NPRINTLN();

	// ===
	NPRINT("Day of week: ");
	NPRINTLN(q->day_of_week);

	// ===
	NPRINT("Time2: ");
	NPRINT(q->hour2);
	NPRINT(":");
	NPRINT(q->minute2);
	NPRINT(":");
	NPRINT(q->second2);
	NPRINTLN();

	// ===
	dump_unknown("unknown_s02_3", q->unknown3, sizeof(q->unknown3));

	// ===
	dump_unknown("unknown_general_status", q->general_status, sizeof(q->general_status));

	// ===
	NPRINT("Add energy: ");
	NPRINTLN( (q->general_status[0] & 0x20) ? "on" : "off");

	NPRINT("Mode (1): ");
	switch(q->general_status[0] & 0x3)
	{
		case 0:	NPRINTLN("Heating");
					break;
		case 1:	NPRINTLN("1?");
					break;
		case 2:	NPRINTLN("2?");
					break;
		case 3:	NPRINTLN("Hot water");
					break;
	}

	NPRINT("Heatpump activity: ");
	switch((q->general_status[1]>>4) & 0x3)
	{
		case 0:	NPRINTLN("Stopped ?");
					break;
		case 1:	NPRINTLN("Stopped");
					break;
		case 2:	NPRINTLN("Running");
					break;
		case 3:	NPRINTLN("Stopping");
					break;
	}

	NPRINT("Mode (2): ");
	switch((q->general_status[1]>>1) & 0x3)
	{
		case 0:	NPRINTLN("Idle");
					break;
		case 1:	NPRINTLN("Heating");
					break;
		case 2:	NPRINTLN("2?");
					break;
		case 3:	NPRINTLN("Hot water");
					break;
	}

	NPRINT("Defrost: ");
	NPRINTLN( (q->general_status[4] & 0x04) ? "on" : "off");

	// ===
	dump_unknown("unknown_s02_3b", q->unknown3b, sizeof(q->unknown3b));

	// ===
	reg_v = get_uint16(q->pending_alarm);
	NPRINT("Pending alarm: ");

	if(reg_v & 0x0002)
	{
		NPRINT("filter ");
	}

	if(reg_v & ~0x0002)
	{
		NPRINT("(0x");
		NPRINT(reg_v);
		NPRINT(")");
	}
	NPRINTLN("");

	// ===
	dump_unknown("unknown_s02_3c", q->unknown3c, sizeof(q->unknown3c));

	// ===
	reg_v = get_uint16(q->acknowledged_alarm);
	NPRINT("Acknowledged alarm: ");

	if(reg_v & 0x0002)
	{
		NPRINT("filter ");
	}

	if(reg_v & ~0x0002)
	{
		NPRINT("(0x");
		NPRINT(reg_v);
		NPRINT(")");
	}
	NPRINTLN("");

	// ===
	dump_unknown("unknown_s02_3d", q->unknown3d, sizeof(q->unknown3d));

	// ===
	for(i = 0 ; i < STATUS_02_NB_SENSORS; i++)
	{
		reg_v = get_int16(q->sensors[i]);

		if(reg_v == -990)		// pas de valeur pour le sensor ? (= -99.0°=
			continue;

		reg_v_f = reg_v;
		reg_v_f /= 10.0;

		NPRINT("Sensor - ");
		NPRINT(sensor_names[i]);
		NPRINT(": ");

		NPRINT(reg_v_f);
		NPRINT("°C (0x");
		NPRINT(reg_v, HEX);
		NPRINT(" 0x");

		if(q->sensors[i][0] < 0x10)
			NPRINT("0");
		NPRINT(q->sensors[i][0], HEX);

		NPRINT(" ");
		if(q->sensors[i][1] < 0x10)
			NPRINT("0");
		NPRINT(q->sensors[i][1], HEX);

		NPRINTLN(")");
	}

	// ===
	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_03(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_03 *q = (R_REPLY_STATUS_03 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	dump_unknown("unknown_s03", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_04(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_04 *q = (R_REPLY_STATUS_04 *)p;

	int reg_v;
	//float reg_v_f;

	// ===
	dump_unknown("unknown_s04", q->unknown, sizeof(q->unknown));

	// ===
	reg_v = get_uint16(q->chauffage_puissance_consommee1);

	NPRINT("Chauffage - Puissance consommée 1: ");
	NPRINT(reg_v);
	NPRINTLN("W");

	// ===
	dump_unknown("unknown_s04_8", q->unknown8, sizeof(q->unknown8));

	// ===
	reg_v = get_uint16(q->chauffage_puissance_consommee2);

	NPRINT("Chauffage - Puissance consommée 2: ");
	NPRINT(reg_v);
	NPRINTLN("W");

	// ===
	reg_v = q->hot_water_production;

	NPRINT("Hot water in progress (s4): ");
	if(reg_v == 0x00)
		NPRINTLN("no");
	else if(reg_v == 0x77)
		NPRINTLN("yes (0x77)");
	else if(reg_v == 0x78)
		NPRINTLN("yes (0x78)");
	else
		NPRINTLN(reg_v, HEX);

	// ===
	dump_unknown("unknown_s04_9", q->unknown9, sizeof(q->unknown9));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_05(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
	int reg_v;

	R_REPLY_STATUS_05 *q = (R_REPLY_STATUS_05 *)p;

	if(q->hot_water_production == 0x00)
		czhp->comfortzone_status.hot_water_production = false;
	else
		czhp->comfortzone_status.hot_water_production = true;

	reg_v = get_uint16(q->room_heating_in_progress);
	if(reg_v == 0x012C)
		czhp->comfortzone_status.room_heating_in_progress = false;
	else
		czhp->comfortzone_status.room_heating_in_progress = true;

	czhp->comfortzone_status.fan_speed = q->fan_speed;
	czhp->comfortzone_status.fan_speed_duty = get_uint16(q->fan_speed_duty);

	czhp->comfortzone_status.room_heating_setting = get_uint16(q->heating_calculated_setting);
	czhp->comfortzone_status.hot_water_calculated_setting = get_uint16(q->hot_water_calculated_setting);

	czhp->comfortzone_status.extra_hot_water_setting = ((q->extra_hot_water == 0x0F)? true : false);

#ifdef DEBUG
	float reg_v_f;
	int i;

	// ===
	reg_v = q->hot_water_production;

	NPRINT("Hot water in progress (s5): ");
	if(reg_v == 0x00)
		NPRINTLN("no");
	else if(reg_v == 0x77)
		NPRINTLN("yes (0x77)");
	else if(reg_v == 0x78)
		NPRINTLN("yes (0x78)");
	else
		NPRINTLN(reg_v, HEX);

	// ===
	dump_unknown("unknown_s05a", q->unknown, sizeof(q->unknown));

	// ===
	reg_v = get_uint16(q->heating_calculated_setting);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heating - Calculated setting: ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");
	
	// ===
	dump_unknown("unknown_s05_02", q->unknown0, sizeof(q->unknown0));

	NPRINT("TE3 Indoor temp history (new values first, 1 value = 10seconds): ");
	// ===
	for(i = 0; i < STATUS_05_TE3_INDOOR_TEMP_HISTORY_NB; i++)
	{
		reg_v = get_int16(q->te3_indoor_temp_history[i]);

		reg_v_f = reg_v;
		reg_v_f /= 10.0;

		NPRINT(reg_v_f);
		NPRINT("°C ");
	}
	NPRINTLN("");

	// ===
	NPRINT("TE2 Return water history (new values first, 1 value = 10seconds): ");
	for(i = 0; i < STATUS_05_TE2_RETURN_WATER_HISTORY_NB; i++)
	{
		reg_v = get_int16(q->te2_return_water_history[i]);

		reg_v_f = reg_v;
		reg_v_f /= 10.0;

		NPRINT(reg_v_f);
		NPRINT("°C ");
	}
	NPRINTLN("");

	// ===
	reg_v = get_uint16(q->room_heating_in_progress);

	NPRINT("Room heating in progress: ");
	if(reg_v == 0x012C)
	{
		NPRINTLN("no (0x012C)");
	}
	else
	{
		NPRINT("yes (0x");
		NPRINT(reg_v, HEX);
		NPRINTLN(")");
	}

	// ===
	dump_unknown("unknown_s05_1", q->unknown1, sizeof(q->unknown1));

	// ===
	reg_v = get_uint16(q->hot_water_calculated_setting);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Hot water Calculated setting (s05): ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");

	// ===
	dump_unknown("unknown_s05_2", q->unknown2, sizeof(q->unknown2));

	// ===
	reg_v = get_uint16(q->unknown_count_down);

	NPRINT("s05 - unknown count down (seems to be hot water remaining runtime. 0x00 00 when not running else count down from hotwater max runtime): ");
	NPRINT(reg_v);
	NPRINTLN("seconds");
	
	// ===
	dump_unknown("unknown_s05_2b", q->unknown2b, sizeof(q->unknown2b));

	// ===
	reg_v = q->extra_hot_water;

	NPRINT("Extra hot water (s05): ");
	if(reg_v == 0x00)
		NPRINTLN("off");
	else if(reg_v == 0x0F)
		NPRINTLN("on");
	else
		NPRINTLN(reg_v, HEX);

	// ===
	dump_unknown("unknown_s05_2c", q->unknown2b, sizeof(q->unknown2b));

	// ===
	reg_v = q->fan_speed;

	NPRINT("Fan speed: ");
	if(reg_v == 0x01)
		NPRINTLN("low");
	else if(reg_v == 0x02)
		NPRINTLN("normal");
	else if(reg_v == 0x03)
		NPRINTLN("high");
	else
		NPRINTLN(reg_v, HEX);

	// ===
	dump_unknown("unknown_s05_3", q->unknown3, sizeof(q->unknown3));

	// ===
	reg_v = get_uint16(q->condensing_temperature);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Condensing temperature: ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");

	// ===
	reg_v = get_uint16(q->condensing_pressure);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Condensing pressure: ");
	NPRINT(reg_v_f);
	NPRINTLN("bar");

	// ===

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_06(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
	R_REPLY_STATUS_06 *q = (R_REPLY_STATUS_06 *)p;

	czhp->comfortzone_status.heatpump_current_compressor_frequency = get_uint16(q->heatpump_current_compressor_frequency);

	czhp->comfortzone_status.heatpump_current_compressor_power = get_uint16(q->heatpump_current_compressor_power);
	czhp->comfortzone_status.heatpump_current_add_power = get_uint16(q->heatpump_current_add_power);
	czhp->comfortzone_status.heatpump_current_total_power = get_uint16(q->heatpump_current_total_power1);
	czhp->comfortzone_status.heatpump_current_compressor_input_power = get_uint16(q->heatpump_compressor_input_power);

#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	// ===
	reg_v = get_uint16(q->evaporator_pressure);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Evaporator pressure: ");
	NPRINT(reg_v_f);
	NPRINTLN("bar");

	// ===
	reg_v = get_uint16(q->pressure_ratio);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Pressure ratio: ");
	NPRINT(reg_v_f);
	NPRINTLN("");

	// ===
	dump_unknown("unknown_s06_0a", q->unknown0a, sizeof(q->unknown0a));

	// ===
	reg_v = get_uint16(q->heatpump_compressor_max_frequency1);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	// not real max as it changes autonomously
	NPRINT("Heatpump - Compressor max frequency (1) (erroneous): ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	reg_v = get_uint16(q->hot_water_active_max_frequency);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	// not real max as it changes autonomously
	NPRINT("Hot water - Compressor active max frequency: ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	// During defrost, forced to 0Hz else set to heating compressor max frequency
	reg_v = get_uint16(q->heatpump_active_max_frequency1);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heatpump - Compressor active max frequency (during defrost, set to 0Hz else real compressor max frequency) (1): ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	dump_unknown("unknown_s06_0c", q->unknown0c, sizeof(q->unknown0c));

	// ===
	reg_v = get_uint16(q->heatpump_active_max_frequency2);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heatpump - Compressor active max frequency (during defrost, set to 0Hz else real compressor max frequency) (2): ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	reg_v = get_uint16(q->heatpump_active_max_frequency3);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heatpump - Compressor active max frequency (during defrost, set to 0Hz else real compressor max frequency) (3): ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	reg_v = get_uint16(q->heatpump_current_compressor_frequency);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heatpump - current compressor frequency: ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	reg_v = get_uint16(q->chauffage_compressor_max_frequency3);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Chauffage - Compressor max frequency (3): ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	dump_unknown("unknown_s06_0d", q->unknown0d, sizeof(q->unknown0d));

	// ===
	reg_v = get_uint16(q->heating_compressor_min_frequency);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heating - Compressor min frequency: ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	reg_v = get_uint16(q->heating_compressor_max_frequency);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heating - Compressor max frequency: ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	dump_unknown("unknown_s06_0", q->unknown0, sizeof(q->unknown0));

	// ===
	reg_v = get_uint16(q->heatpump_current_compressor_power);

	NPRINT("Heatpump - current compressor power: ");
	NPRINT(reg_v);
	NPRINTLN("W");

	// ===
	reg_v = get_uint16(q->heatpump_current_add_power);

	NPRINT("Heatpump - current add power: ");
	NPRINT(reg_v);
	NPRINTLN("W");

	// ===
	reg_v = get_uint16(q->heatpump_current_total_power1);

	NPRINT("Heatpump - current total power 1: ");
	NPRINT(reg_v);
	NPRINTLN("W");

	// ===
	reg_v = get_uint16(q->heatpump_current_total_power2);

	NPRINT("Heatpump - current total power 2: ");
	NPRINT(reg_v);
	NPRINTLN("W");

	// ===
	reg_v = get_uint16(q->heatpump_compressor_input_power);

	NPRINT("Heatpump - Compressor input power: ");
	NPRINT(reg_v);
	NPRINTLN("W");

	// ===
	dump_unknown("unknown_s06_1a", q->unknown1a, sizeof(q->unknown1a));

	// ===
	reg_v = get_uint16(q->unknown_count_down);

	NPRINT("Heatpump - remaining min runtime (?): ");
	NPRINT(reg_v);
	NPRINTLN("seconds");

	// ===
	dump_unknown("unknown_s06_1b", q->unknown1b, sizeof(q->unknown1b));

	// ===
	reg_v = get_uint16(q->heatpump_defrost_delay);

	NPRINT("Heatpump - remaining time to next defrost (=remaining max runtime): ");
	NPRINT(reg_v);
	NPRINTLN("seconds");

	// ===
	dump_unknown("unknown_s06_2", q->unknown2, sizeof(q->unknown2));

	// ===
	reg_v = get_uint16(q->expansion_valve_calculated_setting);
	reg_v_f = reg_v / 10;

	NPRINT("Expansion valve - Calculated setting: ");
	NPRINT(reg_v_f);
	NPRINTLN("K");

	// ===
	reg_v = get_int16(q->vanne_expansion_xxx);
	reg_v_f = reg_v / 10;

	NPRINT("Vanne expansion - xxx?: ");
	NPRINT(reg_v_f);
	NPRINTLN("K");

	// ===
	reg_v = get_int16(q->expansion_valve_temperature_difference1);
	reg_v_f = reg_v / 10;

	NPRINT("Expansion valve - Temperature difference 1: ");
	NPRINT(reg_v_f);
	NPRINTLN("K");

	// ===
	reg_v = get_int16(q->expansion_valve_temperature_difference2);
	reg_v_f = reg_v / 10;

	NPRINT("Expansion valve - Temperature difference 2: ");
	NPRINT(reg_v_f);
	NPRINTLN("K");

	// ===
	dump_unknown("unknown_s06_2a", q->unknown2a, sizeof(q->unknown2a));

	// ===
	reg_v = get_uint16(q->expansion_valve_valve_position1);
	reg_v_f = reg_v / 10;

	NPRINT("Expansion valve - Valve position 1: ");
	NPRINT(reg_v_f);
	NPRINTLN("%");

	// ===
	reg_v = get_uint16(q->expansion_valve_valve_position1);
	reg_v_f = reg_v / 10;

	NPRINT("Expansion valve - Valve position 2: ");
	NPRINT(reg_v_f);
	NPRINTLN("%");

	// ===
	dump_unknown("unknown_s06_2a", q->unknown2a, sizeof(q->unknown2a));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_07(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_07 *q = (R_REPLY_STATUS_07 *)p;

	int reg_v;
	//float reg_v_f;

	// ===
	dump_unknown("unknown_s07", q->unknown, sizeof(q->unknown));

	// ===
	reg_v = get_uint16(q->input_power_limit);

	NPRINT("Input power limit: ");
	NPRINT(reg_v);
	NPRINTLN("W");

	// ===
	dump_unknown("unknown_s07_2", q->unknown2, sizeof(q->unknown2));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_08(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
	R_REPLY_STATUS_08 *q = (R_REPLY_STATUS_08 *)p;

	czhp->comfortzone_status.compressor_energy = get_uint32(q->compressor_energy);
	czhp->comfortzone_status.add_energy = get_uint32(q->add_energy);
	czhp->comfortzone_status.hot_water_energy = get_uint32(q->hot_water_energy);

	czhp->comfortzone_status.compressor_runtime = get_uint32(q->compressor_runtime);
	czhp->comfortzone_status.total_runtime = get_uint32(q->total_runtime);

#ifdef DEBUG
	int reg_v;
	float reg_v_f;

	// ===
	reg_v = q->bcd_second;

	NPRINT("Second (BCD): ");
	NPRINTLN(reg_v, HEX);

	// ===
	reg_v = q->bcd_minute;

	NPRINT("Minute (BCD): ");
	NPRINTLN(reg_v, HEX);

	// ===
	reg_v = q->bcd_hour;

	NPRINT("Hour (BCD): ");
	NPRINTLN(reg_v, HEX);

	// ===
	dump_unknown("unknown_s08_0 (increase by 1 every day but it is not day of week)", &(q->unknown0), sizeof(q->unknown0));

	// ===
	reg_v = q->bcd_day;

	NPRINT("Day (BCD): ");
	NPRINTLN(reg_v, HEX);

	// ===
	reg_v = q->bcd_month;

	NPRINT("Month (BCD): ");
	NPRINTLN(reg_v, HEX);

	// ===
	reg_v = q->bcd_year;

	NPRINT("Year (BCD)(20xx): ");
	NPRINTLN(reg_v, HEX);

	// ===
	dump_unknown("unknown_s08_0a", q->unknown0a, sizeof(q->unknown0a));

	// ===
	dump_unknown("unknown_s08_", q->unknown, sizeof(q->unknown));

	// ===
	reg_v = get_uint32(q->compressor_energy);

	reg_v_f = reg_v;
	reg_v_f /= 100.0;

	NPRINT("Compressor energy: ");
	NPRINT(reg_v_f);
	NPRINTLN("kWh");

	// ===
	reg_v = get_uint32(q->add_energy);

	reg_v_f = reg_v;
	reg_v_f /= 100.0;

	NPRINT("Add energy: ");
	NPRINT(reg_v_f);
	NPRINTLN("kWh");

	// ===
	reg_v = get_uint32(q->hot_water_energy);

	reg_v_f = reg_v;
	reg_v_f /= 100.0;

	NPRINT("Hot water energy: ");
	NPRINT(reg_v_f);
	NPRINTLN("kWh");

	// ===
	reg_v = get_uint32(q->compressor_runtime);

	NPRINT("Compressor runtime: ");
	NPRINT(reg_v / 60);
	NPRINT(":");
	reg_v = reg_v % 60;
	if(reg_v < 10)
		NPRINT("0");
	NPRINT(reg_v);
	NPRINTLN("");

	// ===
	reg_v = get_uint32(q->total_runtime);

	NPRINT("Total runtime: ");
	NPRINT(reg_v / 60);
	NPRINT(":");
	reg_v = reg_v % 60;
	if(reg_v < 10)
		NPRINT("0");
	NPRINT(reg_v);
	NPRINTLN("");

	// ===
	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_09(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
	int reg_v;

	R_REPLY_STATUS_09 *q = (R_REPLY_STATUS_09 *)p;

	reg_v = get_uint16(q->hotwater_priority);

	if(reg_v == 0x4151)
		czhp->comfortzone_status.hot_water_priority_setting = 1;
	else if(reg_v == 0x4152)
		czhp->comfortzone_status.hot_water_priority_setting = 2;
	else if(reg_v == 0x4153)
		czhp->comfortzone_status.hot_water_priority_setting = 3;

#ifdef DEBUG
	float reg_v_f;
	int i;

	// ===
	dump_unknown("unknown_s09", q->unknown, sizeof(q->unknown));

	// ===
	reg_v = get_uint16(q->hotwater_priority);

	NPRINT("Hot water priority: ");
	if(reg_v == 0x4151)
		NPRINTLN("low");
	else if(reg_v == 0x4152)
		NPRINTLN("normal");
	else if(reg_v == 0x4153)
		NPRINTLN("high");
	else
		NPRINTLN(reg_v, HEX);

	// ===
	dump_unknown("unknown_s09_2", q->unknown2, sizeof(q->unknown2));

	// ===
	for(i = 0; i < STATUS_09_NB_AJUSTMENT; i++)
	{
		reg_v = q->hardware_settings_adjustments_teX_adjust[i];

		reg_v_f = reg_v;
		reg_v_f /= 10.0;

		NPRINT("Hardware settings - Adjustments - TE");
		NPRINT(i);
		NPRINT(" Adjust: ");

		NPRINT(reg_v_f);
		NPRINT("°C (0x");

		if(reg_v < 0x10)
			NPRINT("0");
		NPRINT(reg_v, HEX);

		NPRINTLN(")");
	}

	// ===
	dump_unknown("unknown_s09_6", q->unknown6, sizeof(q->unknown6));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_10(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_10 *q = (R_REPLY_STATUS_10 *)p;

	int reg_v;
	float reg_v_f;
	int i;

	// ===
	// seems to never change
	dump_unknown("unknown_s10_", q->unknown, sizeof(q->unknown));

	// ===
	reg_v = get_uint16(q->holiday_temperature_reduction);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Holiday temperature reduction: ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");

	// ===
	reg_v = get_uint16(q->holiday_minimal_room_temperature);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("CW Minimal room temperature: ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");

	// ===
	reg_v = get_uint16(q->cw_minimal_temperature);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("CW Minimal temperature: ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");

	// ===
	reg_v = get_uint16(q->cw_maximal_temperature);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("CW Maximal temperature: ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");

	// ===
	reg_v = get_uint16(q->cw_overheat_hysteresis);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("CW overheat hysteresis: ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");

	// ===
	// seems to never change
	dump_unknown("unknown_s10_2a", q->unknown2a, sizeof(q->unknown2a));

	// ===
	reg_v = get_uint16(q->cw_return_maximal_temperature);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("CW return maximal temperature: ");
	NPRINT(reg_v_f);
	NPRINTLN("°C");

	// ===
	// seems to never change
	dump_unknown("unknown_s10_2b", q->unknown2b, sizeof(q->unknown2b));

	// ===
	reg_v = get_uint16(q->cw_pump);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("CW pump: ");
	NPRINT(reg_v_f);
	NPRINTLN("%");

	// ===
	dump_unknown("unknown_s10_2", q->unknown2, sizeof(q->unknown2));

	// ===
	for(i = 0; i < STATUS_10_STEP_WISE_HEATING_NB; i++)
	{
		reg_v = get_uint16(q->step_wise_heating_temperature[i]);

		reg_v_f = reg_v;
		reg_v_f /= 10.0;

		NPRINT("Step-wise heating temperature ");
		NPRINT(i);
		NPRINT(": ");
		NPRINT(reg_v_f);
		NPRINTLN("°C");

		reg_v_f = q->step_wise_heating_days[i];
		reg_v_f /= 10.0;

		NPRINT("Step-wise heating days ");
		NPRINT(i);
		NPRINT(": ");
		NPRINT(reg_v_f);
		NPRINTLN("day");
	}

	// ===
	dump_unknown("unknown_s10_3", q->unknown3, sizeof(q->unknown3));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_11(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
	R_REPLY_STATUS_11 *q = (R_REPLY_STATUS_11 *)p;

	czhp->comfortzone_status.led_luminosity_setting = q->led_luminosity;

#ifdef DEBUG
	int reg_v;
	//float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_s11_", q->unknown, sizeof(q->unknown));

	// ===
	reg_v = q->led_luminosity;

	NPRINT("Led luminosity: ");
	NPRINTLN(reg_v);

	// ===
	// seems to never change
	dump_unknown("unknown_s11_2", q->unknown2, sizeof(q->unknown2));

	// ===
	reg_v = get_uint16(q->holiday_reduction);

	NPRINT("Holiday reduction: ");
	if(reg_v == 0)
		NPRINTLN("off");
	else
	{
		NPRINT(reg_v);
		NPRINTLN(" day(s)");
	}

	// ===
	// seems to never change
	dump_unknown("unknown_s11_3", q->unknown3, sizeof(q->unknown3));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_12(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_12 *q = (R_REPLY_STATUS_12 *)p;

	int reg_v;
	float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_s12_", q->unknown, sizeof(q->unknown));

	// ===
	reg_v = get_uint16(q->heatpump_compressor_blocked_frequency1);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heatpump - compressor - blocked frequency 1: ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	reg_v = get_uint16(q->heatpump_compressor_blocked_frequency2);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heatpump - compressor - blocked frequency 2: ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	reg_v = get_uint16(q->heatpump_compressor_blocked_frequency3);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heatpump - compressor - blocked frequency 3: ");
	NPRINT(reg_v_f);
	NPRINTLN("Hz");

	// ===
	dump_unknown("unknown_s12_3", q->unknown3, sizeof(q->unknown3));

	// ===
	reg_v = get_uint16(q->heatpump_defrost_delay);

	reg_v_f = reg_v;
	reg_v_f /= 10.0;

	NPRINT("Heatpump - defrost delay: ");
	NPRINT(reg_v_f);
	NPRINTLN("min");

	// ===
	// seems to never change
	dump_unknown("unknown_s12_8", q->unknown8, sizeof(q->unknown8));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_13(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_13 *q = (R_REPLY_STATUS_13 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_s13", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_14(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_14 *q = (R_REPLY_STATUS_14 *)p;

	int reg_v;
	//float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_s14", q->unknown, sizeof(q->unknown));

	// ===
	reg_v = q->language;

	NPRINT("Language: ");
	if(reg_v == 0x01)
		NPRINTLN("English");
	else if(reg_v == 0x04)
		NPRINTLN("Français");
	else
		NPRINTLN(reg_v, HEX);

	// ===
	// seems to never change
	dump_unknown("unknown_s14d", q->unknownd, sizeof(q->unknownd));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_15(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_15 *q = (R_REPLY_STATUS_15 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_s15", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_16(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_16 *q = (R_REPLY_STATUS_16 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_s16", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_17(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_17 *q = (R_REPLY_STATUS_17 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_s17", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_18(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_18 *q = (R_REPLY_STATUS_18 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_s18", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_19(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_19 *q = (R_REPLY_STATUS_19 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	// seems to always contain '\0'
	dump_unknown("unknown_s19", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_20(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_20 *q = (R_REPLY_STATUS_20 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	// seems to always contain '\0'
	dump_unknown("unknown_s20", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_22(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_22 *q = (R_REPLY_STATUS_22 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	dump_unknown("unknown_s22", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_23(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_23 *q = (R_REPLY_STATUS_23 *)p;

	int reg_v;
	//float reg_v_f;
	int i;

	// ===
	// pas de rapport avec le fonctionnement du chauffage à priori car pas de changement entre off=>start=>stop=>off
	for(i = 0; i < STATUS_23_UNKNOWN1_NB; i++)
	{
		reg_v = get_uint16(q->unknown1[i]);

		NPRINT("S23 Unknown1 [");
		NPRINT(i);
		NPRINT("]: ");
		NPRINT(reg_v);
		NPRINT(" (0x");

		if(((reg_v>>8)&0xFF) < 0x10)
			NPRINT("0");
		NPRINT(((reg_v>>8)&0xFF), HEX);

		NPRINT(" ");
		if((reg_v & 0xFF) < 0x10)
			NPRINT("0");
		NPRINT((reg_v & 0xFF), HEX);

		NPRINTLN(")");
	}

	// ===
	// seems to never change
	dump_unknown("unknown_s23_", q->unknown, sizeof(q->unknown));

	// ===
	// pas de rapport avec le fonctionnement du chauffage à priori car pas de changement entre off=>start=>stop=>off
	for(i = 0; i < STATUS_23_UNKNOWN2_NB; i++)
	{
		reg_v = get_uint16(q->unknown2[i]);

		NPRINT("S23 Unknown2 [");
		NPRINT(i);
		NPRINT("]: ");
		NPRINT(reg_v);
		NPRINT(" (0x");

		if(((reg_v>>8)&0xFF) < 0x10)
			NPRINT("0");
		NPRINT(((reg_v>>8)&0xFF), HEX);

		NPRINT(" ");
		if((reg_v & 0xFF) < 0x10)
			NPRINT("0");
		NPRINT((reg_v & 0xFF), HEX);

		NPRINTLN(")");
	}

	// ===
	// seems to never change
	dump_unknown("unknown_s23_3", q->unknown3, sizeof(q->unknown3));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_24(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_24 *q = (R_REPLY_STATUS_24 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	dump_unknown("unknown_s24", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_25(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_25 *q = (R_REPLY_STATUS_25 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_s25", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_v180_x58(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_V180_STATUS_x58 *q = (R_REPLY_STATUS_V180_STATUS_x58 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_v180_x58", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_v180_x68(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_V180_STATUS_x68 *q = (R_REPLY_STATUS_V180_STATUS_x68 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_v180_x68", q->unknown, sizeof(q->unknown));
	dump_unknown("unknown1_v180_x68", q->unknown1, sizeof(q->unknown1));
	dump_unknown("unknown2_v180_x68", q->unknown2, sizeof(q->unknown2));
	dump_unknown("unknown3_v180_x68", q->unknown3, sizeof(q->unknown3));
	dump_unknown("unknown4_v180_x68", q->unknown4, sizeof(q->unknown4));
	dump_unknown("unknown5_v180_x68", q->unknown5, sizeof(q->unknown5));
	dump_unknown("unknown6_v180_x68", q->unknown6, sizeof(q->unknown6));
	dump_unknown("unknown7_v180_x68", q->unknown7, sizeof(q->unknown7));
	dump_unknown("unknown8_v180_x68", q->unknown8, sizeof(q->unknown8));
	dump_unknown("unknown9_v180_x68", q->unknown9, sizeof(q->unknown9));
	dump_unknown("unknown10_v180_x68", q->unknown10, sizeof(q->unknown10));
	dump_unknown("unknown11_v180_x68", q->unknown11, sizeof(q->unknown11));
	dump_unknown("unknown12_v180_x68", q->unknown12, sizeof(q->unknown12));
	dump_unknown("unknown13_v180_x68", q->unknown13, sizeof(q->unknown13));
	dump_unknown("unknown14_v180_x68", q->unknown14, sizeof(q->unknown14));
	dump_unknown("unknown15_v180_x68", q->unknown15, sizeof(q->unknown15));
	dump_unknown("unknown16_v180_x68", q->unknown16, sizeof(q->unknown16));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_v180_x40(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_V180_STATUS_x40 *q = (R_REPLY_STATUS_V180_STATUS_x40 *)p;

	int reg_v;
	float reg_v_f;
	int i;

	for(i = 0; i < STATUS_V180_x40_NB_TEMP; i++)
	{
		reg_v = get_int16(q->temp[i]);

		reg_v_f = reg_v;
		reg_v_f /= 10.0;

		NPRINT("?Temp #");
		NPRINT(i);
		NPRINT(": ");
		NPRINT(reg_v_f);
		NPRINT("°C");
		NPRINT(" ");
		dump_unknown("", q->temp[i], 2);
	}

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_v180_x26(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_V180_STATUS_x26 *q = (R_REPLY_STATUS_V180_STATUS_x26 *)p;

	//int reg_v;
	//float reg_v_f;

	// ===
	// seems to never change
	dump_unknown("unknown_v180_x26", q->unknown, sizeof(q->unknown));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_v180_x8d(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_V180_STATUS_x8d *q = (R_REPLY_STATUS_V180_STATUS_x8d *)p;

	int reg_v;
	float reg_v_f;
	int i;

	for(i = 0; i < STATUS_V180_x8d_NB_TEMP; i++)
	{
		reg_v = get_int16(q->temp[i]);

		reg_v_f = reg_v;
		reg_v_f /= 10.0;

		NPRINT("?Temp #");
		NPRINT(i);
		NPRINT(": ");
		NPRINT(reg_v_f);
		NPRINT("°C");
		NPRINT(" ");
		dump_unknown("", q->temp[i], 2);
	}

	// ===
	// seems to never change
	dump_unknown("unknown_v180_x8d", q->unknown, sizeof(q->unknown));

	for(i = 0; i < STATUS_V180_x8d_NB_TEMP1; i++)
	{
		reg_v = get_int16(q->temp1[i]);

		reg_v_f = reg_v;
		reg_v_f /= 10.0;

		NPRINT("?Temp1 #");
		NPRINT(i);
		NPRINT(": ");
		NPRINT(reg_v_f);
		NPRINT("°C");
		NPRINT(" ");
		dump_unknown("", q->temp1[i], 2);
	}

	// ===
	// seems to never change
	dump_unknown("unknown1_v180_x8d", q->unknown1, sizeof(q->unknown1));
	dump_unknown("unknown2_v180_x8d", q->unknown2, sizeof(q->unknown2));

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}

void czdec::reply_r_status_v180_x2e(comfortzone_heatpump *czhp, KNOWN_REGISTER *kr, R_REPLY *p)
{
#ifdef DEBUG
	R_REPLY_STATUS_V180_STATUS_x2e *q = (R_REPLY_STATUS_V180_STATUS_x2e *)p;

	int reg_v;
	float reg_v_f;

	reg_v = get_uint32(q->unknown1);
	NPRINT("unknown1_v180_x2e : ");
	NPRINT(reg_v);
	NPRINT(" ");
	dump_unknown("", q->unknown1, 4);

	// ===
	reg_v = get_uint32(q->total_runtime);

	NPRINT("Total runtime: ");
	NPRINT(reg_v / 60);
	NPRINT(":");
	reg_v = reg_v % 60;
	if(reg_v < 10)
		NPRINT("0");
	NPRINT(reg_v);
	NPRINTLN("");

	// ===
	reg_v = get_uint32(q->compressor_runtime);

	NPRINT("Compressor runtime: ");
	NPRINT(reg_v / 60);
	NPRINT(":");
	reg_v = reg_v % 60;
	if(reg_v < 10)
		NPRINT("0");
	NPRINT(reg_v);
	NPRINTLN("");

	// ===
	reg_v = get_uint32(q->compressor_energy);

	reg_v_f = reg_v;
	reg_v_f /= 100.0;

	NPRINT("Compressor energy: ");
	NPRINT(reg_v_f);
	NPRINTLN("kWh");

	// ===
	reg_v = get_uint32(q->add_energy);

	reg_v_f = reg_v;
	reg_v_f /= 100.0;

	NPRINT("Add energy: ");
	NPRINT(reg_v_f);
	NPRINTLN("kWh");

	// ===
	reg_v = get_uint32(q->hot_water_energy);

	reg_v_f = reg_v;
	reg_v_f /= 100.0;

	NPRINT("Hot water energy: ");
	NPRINT(reg_v_f);
	NPRINTLN("kWh");

	NPRINT("crc: ");
	if(q->crc < 0x10)
		NPRINT("0");
	NPRINTLN(q->crc, HEX);
#endif
}


