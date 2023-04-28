#include <Arduino.h>

#include "comfortzone_heatpump.h"
#include "rs485_interface.h"

// Example used to dump all received frames and how they are recognized byt the library
// Code was tested on teensy 3.2 with RS485 module connected to
// Serial3 and DE/RE pin connected to pin 2.

// /!\ In comfortzone_config.h, you have to adjust DEBUG and OUTSER define

// Serial connected to RS485 module
#define RS485SER Serial3

// pin connected to DE/RE pin of RS485 module
#define RS485_DE_PIN 2

comfortzone_heatpump heatpump(new ArduinoRS485Interface(RS485SER, RS485_DE_PIN));

void setup()
{
	Serial.begin(115200);

	// let linux detect the new USB device
	delay(1000);

	heatpump.begin();
}

void loop()
{
	heatpump.process();
}

