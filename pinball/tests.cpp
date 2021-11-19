/*
	Test functions
*/

#include <Arduino.h>
#include <FtModules.h>
#include <Wire.h>

#include "pinball.h"
#include "leds.h"
#include "display.h"
#include "sound.h"

#pragma region Test functions --------------------------------------------------

// Sensor state variables

bool leftButtonState = true;
bool rightButtonState = true;
bool leftOutlaneSensorState = true;
bool rightOutlaneSensorState = true;
bool rolloverSkillSensorState = true;
bool rollover3SensorState = true;
bool rollover2SensorState = true;
bool rollover1SensorState = true;
bool ballLostSensorState = true;
bool feederHomeSensorState = true;
bool ballNearHomeSensorState = true;
bool spinnerSensorState = true;
bool leftOrbitSensorState = true;

void testLeds()
{
	leds.Flash(childLeds::LEFT_ORBIT, 100);
	leds.Flash(childLeds::ROLLOVER1, 100);
	leds.OneShot(childLeds::ROLLOVER2, 100);

	delay(1000);

	leds.Off(childLeds::LEFT_ORBIT);
	leds.Off(childLeds::ROLLOVER1);

	delay(1000);
}

uint cLed = 0;
uint cCol = 1;

void testAllLeds()
{
	Display::Stop();
	leds.Off((childLeds)cLed);
	cLed = cLed == 8 ? 0 : cLed + 1;
	if(cLed == 8) {
		cCol = cCol == 9 ? 1 : cCol + 1;
	}
	leds.On((childLeds)cLed);
	delay(500);
	Display::Clear();
	Display::U2s(displayBuffer, cLed);
	Display::Show(displayBuffer);
}

void testSensor(byte sensor, bool *last, char *name)
{
	bool state = digitalRead(sensor);

	if(state != *last) {
		Serial.print(name);
		Serial.print(": ");
		Serial.println(state);
		*last = state;
	}
}

void testAnalogSensor(byte sensor, uint min, uint max, char *name)
{
	uint value = analogRead(sensor);

	if(value >= min && value < max) {
		Serial.print(name);
		Serial.print(": ");
		Serial.println(value);
		delay(50);
	}
}

void analogSensorTestLoop()
{
	Serial.print("Hold: ");
	Serial.print(analogRead(holdSensor));
	delay(50);
	Serial.print(" / Launch: ");
	Serial.println(analogRead(launchSensor));
	delay(50);
}

void testSounds()
{
	for(int i = soundNames::DING; i <= soundNames::BAMBOO; i++) {
		Sound::Play(i);
		Serial.print("Sound #");
		Serial.println(i);
		Display::Stop();
		Display::U2s(displayBuffer, i);
		Display::Show(displayBuffer);
		leds.On((childLeds)(i - 1));
		delay(2000);
		leds.Off((childLeds)(i - 1));
	}
}

void testInputs()
{
	// Digital sensors

	testSensor(leftButton, &leftButtonState, "Left button");
	testSensor(rightButton, &rightButtonState, "Right button");
	testSensor(leftOutlaneSensor, &leftOutlaneSensorState, "Left outlane");
	testSensor(rightOutlaneSensor, &rightOutlaneSensorState, "Right outlane");
	testSensor(rolloverSkillSensor, &rolloverSkillSensorState, "Skill shot rollover");
	testSensor(rollover3Sensor, &rollover3SensorState, "Rollover 3");
	testSensor(rollover2Sensor, &rollover2SensorState, "Rollover 2");
	testSensor(rollover1Sensor, &rollover1SensorState, "Rollover 1");
	testSensor(ballLostSensor, &ballLostSensorState, "Ball lost");
	testSensor(feederHomeSensor, &feederHomeSensorState, "Feeder at home");
	testSensor(ballNearHomeSensor, &ballNearHomeSensorState, "Ball near home");
	testSensor(spinnerSensor, &spinnerSensorState, "Spinner");
	testSensor(leftOrbitSensor, &leftOrbitSensorState, "Left orbit");

	// Analog sensors

	testAnalogSensor(holdSensor, MIN_ANALOG_THRESHOLD, HOLD_SENSOR_THRESHOLD, "Hold");
	testAnalogSensor(launchSensor, MIN_ANALOG_THRESHOLD, LAUNCH_SENSOR_THRESHOLD, "Ball launched");
}

#pragma endregion --------------------------------------------------------------
