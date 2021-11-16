// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Primary Arduino
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include <Arduino.h>
#include <FtModules.h>
#include <Wire.h>

#include "Simpletypes.h"
#include "leds.h"
#include "child.h"
#include "display.h"
#include "sound.h"

#pragma region Hardware constants ----------------------------------------------

// Baud rate

#define BAUDRATE				57600

// Parameters for L298N and 19.5 VDC power supply

#define MAX_POWER_MS			50
#define HOLD_PWM				40

// Analog sensor thresholds

// '10' as minimum prevents false readings when 19 V is off
#define MIN_ANALOG_THRESHOLD	10
#define LAUNCH_SENSOR_THRESHOLD 600
#define HOLD_SENSOR_THRESHOLD	600

// Time constants

#define ANIMATION_TIME			250
#define DEFAULT_DEBOUNCE		200
#define DEFAULT_ONESHOT			500
#define ANALOG_DEBOUNCE			400
#define NORMAL_FLASH			200

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

// Inputs and outputs

const byte pullUps[] = {
	leftButton, rightButton, ballLostSensor,
	rolloverSkillSensor, rollover1Sensor, rollover2Sensor, rollover3Sensor,
	feederHomeSensor, ballNearHomeSensor,
	leftOrbitSensor
};

const byte inputs[] = {
	leftOutlaneSensor, rightOutlaneSensor, spinnerSensor
};

const byte outputs[] = {leftFlipper, rightFlipper, stopMagnet};

#pragma endregion --------------------------------------------------------------

#pragma region Macros ----------------------------------------------------------

// Sensor macros

#define LEFT_BUTTON_ON	 (!digitalRead(leftButton))
#define LEFT_BUTTON_OFF	 (digitalRead(leftButton))
#define RIGHT_BUTTON_ON	 (!digitalRead(rightButton))
#define RIGHT_BUTTON_OFF (digitalRead(rightButton))

#define IS_BALL_LOST	 (digitalRead(ballLostSensor))
#define IS_SPINNER_UP	 (!digitalRead(spinnerSensor))

#pragma endregion --------------------------------------------------------------

#pragma region Game constants --------------------------------------------------

const byte ballsPerGame = 3;
const byte maxBallSaves = 2;
const byte maxMultiplier = 8;
const byte holdThreshold = 3; // Number of stop sensor hits to activate hold

// Time constants

#define BALLSAVER_TIME			3000
#define SKILL_SHOT_TIME			1000
#define ENDGAME_TIME			1500
#define ENDSCORE_TIME			1500
#define BALL_NEAR_HOME_TIME		400
#define DISPLAY_TEST_TIME		200
#define DISPLAY_ROTATE_TIME		200
#define FEEDBALL_TIME			100
#define HOLD_TIME				5000
#define RELEASE_TIME			500		// Must be enough to let the ball go
#define MULTIPLIER_RESET_TIME	1500
#define MULTIPLIER_WAIT_TIME	300
#define END_FLASH_TIME			250
#define SLOW_FLASH_TIME			600
#define BALL_LOST_TIMEOUT		1700

// Points

#define LEFT_ORBIT_POINTS		50
#define STOPMAG_POINTS			1000
#define HOLD_POINTS				STOPMAG_POINTS
#define BALL_LOST_POINTS		250
#define OUTLANE_POINTS			800
#define SPINNER_POINTS			25
#define HIGH_SPINNER_POINTS		75
#define ROLLOVER_POINTS			50
#define SKILL_SHOT_POINTS		1500

// Strings

const char *ballLostMessages[] = {" OOPS", "UH-OH", " OUT"};
const char *endGameMessages[] = {"  BYE", " CIAO", "Ended", "ADIOS", "  End", " LOST"};
const char *replayMessages[] = {"REPLAY", "AGAIN", "LUCKY", "SAVED"};

#pragma endregion --------------------------------------------------------------

#pragma region Enums -----------------------------------------------------------

// Flipper states

enum class flipperStates
{
	IDLE = 0,
	STROKE,
	HOLD,
	HOLDING,
};

// Game states

enum class gameStates
{
	GAME_START = 1,
	LAUNCHED,
	PLAYING,
	SAVE_BALL,
	NEXT_BALL,
	BALL_NEAR_HOME,
	NO_MORE_POINTS,
	BALL_LOST,
	GAME_OVER,
};

