// MQTT.h

#ifndef _MQTT_h
#define _MQTT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "WiFi.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

void MQTT_Setup();
void MQTT_Loop();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

#endif

