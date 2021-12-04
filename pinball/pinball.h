// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Primary Arduino
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#ifndef pinball_h
#define pinball_h

#include <Arduino.h>
#include <FtModules.h>

#include "Simpletypes.h"
#include "pb_child.h"

#include "leds.h"

// Hardware constants

#define SEVENSEGDISPLAY_ADR		0x09
#define DISPLAYCHARS			6

// Enums

enum class gameStates
{
	GAME_START = 1,
	BALL_START,
	LAUNCHING,
	PLAYING,
	NO_MORE_POINTS,
	BALL_LOST,
	SAVE_BALL,
	NEXT_BALL,
	BALL_NEAR_HOME,
	GAME_OVER,
};

// Arduino pin assignments

const byte leftButton = 2;
const byte rightButton = 3;
const byte leftOutlaneSensor = 4;
const byte rightOutlaneSensor = 5;
const byte rolloverSkillSensor = 6;
const byte rollover3Sensor = 7;
const byte rollover2Sensor = 8;
const byte rollover1Sensor = 9;
const byte leftFlipper = 10;
const byte rightFlipper = 11;
const byte ballLostSensor = 12;
const byte stopMagnet = 13;

const byte feederHomeSensor = A0;
const byte ballNearHomeSensor = A1;
const byte spinnerSensor = A2;
const byte leftOrbitSensor = A3;
const byte holdSensor = A6;
const byte launchSensor = A7;

// Analog sensor thresholds

#define MIN_ANALOG_THRESHOLD	10		// Prevent false readings w/ 19 V off
#define LAUNCH_SENSOR_THRESHOLD 600
#define HOLD_SENSOR_THRESHOLD	600

// Sensor macros

#define LEFT_BUTTON_ON		(!digitalRead(leftButton))
#define RIGHT_BUTTON_ON		(!digitalRead(rightButton))
#define LEFT_BUTTON_OFF		(digitalRead(leftButton))
#define RIGHT_BUTTON_OFF	(digitalRead(rightButton))
#define IS_BALL_LOST		(digitalRead(ballLostSensor))

#define ARDUINO_PINS		(A7 + 1)

// Global variables

extern Leds leds;
extern char displayBuffer[];
extern uint sensorState[ARDUINO_PINS];

#endif // pinball_h