enum class extraBallStates
{
	NOEXTRABALL = 0,
	READY,
	ACTIVE,
};

#pragma endregion --------------------------------------------------------------

#pragma region Hardware variables ----------------------------------------------

flipperStates leftFlipperState = flipperStates::IDLE;
flipperStates rightFlipperState = flipperStates::IDLE;

bool spinnerState = false;

// Time variables

ulong leftButtonPreviousMs;
ulong rightPreviousMs;
ulong leftOrbitMs = 0;
ulong stopMagMs = 0;
ulong spinnerMs = 0;
ulong rollover1Ms = 0;
ulong rollover2Ms = 0;
ulong rollover3Ms = 0;
ulong rolloverSkillMs = ULONG_MAX;
ulong leftOutlaneMs = 0;
ulong rightOutlaneMs = 0;
ulong holdSensorMs = 0;
ulong ballMs = 0;
ulong multiplierMs = ULONG_MAX;

unsigned long currentMs;

char displayBuffer[DISPLAYCHARS];

FtModules::I2C i2c;
Leds leds;

#pragma endregion --------------------------------------------------------------

#pragma region Game variables --------------------------------------------------

gameStates gameState = gameStates::GAME_START;
byte currentBall = 1;
byte multiplier = 1;
byte ballSaves = 0;
byte stopSensorHits = 0;
extraBallStates extraBallState = extraBallStates::NOEXTRABALL;

uint numGames = 0;
uint numBalls = 0;
uint numSaves = 0;

bool rollovers[3] = {false, false, false};
bool skillShotActive = false;
bool hold = false;

ulong playerScore = 0;
ulong lastScore = 0;

#pragma endregion --------------------------------------------------------------

#pragma region Setup -----------------------------------------------------------

void setup()
{
	// Initialize

	Serial.begin(BAUDRATE);
	Wire.begin();

	leds.Init();
	setPinModes();
	resetChild();
	initDisplay();

	Serial.println("");
	Serial.println("Started");
	preStartGame();
	Serial.println("----------------------------");
	Serial.println("gameState: Ready");
}

void setPinModes()
{
	// Set up pin modes

	for(int i = 0; i < sizeof pullUps; i++) {
		pinMode(pullUps[i], INPUT_PULLUP);
	}

	for(int i = 0; i < sizeof inputs; i++) {
		pinMode(inputs[i], INPUT);
	}

	for(int i = 0; i < sizeof outputs; i++) {
		pinMode(outputs[i], OUTPUT);
		digitalWrite(outputs[i], (int)outState::OFF);
	}
}

void resetChild()
{
	i2c.Cmd(CHILD_ADDRESS, (int)childCommands::RESET);
}

#pragma endregion --------------------------------------------------------------

#pragma region Main loop -------------------------------------------------------

void loop()
{
	gameLoop();

	// leds.waitAnimation(checkButtons);
	// testAllLeds();
	// leds.redFlashes(100); delay(2000); leds.allOff(); delay(2000);
	// testSounds();
	// testInputs();
	// analogSensorTestLoop();
}

void gameLoop()
{
	switch((gameStates)gameState) {

		case gameStates::GAME_START:
			gameStart();
			break;

		case gameStates::LAUNCHED:
			launched();
			break;

		case gameStates::PLAYING:
			playing();
			break;

		case gameStates::BALL_LOST:
			ballLost();
			break;

		case gameStates::NO_MORE_POINTS:
			noMorePoints();
			break;

		case gameStates::SAVE_BALL:
			saveBall();
			break;

		case gameStates::NEXT_BALL:
			nextBall();
			break;

		case gameStates::BALL_NEAR_HOME:
			ballNearHome();
			break;

		case gameStates::GAME_OVER:
			gameOver();
			break;
	}
}

#pragma endregion --------------------------------------------------------------

#pragma region State machine functions -----------------------------------------

void gameStart()
{
	if(checkButtons()) {
		DisplayStop();
		DisplayShow("START");
		ballStart(true);
	} else {
		leds.waitAnimation(checkButtons);
	}
}

