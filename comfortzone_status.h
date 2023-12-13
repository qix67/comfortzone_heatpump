#ifndef _COMFORTZONE_STATUS_H
#define _COMFORTZONE_STATUS_H

#include "platform_specific.h"

typedef enum
{
	CZCMP_STOPPED,			// compressor is stopped
	CZCMP_STOPPING,		// compressor is shutting down
	CZCMP_RUNNING,			// compressor is up and running
	CZCMP_UNKNOWN,			// undefined status (very rare)
#if HP_PROTOCOL == HP_PROTOCOL_1_8
	CZCMP_DEFROST,			// (v1.8 only)
#endif
} COMFORTZONE_COMPRESSOR_ACTIVITY;

typedef enum
{
	CZMD_IDLE,				// no mode chosen (rare)
	CZMD_ROOM_HEATING,	// default mode (even when everything is off)
	CZMD_HOT_WATER,		// hot water production
	CZMD_UNKNOWN,			// undefined mode	(rarer)
#if HP_PROTOCOL == HP_PROTOCOL_1_8
	CZMD_ROOM_HEATING_AND_HOT_WATER,		// room heating + hot water production
#endif
} COMFORTZONE_MODE;

template <typename T>
class Subscribable {
public:
	using callback_t = void (*)(T, void*);
	
	void subscribe(callback_t callback, void *context) {
		_on_change = callback;
		_context = context;
	}

	Subscribable<T>& operator=(const T& value) {
		_value = value;
		if (_on_change != NULL) {
			_on_change(_value, _context);
		}
		return *this;
	}

	operator T() const {
		return _value;
	}

private:
	T _value;
	callback_t _on_change = NULL;
	void *_context = NULL;
};

// Heatpump status
// Not all values are available on all protocol version
// Supported protocols are 1.60, 1.80, 2.21. If no protocol version is specified, only 1.60 is supported
typedef struct
{
	Subscribable<uint16_t> fan_time_to_filter_change;		// days (proto: 1.60, 1.80, 2.21)

	Subscribable<bool> filter_alarm;								// true = replace/clean filter alarm, false = filter ok (proto: 1.60, 1.80)

	Subscribable<bool> hot_water_production;					// true = on, false = off
	Subscribable<bool> room_heating_in_progress;				// true = on, false = off

	Subscribable<COMFORTZONE_COMPRESSOR_ACTIVITY> compressor_activity;		// (proto: 1.60, 1.80)
	Subscribable<bool> additional_power_enabled;				// true = resistor is enabled, false = resistor is disabled (proto: 1.60, 1.80)

	Subscribable<COMFORTZONE_MODE> mode;						// (proto: 1.60, 1.80)
	Subscribable<bool> defrost_enabled;							// true = defrost in progress, false = no defrost in progress (proto: 1.60, 1.80)

	Subscribable<int16_t> sensors_te0_outdoor_temp;			// °C, * 10 (proto: 1.60, 1.80)
	Subscribable<int16_t> sensors_te1_flow_water;			// °C, * 10 (proto: 1.60, 1.80)
	Subscribable<int16_t> sensors_te2_return_water;			// °C, * 10 (proto: 1.60, 1.80)
	Subscribable<int16_t> sensors_te3_indoor_temp;			// °C, * 10 (proto: 1.60, 1.80)
	Subscribable<int16_t> sensors_te4_hot_gas_temp;			// °C, * 10 (proto: 1.60, 1.80)
	Subscribable<int16_t> sensors_te5_exchanger_out;		// °C, * 10 (proto: 1.60, 1.80)
	Subscribable<int16_t> sensors_te6_evaporator_in;		// °C, * 10 (proto: 1.60, 1.80)
	Subscribable<int16_t> sensors_te7_exhaust_air;			// °C, * 10 (proto: 1.60, 1.80)
	Subscribable<int16_t> sensors_te24_hot_water_temp;		// °C, * 10 (proto: 1.60, 1.80)

	Subscribable<int16_t> heatpump_current_compressor_frequency;	// Hz, * 10 (proto: 1.60, 1.80)

	Subscribable<int16_t> heatpump_current_compressor_power;			// W = power generated by compressor (proto: 1.60, 1.80)
	Subscribable<int16_t> heatpump_current_add_power;					// W = additional power (resistor) (proto: 1.60, 1.80)
	Subscribable<int16_t> heatpump_current_total_power;				// W = total power produced (proto: 1.60, 1.80)
	Subscribable<int16_t> heatpump_current_compressor_input_power;	// W = power sent to compressor (proto: 1.60, 1.80)

	Subscribable<uint32_t> compressor_energy;					// kWh, * 100 (proto: 1.60, 1.80, 2.21)
	Subscribable<uint32_t> add_energy;							// kWh, * 100 (proto: 1.60, 1.80, 2.21)
	Subscribable<uint32_t> hot_water_energy;					// kWh, * 100 (proto: 1.60, 1.80, 2.21)

	Subscribable<uint32_t> compressor_runtime;				// minutes (proto: 1.60, 1.80, 2.21)
	Subscribable<uint32_t> total_runtime;						// minutes (proto: 1.60, 1.80, 2.21)

	// heatpump current time
	Subscribable<byte> hour; 										// (proto: 1.60, 1.80, 2.21)
	Subscribable<byte> minute; 									// (proto: 1.60, 1.80, 2.21)
	Subscribable<byte> second; 									// (proto: 1.60, 1.80, 2.21)

	// heatpump current day
	Subscribable<byte> day; 										// (proto: 1.60, 1.80, 2.21)
	Subscribable<byte> month; 										// (proto: 1.60, 1.80, 2.21)
	Subscribable<uint16_t> year; 									// (proto: 1.60, 1.80, 2.21)

	Subscribable<byte> day_of_week;								// 1 = monday, 7 = sunday (proto: 1.60, 1.80, 2.21)

	// current user settings
	Subscribable<byte> fan_speed;									// 1 = low, 2 = normal, 3 = fast (proto: 1.60, 1.80, 2.21)

	Subscribable<int16_t> room_heating_setting;				// °C, * 10 (user selected) (proto: 1.60, 1.80, 2.21)
	Subscribable<int16_t> hot_water_setting;					// °C, * 10 (user selected) (proto: 1.60, 1.80)
	Subscribable<bool> extra_hot_water_setting;				// true = on, false = off

	Subscribable<byte> hot_water_priority_setting;			// 1 = low, 2 = normal, 3 = fast

	Subscribable<byte> led_luminosity_setting;				// 0 = off -> 6 = highest level (proto: 1.60, 1.80)

	// current heatpump calculated setting
	Subscribable<uint16_t> fan_speed_duty;						// %, * 10 (proto: 1.60, 1.80)
	Subscribable<int16_t> hot_water_calculated_setting;	// °C, * 10 (heatpump selected). can be hot_water_setting (no extra hot water) or a different value (proto: 1.60, 1.80)

	Subscribable<int16_t> calculated_flow_set;				// °C, * 10 (heatpump selected) (proto: 1.80)
} COMFORTZONE_STATUS;

#endif

// vim: set ts=3:
