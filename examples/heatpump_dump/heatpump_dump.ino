#include <Arduino.h>

#include "comfortzone_heatpump.h"

// Basic example showing how to use library to decode message.
// Code was tested on teensy 3.2 with RS485 module connected to
// Serial3 and DE/RE pin connected to pin 2.

// Note: during the first 5 seconds, some (all) variables of
// heatpump.comfortzone_status variable are not initialized

// Serial connected to RS485 module
#define RS485SER Serial3

// pin connected to DE/RE pin of RS485 module
#define RS485_DE_PIN 2

comfortzone_heatpump heatpump;

// print periodic status every X seconds
#define PERIODIC_STATUS 5

static unsigned long last_status_report = 0;

void periodic_status()
{  
	unsigned long int now;
	
	now = millis();
	
	if( (now - last_status_report) > (PERIODIC_STATUS * 1000) )
	{  // periodically report status to domoticz to avoid red status

		Serial.print("Fan - time to filter change: ");
		Serial.print(heatpump.comfortzone_status.fan_time_to_filter_change);
		Serial.println(" days");

		Serial.print("Fan speed: ");
		switch(heatpump.comfortzone_status.fan_speed)
		{
			case 1:	Serial.println("low");
						break;
			case 2:	Serial.println("normal");
						break;
			case 3:	Serial.println("high");
						break;
		}

		Serial.print("Hot water production: ");
		Serial.println((heatpump.comfortzone_status.hot_water_production ? "yes" : "no" ));

		Serial.print("Room heating in progress: ");
		Serial.println((heatpump.comfortzone_status.room_heating_in_progress ? "yes" : "no" ));

		Serial.print("Compressor activity: ");
		switch(heatpump.comfortzone_status.compressor_activity)
		{
			case CZCMP_STOPPED:	Serial.println("stopped");
										break;
   		case CZCMP_STOPPING:Serial.println("shutting down");
										break;
   		case CZCMP_RUNNING:	Serial.println("running");
										break;
   		case CZCMP_UNKNOWN:	Serial.println("unknown");
										break;
		}

		Serial.print("Additional power (resistor): ");
		Serial.println((heatpump.comfortzone_status.additional_power_enabled ? "yes" : "no" ));

		Serial.print("Mode: ");
		switch(heatpump.comfortzone_status.mode)
		{
			case CZMD_IDLE:		Serial.println("idle");
										break;
   		case CZMD_ROOM_HEATING:Serial.println("room heating");
										break;
   		case CZMD_HOT_WATER:Serial.println("hot water");
										break;
   		case CZMD_UNKNOWN:	Serial.println("unknown");
										break;
		}

		Serial.print("Defrost enabled: ");
		Serial.println((heatpump.comfortzone_status.defrost_enabled ? "yes" : "no" ));
		
		Serial.print("TE1 - Flow water: ");
		Serial.print( ((float)(heatpump.comfortzone_status.sensors_te1_flow_water) / 10.0) );
		Serial.println("°C");

		Serial.print("TE2 - Return water: ");
		Serial.print( ((float)(heatpump.comfortzone_status.sensors_te2_return_water) / 10.0) );
		Serial.println("°C");

		Serial.print("TE3 - Indoor temp: ");
		Serial.print( ((float)(heatpump.comfortzone_status.sensors_te3_indoor_temp) / 10.0) );
		Serial.println("°C");

		Serial.print("TE4 - Hot gas temp: ");
		Serial.print( ((float)(heatpump.comfortzone_status.sensors_te4_hot_gas_temp) / 10.0) );
		Serial.println("°C");

		Serial.print("TE5 - Exchanger out: ");
		Serial.print( ((float)(heatpump.comfortzone_status.sensors_te5_exchanger_out) / 10.0) );
		Serial.println("°C");

		Serial.print("TE6 - Evaporator in: ");
		Serial.print( ((float)(heatpump.comfortzone_status.sensors_te6_evaporator_in) / 10.0) );
		Serial.println("°C");

		Serial.print("TE7 - Exhaust air: ");
		Serial.print( ((float)(heatpump.comfortzone_status.sensors_te7_exhaust_air) / 10.0) );
		Serial.println("°C");

		Serial.print("TE24 - Hot water temp: ");
		Serial.print( ((float)(heatpump.comfortzone_status.sensors_te24_hot_water_temp) / 10.0) );
		Serial.println("°C");

		Serial.print("Heatpump - Compressor frequency: ");
		Serial.print( ((float)(heatpump.comfortzone_status.heatpump_current_compressor_frequency) / 10.0) );
		Serial.println("Hz");

		Serial.print("Heatpump - Compressor power: ");
		Serial.print(heatpump.comfortzone_status.heatpump_current_compressor_power);
		Serial.println("W");

		Serial.print("Heatpump - Add power: ");
		Serial.print(heatpump.comfortzone_status.heatpump_current_add_power);
		Serial.println("W");

		Serial.print("Heatpump - Total power: ");
		Serial.print(heatpump.comfortzone_status.heatpump_current_total_power);
		Serial.println("W");

		Serial.print("Heatpump - Compressor input power: ");
		Serial.print(heatpump.comfortzone_status.heatpump_current_compressor_input_power);
		Serial.println("W");

		Serial.print("Compressor energy: ");
		Serial.print( ((float)(heatpump.comfortzone_status.compressor_energy) / 100.0) );
		Serial.println("kWh");

		Serial.print("Add energy: ");
		Serial.print( ((float)(heatpump.comfortzone_status.add_energy) / 100.0) );
		Serial.println("kWh");

		Serial.print("Hot water energy: ");
		Serial.print( ((float)(heatpump.comfortzone_status.hot_water_energy) / 100.0) );
		Serial.println("kWh");

		Serial.print("Compressor runtime: ");
		Serial.print(heatpump.comfortzone_status.compressor_runtime);
		Serial.println(" minutes");

		Serial.print("Total runtime: ");
		Serial.print(heatpump.comfortzone_status.total_runtime);
		Serial.println(" minutes");

		Serial.println("==================================");
		last_status_report = now;
	}
}

void setup()
{
	Serial.begin(115200);

	// let linux detect the new USB device
	delay(1000);

	heatpump.begin(RS485SER, RS485_DE_PIN);
}

void loop()
{
	heatpump.process();

	periodic_status();
}