// Executed before each ball and before each game
void ballStart(bool resetGame)
{
	feedBall();
	startBall();
	skillShotActive = false;
	hold = false;
	// leds.Off(childLeds::HOLD);
	stopSensorHits = 0;
	if(resetGame) {
		multiplier = 1;
		currentBall = 1;
		playerScore = 0;
	}
	Serial.print("Ball #");
	Serial.println(currentBall);
	openDoor();
	if(resetGame) {
		leds.allOff(false);
	}
	leds.Off(childLeds::LEFT_OUTLANE);
	leds.Off(childLeds::RIGHT_OUTLANE);
	leds.Flash(childLeds::ROLLOVER_SKILL, NORMAL_FLASH);
	displayBall();
	gameState = gameStates::LAUNCHED;
	Serial.println("----------------------------");
	Serial.println("gameState: Launched");
}

void launched()
{
	currentMs = millis();
	bool launch = false;

	driveLeftFlipper();
	driveRightFlipper();

	if(analogRead(launchSensor) < LAUNCH_SENSOR_THRESHOLD) {
		Serial.println("Launched by launch sensor");
		launch = true;
	}

	// The checks below are required because launchSensor is not 100% reliable

	if(checkSpinner()) {
		Serial.println("Launched by spinner");
		launch = true;
	}

	if(checkSkillShot(true)) {
		Serial.println("Launched by skill shot");
		launch = true;
	}

	if(checkLeftOrbit()) {
		Serial.println("Launched by left orbit");
		launch = true;
	}

	if(checkRollovers()) {
		Serial.println("Launched by rollover");
		launch = true;
	}

	if(checkHold()) {
		Serial.println("Launched by hold sensor");
		launch = true;
	}

	// Check outlanes for ball out

	if(checkOutlanes()) {
		gameState = gameStates::NO_MORE_POINTS;
		Serial.println("----------------------------");
		Serial.println("gameState: No more points");
	}

	if(IS_BALL_LOST) {
		gameState = gameStates::SAVE_BALL;
		Serial.println("----------------------------");
		Serial.println("gameState: Save ball");
	}

	if(launch) {
		// Serial.print("Launched ball #");
		// Serial.println(currentBall);
		closeDoor();
		lastScore = playerScore;
		ballMs = millis();
		rolloverSkillMs = ballMs;
		skillShotActive = true;
		displayScore();
		PlaySound(soundNames::CLANG);
		gameState = gameStates::PLAYING;
		Serial.println("----------------------------");
		Serial.println("gameState: Playing");
	}
}

void playing()
{
	currentMs = millis();

	driveLeftFlipper();
	driveRightFlipper();

	checkSpinner();
	checkSkillShot(false);
	checkLeftOrbit();
	checkRollovers();
	checkHold();

	if(checkOutlanes()) {
		resetFlippers();
		gameState = gameStates::NO_MORE_POINTS;
		Serial.println("----------------------------");
		Serial.println("gameState: No more points");
	}

	if(IS_BALL_LOST) {
		resetFlippers();
		gameState = gameStates::BALL_LOST;
		Serial.println("----------------------------");
		Serial.println("gameState: Ball lost");
	}
}

void ballLost()
{
	if(millis() - ballMs <= BALLSAVER_TIME && ballSaves < maxBallSaves) {
		gameState = gameStates::SAVE_BALL;
		Serial.println("----------------------------");
		Serial.println("gameState: Save ball");
	} else {
		playerScore += BALL_LOST_POINTS;
		displayScore();
		if(currentBall < ballsPerGame) {
			gameState = gameStates::NEXT_BALL;
			Serial.println("----------------------------");
			Serial.println("gameState: Next ball");
		} else {
			gameState = gameStates::GAME_OVER;
			Serial.println("-----*****-----*****-----*****-----");
			Serial.println("gameState: Game over");
			Serial.println("");
		}
	}
}

void noMorePoints()
{
	if(IS_BALL_LOST) {
		gameState = gameStates::BALL_LOST;
		Serial.println("----------------------------");
		Serial.println("gameState: Ball lost");
	}
}

void saveBall()
{
	ballSaves++;
	playerScore = lastScore;
	// Serial.println("Saved");
	showMultiString(replayMessages, NUMITEMS(replayMessages), &numSaves);
	PlaySound(soundNames::BAMBOO);
	gameState = gameStates::BALL_NEAR_HOME;
	Serial.println("----------------------------");
	Serial.println("gameState: Ball near home");
}

