// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Sensor check functions
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include <AsyncDelay.h>

#include "sensors.h"

#include "debounce.h"
#include "flippers.h"
#include "game.h"
#include "messages.h"
#include "sound.h"
#include "tests.h"

#pragma region Macros ----------------------------------------------------------

#define ALL_ROLLOVERS_ON	(rollovers[0] && rollovers[1] && rollovers[2])
#define IS_BALL_LOST		(digitalRead(ballLostSensor))
#define ON_OUTLANE			(!digitalRead(leftOutlaneSensor) || !digitalRead(rightOutlaneSensor))

#pragma endregion --------------------------------------------------------------

#pragma region Hardware variables ----------------------------------------------

extern Messages Msg;

AsyncDelay multipliersTimer;
AsyncDelay skillShotTimer;
AsyncDelay holdTimer;
AsyncDelay holdScoreTimer;

#pragma endregion --------------------------------------------------------------

#pragma region Game variables --------------------------------------------------

extern byte multiplier;
extern byte stopSensorHits;

extern bool skillShotActive;
extern bool holdActive;
extern bool greasyActive;

extern ulong playerScore;
extern ulong lastScore;
extern ulong greasyScore;
extern ulong eobBonus;

bool rollovers[3] = {false, false, false};

#pragma endregion --------------------------------------------------------------

#pragma region External functions ----------------------------------------------

extern void incrementScore(ulong points);

#pragma endregion --------------------------------------------------------------

#pragma region Auxiliary functions ---------------------------------------------

void rotateRollovers()
{
	bool s = rollovers[0];
	rollovers[0] = rollovers[1];
	rollovers[1] = rollovers[2];
	rollovers[2] = s;
}

void showRolloverLeds()
{
	if(ALL_ROLLOVERS_ON) {
		for(int i = 0; i <= 2; i++) {
			leds.On((childLeds)i);
		}
	} else {
		for(int i = 0; i <= 2; i++) {
			if(rollovers[i]) {
				leds.On((childLeds)i);
			} else {
				leds.Off((childLeds)i);
			}
		}
	}
}

void rolloverCallback(uint nRollover)
{
	if(!multipliersTimer.isExpired()) {
		return;
	}

	incrementScore(ROLLOVER_POINTS);
	rollovers[nRollover] = true;
	Msg.ShowScore();
	if(ALL_ROLLOVERS_ON) {
		if(multiplier < MAX_MULTIPLIER) {
			multiplier++;
		}
		Msg.ShowMultiplier();
		multipliersTimer.start(MULTIPLIER_RESET_TIME, AsyncDelay::MILLIS);
	}
	showRolloverLeds();
	Sound::Play(soundNames::DING);
}

void resetRollovers()
{
	for(int i = 0; i <= 2; i++) {
		rollovers[i] = false;
	}
}

void resetHold()
{
	holdActive = false;
	digitalWrite(stopMagnet, LOW);
	holdScoreTimer.expire();
	leds.Off(childLeds::HOLD);
	stopSensorHits = 0;
}

#pragma endregion --------------------------------------------------------------

#pragma region Sensor check functions ------------------------------------------

bool checkButtons()
{
	return RIGHT_BUTTON_ON || LEFT_BUTTON_ON;
}

bool checkOrbitSensor()
{
	static bool result;
	result = false;

	Debounce::Digital(leftOrbitSensor, []() {
		incrementScore(LEFT_ORBIT_POINTS);
		if(greasyActive) {
			Sound::Play(soundNames::CLANG);	// TODO: trocar
			eobBonus += GREASY_BONUS;
		} else {
			Sound::Play(soundNames::DING);
		}
		Msg.ShowScore();
		result = true;
	}, false);

	return result;
}

bool checkRollovers()
{
	static bool result;
	result = false;

	Debounce::Digital(rollover1Sensor, []() {
		rolloverCallback(0);
		result = true;
	});

	Debounce::Digital(rollover2Sensor, []() {
		rolloverCallback(1);
		result = true;
	});

	Debounce::Digital(rollover3Sensor, []() {
		rolloverCallback(2);
		result = true;
	});

	if(ALL_ROLLOVERS_ON && multipliersTimer.isExpired()) {
		resetRollovers();
		showRolloverLeds();
		Msg.ShowScore();
	}

	return result;
}

