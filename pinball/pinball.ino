// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Primary Arduino
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include <Wire.h>

#include "pinball.h"
#include "flippers.h"
#include "leds.h"
#include "display.h"
#include "sound.h"

#pragma region Hardware constants ----------------------------------------------

// Baud rate

#define BAUDRATE				57600

// Time constants

#define ANIMATION_TIME			250
#define DEFAULT_DEBOUNCE		200
#define DEFAULT_ONESHOT			800
#define ANALOG_DEBOUNCE			400
#define NORMAL_FLASH			200

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

bool spinnerState = false;

// Time variables

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
		Display::Stop();
		Display::Show("START");
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

	Flippers::Left();
	Flippers::Right();

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
		Sound::Play(soundNames::CLANG);
		gameState = gameStates::PLAYING;
		Serial.println("----------------------------");
		Serial.println("gameState: Playing");
	}
}

void playing()
{
	currentMs = millis();

	Flippers::Left();
	Flippers::Right();

	checkSpinner();
	checkSkillShot(false);
	checkLeftOrbit();
	checkRollovers();
	checkHold();

	if(checkOutlanes()) {
		Flippers::Reset();
		gameState = gameStates::NO_MORE_POINTS;
		Serial.println("----------------------------");
		Serial.println("gameState: No more points");
	}

	if(IS_BALL_LOST) {
		Flippers::Reset();
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
	Sound::Play(soundNames::BAMBOO);
	gameState = gameStates::BALL_NEAR_HOME;
	Serial.println("----------------------------");
	Serial.println("gameState: Ball near home");
}

void nextBall()
{
	showMultiString(ballLostMessages, NUMITEMS(ballLostMessages), &numBalls);
	Sound::Play(soundNames::BOING);
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
	Sound::Play(soundNames::WHISTLE);
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
			Sound::Play(soundNames::DING);
			if(callback) {
				callback(param);
			}
			if(led != childLeds::LIGHTS) {
				if(ledOp == outState::ONESHOT) {
					leds.OneShot(led, DEFAULT_ONESHOT);
				} else {
					leds.On(led);
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
			Sound::Play(soundNames::DING);
			do {
				Flippers::Left();
				Flippers::Right();
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
			Display::Rotate(200);
			Display::Show("EXTRA BALL ");
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
		// delay(DEFAULT_DEBOUNCE);
	}
	if(sensorScore(rightOutlaneSensor, &rightOutlaneMs, false,
		OUTLANE_POINTS, childLeds::RIGHT_OUTLANE, outState::ONESHOT, NULL, 0)) {
		hit = true;
		// HACK: Não está claro se resolve o problema
		// delay(DEFAULT_DEBOUNCE);
	}

	return hit;
}

bool checkHold()
{
	bool hit = false;

	if(hold && currentMs > stopMagMs + RELEASE_TIME) {
		if(analogScore(holdSensor, &holdSensorMs, MIN_ANALOG_THRESHOLD,
			HOLD_SENSOR_THRESHOLD, HOLD_POINTS)) {
			leds.OneShot(childLeds::HOLD, DEFAULT_ONESHOT);
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
				leds.On(childLeds::HOLD);
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
	Display::Clear();
	Display::Test();
	delay(DISPLAY_TEST_TIME);
}

void startAnimation()
{
	Display::Rotate(DISPLAY_ROTATE_TIME);
	Display::Show("oooooo*oooooo******o******");
	//            1234567890123456789012345678901
}

void newBallAnimation()
{
	Display::Clear();
	Display::Rotate(120);
	Display::Show("_-@-_-@-");
}

void displayBall()
{
	Display::Stop();
	strcpy(displayBuffer, "BALL  ");
	displayBuffer[5] = '0' + currentBall;
	flashMessage(displayBuffer);
	Display::Hold(1000);
}

void displayScore()
{
	Display::Stop();
	Display::U2s(displayBuffer, playerScore);
	Display::Show(displayBuffer);
}

void displayMultiplier()
{
	strcpy(displayBuffer, "MULT ");
	displayBuffer[5] = '0' + multiplier;
	Display::Show(displayBuffer);
	Display::Hold(1000);
	Serial.print("*** Multiplier: ");
	Serial.println(multiplier);
}

void displayHold()
{
	strcpy(displayBuffer, "HOLD  ");
	displayBuffer[5] = '1' + stopSensorHits;
	Display::Show(displayBuffer);
	Display::Hold(700);
	Serial.print("Hold: ");
	Serial.println(stopSensorHits);
}

void flashScore(uint speed)
{
	Display::U2s(displayBuffer, playerScore);
	flashMessage(displayBuffer, speed);
	// display.Flash(speed);
	// display.Show(displayBuffer);
}

void flashMessage(char *msg)
{
	flashMessage(msg, SLOW_FLASH_TIME);
}

void flashMessage(char *msg, uint speed)
{
	Display::Stop();
	Display::Flash(speed);
	Display::Show(msg);
}

void showMultiString(const char *msg[], uint nItems, uint *index)
{
	Display::Show((char *)msg[*index % nItems]);
	(*index)++;
}

#pragma endregion --------------------------------------------------------------

#pragma region Output and actuator functions -----------------------------------

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