void nextBall()
{
	showMultiString(ballLostMessages, NUMITEMS(ballLostMessages), &numBalls);
	PlaySound(soundNames::BOING);
	currentBall++;
	ballSaves = 0;
	extraBallState = extraBallStates::NOEXTRABALL;
	gameState = gameStates::BALL_NEAR_HOME;
	Serial.println("----------------------------");
	Serial.println("gameState: Ball near home");
}

void ballNearHome()
{
	ulong ms = millis();
	while((millis() < ms + BALL_LOST_TIMEOUT) && digitalRead(ballNearHomeSensor)) {
		//
	}
	newBallAnimation();
	delay(BALL_NEAR_HOME_TIME);
	ballStart(false);
}

void gameOver()
{
	// Serial.println("Game over -----------------------------------");
	showMultiString(endGameMessages, NUMITEMS(endGameMessages), &numGames);
	PlaySound(soundNames::WHISTLE);
	delay(ENDGAME_TIME);
	flashScore(END_FLASH_TIME);
	delay(ENDSCORE_TIME);
	preStartGame();
	gameState = gameStates::GAME_START;
	Serial.println("----------------------------");
	Serial.println("gameState: Game start");
}

#pragma endregion --------------------------------------------------------------

#pragma region Game functions --------------------------------------------------

void preStartGame()
{
	startBall();
	startAnimation();
	closeDoor();
}

void startBall()
{
	leds.On(childLeds::LIGHTS);

	for(int i = 0; i <= 2; i++) {
		rollovers[i] = false;
	}

	for(int i = 0; i <= 7; i++) {
		leds.Off((childLeds)i);
	}
}

void rotateRollovers(bool right)
{
	if(right) {
		bool s = rollovers[2];
		rollovers[2] = rollovers[1];
		rollovers[1] = rollovers[0];
		rollovers[0] = s;
	} else {
		bool s = rollovers[0];
		rollovers[0] = rollovers[1];
		rollovers[1] = rollovers[2];
		rollovers[2] = s;
	}

	for(int i = 0; i <= 2; i++) {
		if(rollovers[i]) {
			leds.On((childLeds)i);
		} else {
			leds.Off((childLeds)i);
		}
	}
}

#pragma endregion --------------------------------------------------------------

#pragma region Sensor functions ------------------------------------------------

bool sensorScore(uint port, ulong *msVar, bool positiveLogic,
	ulong points, childLeds led, outState ledOp, void (*callback)(uint), uint param)
{
	if(currentMs > *msVar + DEFAULT_DEBOUNCE) {
		if((positiveLogic && digitalRead(port)) || (!positiveLogic && !digitalRead(port))) {
			*msVar = currentMs;
			playerScore += points * multiplier;
			displayScore();
			PlaySound(soundNames::DING);
			if(callback) {
				callback(param);
			}
			if(led != childLeds::LIGHTS) {
				if(ledOp == outState::ONESHOT) {
					leds.OneShot(led, DEFAULT_ONESHOT, colorIndex::WHITE);
				} else {
					leds.On(led, colorIndex::WHITE);
				}
			}
			if(positiveLogic) {
				while(digitalRead(port)) {
					// Serial.print("+ port "); Serial.println(port);
				};
			} else {
				while(!digitalRead(port)) {
					// Serial.print("- port "); Serial.println(port);
				};
			}
			return true;
		}
	}
	return false;
}

bool analogScore(uint port, ulong *msVar, uint min, uint max, ulong points)
{
	if(currentMs > *msVar + ANALOG_DEBOUNCE) {
		int value = analogRead(port);

		if(value >= min && value < max) {
			*msVar = millis();
			playerScore += points * multiplier;
			PlaySound(soundNames::DING);
			do {
				driveLeftFlipper();
				driveRightFlipper();
				value = analogRead(port);
			} while ((value >= min && value < max) && (millis() < *msVar + 2 * ANALOG_DEBOUNCE));
			return true;
		}
	}
	return false;
}

bool checkSpinner()
{
	bool hit = false;

	if(spinnerState) {
		if(!IS_SPINNER_UP) {
			spinnerState = false;
			playerScore += SPINNER_POINTS;
			rotateRollovers(false);
			displayScore();
			hit = true;
		}
	} else {
		if(IS_SPINNER_UP) {
			spinnerState = true;
		}
	}
	return hit;
}

