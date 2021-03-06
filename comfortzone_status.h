#ifndef _COMFORTZONE_STATUS_H
#define _COMFORTZONE_STATUS_H

#include <Arduino.h>

typedef enum
{
	CZCMP_STOPPED,			// compressor is stopped
	CZCMP_STOPPING,		// compressor is shutting down
	CZCMP_RUNNING,			// compressor is up and running
	CZCMP_UNKNOWN,			// undefined status (very rare)
} COMFORTZONE_COMPRESSOR_ACTIVITY;

typedef enum
{
	CZMD_IDLE,				// no mode chosen (rare)
	CZMD_ROOM_HEATING,	// default mode (even when everything is off)
	CZMD_HOT_WATER,		// hot water production
	CZMD_UNKNOWN,			// undefined mode	(rarer)
} COMFORTZONE_MODE;

typedef struct
{
	uint16_t fan_time_to_filter_change;		// days

	bool filter_alarm;							// true = replace/clean filter alarm, false = filter ok

	bool hot_water_production;					// true = on, false = off
	bool room_heating_in_progress;			// true = on, false = off

	COMFORTZONE_COMPRESSOR_ACTIVITY compressor_activity;
	bool additional_power_enabled;			// true = resistor is enabled, false = resistor is disabled

	COMFORTZONE_MODE mode;
	bool defrost_enabled;						// true = defrost in progress, false = no defrost in progress

	int16_t sensors_te1_flow_water;			// °C, * 10
	int16_t sensors_te2_return_water;		// °C, * 10
	int16_t sensors_te3_indoor_temp;			// °C, * 10
	int16_t sensors_te4_hot_gas_temp;		// °C, * 10
	int16_t sensors_te5_exchanger_out;		// °C, * 10
	int16_t sensors_te6_evaporator_in;		// °C, * 10
	int16_t sensors_te7_exhaust_air;			// °C, * 10
	int16_t sensors_te24_hot_water_temp;	// °C, * 10

	int16_t heatpump_current_compressor_frequency;		// Hz, * 10

	int16_t heatpump_current_compressor_power;	// W = power generated by compressor
	int16_t heatpump_current_add_power;				// W = additional power (resistor)
	int16_t heatpump_current_total_power;			// W = total power produced
	int16_t heatpump_current_compressor_input_power;	// W = power sent to compressor

	uint32_t compressor_energy;				// kWh, * 100
	uint32_t add_energy;							// kWh, * 100
	uint32_t hot_water_energy;					// kWh, * 100

	uint32_t compressor_runtime;				// minutes
	uint32_t total_runtime;						// minutes

	// heatpump current time
	byte hour;
	byte minute;
	byte second;

	// heatpump currant day
	byte day;
	byte month;
	uint16_t year;

	byte day_of_week;								// 1 = monday, 7 = sunday

	// current user settings
	byte fan_speed;								// 1 = low, 2 = normal, 3 = fast

	int16_t room_heating_setting;				// °C, * 10 (user selected)
	int16_t hot_water_setting;					// °C, * 10 (user selected)
	bool extra_hot_water_setting;				// true = on, false = off

	byte hot_water_priority_setting;			// 1 = low, 2 = normal, 3 = fast

	byte led_luminosity_setting;				// 0 = off -> 6 = highest level

	// current heatpump calculated setting
	uint16_t fan_speed_duty;					// %, * 10
	int16_t hot_water_calculated_setting;	// °C, * 10 (heatpump selected). can be hot_water_setting (no extra hot water) or a different value
} COMFORTZONE_STATUS;

#endif
