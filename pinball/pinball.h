/*
  pinball.h
*/

#ifndef pinball_h
#define pinball_h

#include <Arduino.h>
#include <FtModules.h>
#include "Simpletypes.h"
#include "child.h"

#include "leds.h"
#include "display.h"
#include "sound.h"

// ------------------------------------------------- Constants for other modules

#define SEVENSEGDISPLAY_ADR		0x09
#define DISPLAYCHARS			6

// ---------------------------------------------------------- Hardware constants

// Arduino pins

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

// '10' as minimum prevents false readings when 19 V is off
#define MIN_ANALOG_THRESHOLD	10
#define LAUNCH_SENSOR_THRESHOLD 600
#define HOLD_SENSOR_THRESHOLD	600

// ------------------------------------------------------------ Global variables

extern "C" FtModules::I2C i2c;
extern "C" Leds leds;
extern "C" char displayBuffer[];

#endif // pinball_h