bool checkLeftOrbit()
{
	return sensorScore(leftOrbitSensor, &leftOrbitMs, true,
		LEFT_ORBIT_POINTS, childLeds::LEFT_ORBIT, outState::ONESHOT, NULL, 0);
}

bool checkSkillShot(bool override)
{
	if(skillShotActive) {
		if(currentMs > rolloverSkillMs + SKILL_SHOT_TIME) {
			skillShotActive = false;
			leds.Off(childLeds::ROLLOVER_SKILL);
			rolloverSkillMs = ULONG_MAX;
		}
	}

	bool result = sensorScore(rolloverSkillSensor, &rolloverSkillMs, false,
		override || skillShotActive ? SKILL_SHOT_POINTS : ROLLOVER_POINTS,
		childLeds::ROLLOVER_SKILL, outState::ONESHOT, NULL, 0);

	if(result && !skillShotActive) {
		if(extraBallState == extraBallStates::NOEXTRABALL) {
			extraBallState = extraBallStates::READY;
			leds.On(childLeds::ROLLOVER_SKILL);
		} else if(extraBallState == extraBallStates::READY) {
			DisplayRotate(200);
			DisplayShow("EXTRA BALL ");
			Serial.println("Extra ball");
			leds.Flash(childLeds::LEFT_OUTLANE, NORMAL_FLASH);
			leds.Flash(childLeds::RIGHT_OUTLANE, NORMAL_FLASH);
			extraBallState = extraBallStates::ACTIVE;
		}
	}

	return result;
}

void checkMultiplier(uint nRollover)
{
	rollovers[nRollover] = true;

	if(rollovers[0] && rollovers[1] && rollovers[2] &&
		(multiplierMs == ULONG_MAX || currentMs > multiplierMs + MULTIPLIER_RESET_TIME)) {
		if(multiplier < maxMultiplier) {
			multiplier++;
		}
		displayMultiplier();
		// strcpy(displayBuffer, "MULT ");
		// displayBuffer[5] = '0' + multiplier;
		// display.Show(displayBuffer, 100);
		// Serial.print("Multiplier: ");
		// Serial.println(multiplier);
		multiplierMs = currentMs;
	}
}

bool checkRollovers()
{
	bool hit = false;

	if(sensorScore(rollover1Sensor, &rollover1Ms, false,
		ROLLOVER_POINTS, childLeds::ROLLOVER1, outState::ON, &checkMultiplier, 0)) {
		hit = true;
	}
	if(sensorScore(rollover2Sensor, &rollover2Ms, false,
		ROLLOVER_POINTS, childLeds::ROLLOVER2, outState::ON, &checkMultiplier, 1)) {
		hit = true;
	}
	if(sensorScore(rollover3Sensor, &rollover3Ms, false,
		ROLLOVER_POINTS, childLeds::ROLLOVER3, outState::ON, &checkMultiplier, 2)) {
		hit = true;
	}

	// Resets rollover LEDs

	if(rollovers[0] && rollovers[1] && rollovers[2]) {
		if(currentMs > multiplierMs + MULTIPLIER_RESET_TIME) {
			for(int i = 0; i <= 2; i++) {
				rollovers[i] = false;
				leds.Off((childLeds)i);
			}
			// Serial.println("Reset rollovers");
			// displayScore();
			multiplierMs = ULONG_MAX;
		}
	}

	return hit;
}

bool checkOutlanes()
{
	bool hit = false;

	if(sensorScore(leftOutlaneSensor, &leftOutlaneMs, false,
		OUTLANE_POINTS, childLeds::LEFT_OUTLANE, outState::ONESHOT, NULL, 0)) {
		hit = true;
		// HACK: Não está claro se resolve o problema
		delay(DEFAULT_DEBOUNCE);
	}
	if(sensorScore(rightOutlaneSensor, &rightOutlaneMs, false,
		OUTLANE_POINTS, childLeds::RIGHT_OUTLANE, outState::ONESHOT, NULL, 0)) {
		hit = true;
		// HACK: Não está claro se resolve o problema
		delay(DEFAULT_DEBOUNCE);
	}

	return hit;
}

