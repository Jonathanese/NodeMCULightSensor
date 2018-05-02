
#include "StopWatch.h"
//#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "MQTT.h"

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

StopWatch ReportValue;
StopWatch SendStateTime;
StopWatch CalTime(false);

void setup()
{
	DM_MASK = DM_INFO | DM_ERROR;
	Serial.begin(921600);
	MQTT_Setup();


	tcs.setGain(TCS34725_GAIN_60X);
	tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_2_4MS);
	if (!tcs.begin()) {
		DebugMessage(DM_ERROR, "No Light Sensor Detected");
	}

	pinMode(D5, OUTPUT);
	digitalWrite(D5, LOW);

	pinMode(D6, INPUT_PULLUP);

}

uint16_t clear_state, red_state, green_state, blue_state;
uint64_t red_acc, green_acc, blue_acc;
uint16_t samples;
float red_cal = 1, green_cal = 1, blue_cal = 1;


void loop()
{
	MQTT_Loop();
	accumulate();

	if (!digitalRead(D6))
	{
		//Run Once on entry
		if (!CalTime.isRunning)
		{
			DebugMessage(DM_INFO, "Start Calibration");
			CalTime.start();
			digitalWrite(D5, HIGH);
			samples = 1;
			red_acc = 1;
			green_acc = 1;
			blue_acc = 1;
		}


	}//Run Once on exit
	else if (CalTime.isRunning)
	{
		DebugMessage(DM_INFO, "End Calibration: " + String(samples) + " samples");
		CalTime.stop();
		digitalWrite(D5, LOW);

		uint32_t min;
		min = red_acc;
		if (green_acc < min) min = green_acc;
		if (blue_acc < min) min = blue_acc;

		red_cal = (float)min / red_acc;
		green_cal = (float)min / green_acc;
		blue_cal = (float)min / blue_acc;

		DebugMessage(DM_INFO, "Red: " + String(red_cal) + " Green: " + String(green_cal) + " Blue: " + String(blue_cal));
		
	}
	else if (SendStateTime.getTime() > 60000)
	{
		uint64_t max;
		
		//calibrate the values
		red_acc *= red_cal;
		green_acc *= green_cal;
		blue_acc *= blue_cal;

		//find the max value
		max = red_acc;
		if (green_acc > max) max = green_acc;
		if (blue_acc > max) max = blue_acc;

		//scale to 256
		red_acc *= 0xFF;
		green_acc *= 0xFF;
		blue_acc *= 0xFF;

		//divide out the max value
		red_state = red_acc / max;
		green_state = green_acc / max;
		blue_state = blue_acc / max;



		//Send the state and reset the timer
		DebugMessage(DM_INFO, "Samples: " + String(samples) + " Max: " + String((float)max));
		DebugMessage(DM_INFO, String(red_state) + ", " + String(green_state) + ", " + String(blue_state));
		sendState();
		SendStateTime.reset();
		samples = 1;
		red_acc = 1;
		green_acc = 1;
		blue_acc = 1;
	}

}

void accumulate()
{
	uint16_t r, g, b, c;
	tcs.getRawData(&r, &g, &b, &c);
	red_acc += r;
	green_acc += g;
	blue_acc += b;
	samples++;
}

void sendState() {
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

	JsonObject& root = jsonBuffer.createObject();
	root["state"] = "ON";
	JsonObject& color = root.createNestedObject("color");
	color["r"] = red_state;
	color["g"] = green_state;
	color["b"] = blue_state;

	root["brightness"] = clear_state;

	char* buffer = new char[root.measureLength() + 1];
	root.printTo(buffer, root.measureLength() + 1);

	publish(buffer);

	DebugMessage(DM_SEND, "Send State: " + String(buffer));

	delete buffer;
}