bool checkSkillShot()
{
	static bool result;
	result = false;

	Debounce::Digital(rolloverSkillSensor, []() {
		if(skillShotActive && !skillShotTimer.isExpired()) {
			skillShotTimer.expire();
			incrementScore(SKILL_SHOT_POINTS);
			Msg.ShowScore();
			Sound::Play(soundNames::CLANG);
		} else {
			incrementScore(ROLLOVER_POINTS);
			Msg.ShowScore();
			leds.OneShot(childLeds::ROLLOVER_SKILL, NORMAL_ONESHOT);
			Sound::Play(soundNames::DING);
		}
		result = true;
	});

	if(skillShotActive) {
		if(skillShotTimer.isExpired()) {
			leds.Off(childLeds::ROLLOVER_SKILL);
			skillShotActive = false;
		}
	}

	return result;
}

bool checkStopMagnet()
{
	static bool result;
	result = false;

	if(!holdActive) {
		Debounce::Analog(holdSensor, MIN_ANALOG_THRESHOLD, HOLD_SENSOR_THRESHOLD, []() {
			Msg.ShowHoldState();
			if(stopSensorHits < HOLD_THRESHOLD - 1) {
				if(stopSensorHits == 0) {
					leds.Flash(childLeds::HOLD, NORMAL_FLASH_LEDS);
				}
				stopSensorHits++;
				incrementScore(HOLD_POINTS);
			} else {
				digitalWrite(stopMagnet, HIGH);
				holdTimer.start(HOLD_TIME, AsyncDelay::MILLIS);
				leds.On(childLeds::HOLD);
				holdActive = true;
				holdScoreTimer.start(HOLD_COUNTER_TIME, AsyncDelay::MILLIS);
				incrementScore(HOLD_ACTIVE_POINTS);
			}
			Sound::Play(soundNames::DING);
			result = true;
		});
	} else {
		if(holdTimer.isExpired()) {
			resetHold();
		} else {
			if(analogRead(holdSensor) < HOLD_SENSOR_THRESHOLD) {
				if(holdScoreTimer.isExpired()) {
					incrementScore(HOLD_ACTIVE_POINTS);
					Msg.ShowScore();
					Sound::Play(soundNames::DING);
					holdScoreTimer.repeat();
				}
			} else {
				resetHold();
			}
		}
	}

	return result;
}

bool checkSpinner()
{
	static bool result;
	result = false;

	Debounce::Read(spinnerSensor, []() {
		incrementScore(SPINNER_POINTS);
		rotateRollovers();
		showRolloverLeds();
		Msg.ShowScore();
		result = true;
	});

	return result;
}

bool checkOutlanes()
{
	static bool result;
	result = false;

	if(ON_OUTLANE) {
		Flippers::Reset();
		digitalWrite(stopMagnet, LOW);
		incrementScore(OUTLANE_POINTS);
		Sound::Play(soundNames::CLANG);	// TODO: trocar por outro som
		Msg.ShowScore();
		result = true;
	}

	return result;
}

bool checkBallLost()
{
	static bool result;
	result = false;

	if(IS_BALL_LOST) {
		Flippers::Reset();
		digitalWrite(stopMagnet, LOW);
		result = true;
	}

	return result;
}

bool checkLaunch()
{
	static bool done;
	static char *sensorName;

	done = false;
	sensorName = "(none)";

	if(analogRead(launchSensor) < LAUNCH_SENSOR_THRESHOLD) {
		sensorName = "launch sensor";
		done = true;
	} else {

		// The checks below are required because launchSensor is not 100% reliable

		if(checkOrbitSensor()) {
			done = true;
		}
		if(checkRollovers()) {
			done = true;
		}
		if(checkSkillShot()) {
			done = true;
		}
		if(checkStopMagnet()) {
			done = true;
		}
		if(checkSpinner()) {
			done = true;
		}
		if(checkOutlanes()) {
			done = true;
		}
		if(checkBallLost()) {
			done = true;
		}
	}

	if(done) {
		Serial.print("  --> GameState changed by ");
		Serial.println(sensorName);
	}

	return done;
}

#pragma endregion --------------------------------------------------------------