bool checkHold()
{
	bool hit = false;

	if(hold && currentMs > stopMagMs + RELEASE_TIME) {
		if(analogScore(holdSensor, &holdSensorMs, MIN_ANALOG_THRESHOLD,
			HOLD_SENSOR_THRESHOLD, HOLD_POINTS)) {
			leds.OneShot(childLeds::HOLD, DEFAULT_ONESHOT, colorIndex::RED);
			hit = true;
			displayScore();
		}
	} else {
		if(analogScore(holdSensor, &holdSensorMs, MIN_ANALOG_THRESHOLD,
			HOLD_SENSOR_THRESHOLD, STOPMAG_POINTS)) {

			hit = true;
			// Serial.print("------------ Score: ");
			// Serial.println(playerScore);

			displayHold();
			// strcpy(displayBuffer, "HOLD  ");
			// displayBuffer[5] = '1' + stopSensorHits;
			// display.Show(displayBuffer);

			if(stopSensorHits + 1 == holdThreshold) {
				hold = true;
				leds.On(childLeds::HOLD, colorIndex::WHITE);
				digitalWrite(stopMagnet, HIGH);
				stopMagMs = currentMs;
			} else {
				if(!hold) {
					stopSensorHits++;
				}
			}
		}
	}
	// if(hold && (currentMs > stopMagMs + RELEASE_TIME)) {
	// 	digitalWrite(stopMagnet, HIGH);
	// }

	if(hold && (currentMs > stopMagMs + RELEASE_TIME + HOLD_TIME)) {
		hold = false;
		stopSensorHits = 0;
		stopMagMs = ULONG_MAX;
		digitalWrite(stopMagnet, LOW);
		delay(RELEASE_TIME);
		leds.Off(childLeds::HOLD);
	}

	return hit;
}

bool checkButtons()
{
	return RIGHT_BUTTON_ON || LEFT_BUTTON_ON;
}

#pragma endregion --------------------------------------------------------------

#pragma region Display functions -----------------------------------------------

void initDisplay()
{
	DisplayClear();
	DisplayTest();
	delay(DISPLAY_TEST_TIME);
}

void startAnimation()
{
	DisplayRotate(DISPLAY_ROTATE_TIME);
	DisplayShow("oooooo*oooooo******o******");
	//            1234567890123456789012345678901
}

void newBallAnimation()
{
	DisplayClear();
	DisplayRotate(120);
	DisplayShow("_-@-_-@-");
}

void displayBall()
{
	DisplayStop();
	strcpy(displayBuffer, "BALL  ");
	displayBuffer[5] = '0' + currentBall;
	flashMessage(displayBuffer);
	DisplayHold(1000);
}

void displayScore()
{
	DisplayStop();
	u2s(playerScore);
	DisplayShow(displayBuffer);
}

void displayMultiplier()
{
	strcpy(displayBuffer, "MULT ");
	displayBuffer[5] = '0' + multiplier;
	DisplayShow(displayBuffer);
	DisplayHold(1000);
	Serial.print("*** Multiplier: ");
	Serial.println(multiplier);
}

void displayHold()
{
	strcpy(displayBuffer, "HOLD  ");
	displayBuffer[5] = '1' + stopSensorHits;
	DisplayShow(displayBuffer);
	DisplayHold(700);
	Serial.print("Hold: ");
	Serial.println(stopSensorHits);
}

void flashScore(uint speed)
{
	u2s(playerScore);
	flashMessage(displayBuffer, speed);
	// display.Flash(speed);
	// display.Show(displayBuffer);
}

void u2s(unsigned long value)
{
	// https://forum.arduino.cc/t/right-justify/93157/10

	for(int i = DISPLAYCHARS - 1; i >= 0; i--) {
		displayBuffer[i] = (value == 0 && i != DISPLAYCHARS - 1) ? ' ' : '0' + value % 10;
		value /= 10;
	}
}

void flashMessage(char *msg)
{
	flashMessage(msg, SLOW_FLASH_TIME);
}

void flashMessage(char *msg, uint speed)
{
	DisplayStop();
	DisplayFlash(speed);
	DisplayShow(msg);
}

void showMultiString(const char *msg[], uint nItems, uint *index)
{
	DisplayShow((char *)msg[*index % nItems]);
	(*index)++;
}

