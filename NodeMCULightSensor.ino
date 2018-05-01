
#include "StopWatch.h"
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "MQTT.h"

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

StopWatch ReportValue;

void setup()
{
	DM_MASK = DM_INFO | DM_ERROR | DM_RECEIVE | DM_SEND;
	Serial.begin(921600);
	MQTT_Setup();

	if (!tcs.begin()) {
		DebugMessage(DM_ERROR, "No Light Sensor Detected");
	}

	pinMode(D5, OUTPUT);
	digitalWrite(D5, LOW);

}

uint16_t clear, red, green, blue;

void loop()
{
	MQTT_Loop();

	if (ReportValue.getTime() > 5000)
	{
		ReportValue.reset();
		tcs.getRawData(&red, &green, &blue, &clear);
		red = (float)red / (float)clear * 256;
		green = (float)green / (float)clear * 256;
		blue = (float)blue / (float)clear * 256;
		DebugMessage(DM_INFO, "R:" + String(red) + "  G:" + String(green) + "  B:" + String(blue));

	}
}