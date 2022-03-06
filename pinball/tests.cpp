// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Tests
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include <Arduino.h>
#include <FtModules.h>

#include "tests.h"
#include "pinball.h"
#include "display.h"
#include "sound.h"
#include "servo.h"

#pragma region Variables -------------------------------------------------------

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

uint nSound = 0;
uint nLedTest = 0;
Servo servoTest;

const char *names[] = {
	"DING", "DRAIN", "GLASS", "CLANG", "FAUCET", "CRASH",
	"FRYING", "BUBBLES", "CABINET", "SHAKE"
};

#pragma endregion --------------------------------------------------------------

#pragma region Public methods --------------------------------------------------

void Tests::Leds()
{
	Display::Stop();
	leds.Off((childLeds)nLedTest);
	nLedTest = nLedTest == NLEDS - 1 ? 0 : nLedTest + 1;
	leds.On((childLeds)nLedTest);
	delay(500);
	Display::Clear();
	Display::U2s(displayBuffer, nLedTest);
	Display::Show(displayBuffer);
}

void Tests::Sounds()
{
	if(nSound == 0 || RIGHT_BUTTON_ON) {
		nSound = nSound == soundNames::SHAKE ? 1 : nSound + 1;
		displaySound(nSound);
		delay(100);
	} else if(LEFT_BUTTON_ON) {
		Sound::Play(nSound);
	}
	while(LEFT_BUTTON_ON || RIGHT_BUTTON_ON) {
		delay(1);
	}
	delay(5);
}

void Tests::Inputs()
{
	// Digital sensors

	testDigitalSensor(leftButton, &leftButtonState, "Left button");
	testDigitalSensor(rightButton, &rightButtonState, "Right button");
	testDigitalSensor(leftOutlaneSensor, &leftOutlaneSensorState, "Left outlane");
	testDigitalSensor(rightOutlaneSensor, &rightOutlaneSensorState, "Right outlane");
	testDigitalSensor(rolloverSkillSensor, &rolloverSkillSensorState, "Skill shot rollover");
	testDigitalSensor(rollover3Sensor, &rollover3SensorState, "Rollover 3");
	testDigitalSensor(rollover2Sensor, &rollover2SensorState, "Rollover 2");
	testDigitalSensor(rollover1Sensor, &rollover1SensorState, "Rollover 1");
	testDigitalSensor(ballLostSensor, &ballLostSensorState, "Ball lost");
	testDigitalSensor(feederHomeSensor, &feederHomeSensorState, "Feeder at home");
	testDigitalSensor(ballNearHomeSensor, &ballNearHomeSensorState, "Ball near home");
	testDigitalSensor(spinnerSensor, &spinnerSensorState, "Spinner");
	testDigitalSensor(leftOrbitSensor, &leftOrbitSensorState, "Left orbit");

	// Analog sensors

	testAnalogSensor(holdSensor, MIN_ANALOG_THRESHOLD, HOLD_SENSOR_THRESHOLD, "Hold");
	testAnalogSensor(launchSensor, MIN_ANALOG_THRESHOLD, LAUNCH_SENSOR_THRESHOLD, "Ball launched");
}

void Tests::AnalogSensors()
{
	Serial.print("Hold: ");
	Serial.print(analogRead(holdSensor));
	delay(50);
	Serial.print(" / Launch: ");
	Serial.println(analogRead(launchSensor));
	delay(50);
}

void Tests::Servo()
{
	servoTest.OpenDoor();
	delay(1000);
	servoTest.CloseDoor();
	delay(1000);
}

void Tests::GameState(gameStates state)
{
	Serial.println("----------------------------");
	Serial.print("gameState: ");

	switch(state) {
		case gameStates::GAME_START:
			Serial.println("Game start");
			return;
		case gameStates::BALL_START:
			Serial.println("Ball start");
			return;
		case gameStates::LAUNCHING:
			Serial.println("Launching");
			return;
		case gameStates::PLAYING:
			Serial.println("Playing");
			return;
		case gameStates::NO_MORE_POINTS:
			Serial.println("No more points");
			return;
		case gameStates::BALL_LOST:
			Serial.println("Ball lost");
			return;
		case gameStates::SAVE_BALL:
			Serial.println("Save ball");
			return;
		case gameStates::NEXT_BALL:
			Serial.println("Next ball");
			return;
		case gameStates::BALL_NEAR_HOME:
			Serial.println("Ball near home");
			return;
		case gameStates::GAME_OVER:
			Serial.println("Game over");
			return;
		default:
			Serial.print("Unknown: ");
			Serial.println((int)state);
			return;
	}
}

#pragma endregion --------------------------------------------------------------

#pragma region Private methods -------------------------------------------------

void Tests::testDigitalSensor(byte sensor, bool *last, char *name)
{
	bool state = digitalRead(sensor);

	if(state != *last) {
		Serial.print(name);
		Serial.print(": ");
		Serial.println(state);
		*last = state;
	}
}

void Tests::testAnalogSensor(byte sensor, uint min, uint max, char *name)
{
	uint value = analogRead(sensor);

	if(value >= min && value < max) {
		Serial.print(name);
		Serial.print(": ");
		Serial.println(value);
		delay(50);
	}
}

void Tests::displaySound(byte n)
{
	Serial.print("Sound #");
	Serial.print(n);
	Serial.print(": ");
	Serial.println(names[n - 1]);
	Display::Stop();
	Display::U2s(displayBuffer, n);
	Display::Show(displayBuffer);
}

#pragma endregion --------------------------------------------------------------