#pragma endregion --------------------------------------------------------------

#pragma region Output and actuator functions -----------------------------------

void driveLeftFlipper()
{
	if(leftFlipperState == flipperStates::IDLE) {
		if(LEFT_BUTTON_ON) {
			digitalWrite(leftFlipper, HIGH);
			leftButtonPreviousMs = currentMs;
			leftFlipperState = flipperStates::STROKE;
		}
	} else if(leftFlipperState == flipperStates::STROKE) {
		if(LEFT_BUTTON_OFF) {
			digitalWrite(leftFlipper, LOW);
			leftFlipperState = flipperStates::IDLE;
		} else if(currentMs - leftButtonPreviousMs >= MAX_POWER_MS) {
			leftFlipperState = flipperStates::HOLD;
		}
	} else if(leftFlipperState == flipperStates::HOLD) {
		if(LEFT_BUTTON_ON) {
			analogWrite(leftFlipper, HOLD_PWM);
		}
		leftFlipperState = flipperStates::HOLDING;
	} else if(leftFlipperState == flipperStates::HOLDING) {
		if(LEFT_BUTTON_OFF) {
			digitalWrite(leftFlipper, LOW);
			leftFlipperState = flipperStates::IDLE;
		}
	}
}

void driveRightFlipper()
{
	if(rightFlipperState == flipperStates::IDLE) {
		if(RIGHT_BUTTON_ON) {
			digitalWrite(rightFlipper, HIGH);
			rightPreviousMs = currentMs;
			rightFlipperState = flipperStates::STROKE;
		}
	} else if(rightFlipperState == flipperStates::STROKE) {
		if(RIGHT_BUTTON_OFF) {
			digitalWrite(rightFlipper, LOW);
			rightFlipperState = flipperStates::IDLE;
		} else if(currentMs - rightPreviousMs >= MAX_POWER_MS) {
			rightFlipperState = flipperStates::HOLD;
		}
	} else if(rightFlipperState == flipperStates::HOLD) {
		if(RIGHT_BUTTON_ON) {
			analogWrite(rightFlipper, HOLD_PWM);
		}
		rightFlipperState = flipperStates::HOLDING;
	} else if(rightFlipperState == flipperStates::HOLDING) {
		if(RIGHT_BUTTON_OFF) {
			digitalWrite(rightFlipper, LOW);
			rightFlipperState = flipperStates::IDLE;
		}
	}
}

void resetFlippers()
{
	digitalWrite(leftFlipper, LOW);
	digitalWrite(rightFlipper, LOW);
}

void feedBall()
{
	i2c.Cmd(CHILD_ADDRESS, (int)childCommands::MOTOR, HIGH);
	delay(FEEDBALL_TIME);
	while(digitalRead(feederHomeSensor)) {
		delay(5);
	}
	i2c.Cmd(CHILD_ADDRESS, (int)childCommands::MOTOR, LOW);
}

void closeDoor()
{
	i2c.Cmd(CHILD_ADDRESS, (int)childCommands::SERVO, (int)servoCmd::CLOSE);
	delay(SERVO_TIMER);
}

void openDoor()
{
	i2c.Cmd(CHILD_ADDRESS, (int)childCommands::SERVO, (int)servoCmd::OPEN);
	delay(SERVO_TIMER);
}

#pragma endregion --------------------------------------------------------------

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

void testLeds()
{
	leds.Flash(childLeds::LEFT_ORBIT, 100, colorIndex::ORANGE);
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
	DisplayStop();
	leds.Off((childLeds)cLed);
	cLed = cLed == 8 ? 0 : cLed + 1;
	if(cLed == 8) {
		cCol = cCol == 9 ? 1 : cCol + 1;
	}
	leds.On((childLeds)cLed, (colorIndex)cCol);
	delay(500);
	DisplayClear();
	u2s(cLed);
	DisplayShow(displayBuffer);
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
		PlaySound(i);
		Serial.print("Sound #");
		Serial.println(i);
		DisplayStop();
		u2s(i);
		DisplayShow(displayBuffer);
		leds.On((childLeds)(i - 1));
		delay(2000);
		leds.Off((childLeds)(i - 1));
	}
}

#pragma endregion --------------------------------------------------------------
