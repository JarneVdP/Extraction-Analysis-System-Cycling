#ifndef ENVSENSOR_H
#define ENVSENSOR_H

// https://github.com/DFRobot/DFRobot_EnvironmentalSensor/blob/master/examples/readData/readData.ino
#include "DFRobot_EnvironmentalSensor.h"

	#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
	#include <SoftwareSerial.h>
	#endif

	#define MODESWITCH /*UART:1*/ /*I2C: */ 0

	#if MODESWITCH
	#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
	SoftwareSerial mySerial(/*rx =*/4, /*tx =*/5);
	DFRobot_EnvironmentalSensor environment(/*addr =*/SEN050X_DEFAULT_DEVICE_ADDRESS, /*s =*/&mySerial);
	#else
	DFRobot_EnvironmentalSensor environment(/*addr =*/SEN050X_DEFAULT_DEVICE_ADDRESS, /*s =*/&Serial1);
	#endif
	#else
	// DFRobot_EnvironmentalSensor environment(/*addr = */ SEN050X_DEFAULT_DEVICE_ADDRESS, /*pWire = */ &Wire);
	extern DFRobot_EnvironmentalSensor environment;
	#endif

void envsensor();
#endif