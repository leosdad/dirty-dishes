// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Primary Arduino
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include <Wire.h>
#include <AsyncDelay.h>

#include "pinball.h"

#include "debounce.h"
#include "flippers.h"
#include "game.h"
#include "general.h"
#include "leds.h"
#include "messages.h"
#include "motor.h"
#include "sensors.h"
#include "servo.h"
#include "sound.h"
#include "tests.h"

#pragma region Hardware constants ----------------------------------------------

// Baud rate

#define BAUDRATE				57600

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

#pragma region Enums -----------------------------------------------------------

enum class extraBallStates
{
	NOEXTRABALL = 0,
	READY,
	ACTIVE,
};

#pragma endregion --------------------------------------------------------------

#pragma region Hardware variables ----------------------------------------------

Leds leds;
Messages Msg;
Motor motor;
Servo servo;

// Time variables

AsyncDelay ballSaveTimer;
AsyncDelay servoTimer;

extern AsyncDelay skillShotTimer;

#pragma endregion --------------------------------------------------------------

#pragma region Game variables --------------------------------------------------

gameStates gameState = (gameStates)-1;
gameStates lastGameState = (gameStates)-2;
byte currentBall = 1;
byte multiplier = 1;
byte ballSaves = 0;
byte stopSensorHits = 0;
extraBallStates extraBallState = extraBallStates::NOEXTRABALL;

bool skillShotActive = false;
bool holdActive = false;
bool greasyActive = false;

ulong playerScore = 0;
ulong lastScore = 0;
ulong greasyScore = 0;
ulong eobBonus = 0;

#pragma endregion --------------------------------------------------------------

#pragma region Setup -----------------------------------------------------------

void setup()
{
	// Initialize

	Serial.begin(BAUDRATE);
	Wire.begin();

	setPinModes();
	General::Reset();
	Msg.Init();

	preStartGame();
	setGameState(gameStates::GAME_START);
}

void setPinModes()
{
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

#pragma endregion --------------------------------------------------------------

#pragma region Main loop -------------------------------------------------------

void loop()
{
	gameLoop();

	// Tests::Leds();
	// Tests::Sounds();
	// Tests::Inputs();
	// Tests::AnalogSensors();
	// Tests::Servo();
}

void gameLoop()
{
	switch((gameStates)gameState) {

		case gameStates::GAME_START:
			gameStart();
			break;

		case gameStates::BALL_START:
			ballStart();
			break;

		case gameStates::LAUNCHING:
			launching();
			break;

		case gameStates::PLAYING:
			playing();
			break;

		case gameStates::NO_MORE_POINTS:
			noMorePoints();
			break;

		case gameStates::BALL_LOST:
			ballLost();
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
		Msg.Show("START");
		currentBall = 1;
		multiplier = 1;
		playerScore = 0;
		lastScore = 0;
		leds.On(childLeds::LIGHTS);
		leds.allOff(false);
		setGameState(gameStates::BALL_START);
	} else {
		leds.waitAnimation();
	}
}

void ballStart()
{
	motor.FeedBall();
	resetLeds();
	skillShotActive = false;
	holdActive = false;
	stopSensorHits = 0;
	greasyScore = 0;
	eobBonus = 0;
	greasyActive = false;
	resetRollovers();
	servo.OpenDoor();
	servoTimer.start(SERVO_TIMER, AsyncDelay::MILLIS);
	leds.Off(childLeds::LEFT_OUTLANE);
	leds.Off(childLeds::RIGHT_OUTLANE);
	leds.Flash(childLeds::ROLLOVER_SKILL, NORMAL_FLASH_LEDS);
	// Serial.print("Ball #");
	// Serial.println(currentBall);
	Msg.ShowBall();

	// Wait for servo door to open before changing state
	while(!servoTimer.isExpired()) {
		Flippers::Left();
		Flippers::Right();
	};
	setGameState(gameStates::LAUNCHING);
}

void launching()
{
	Flippers::Left();
	Flippers::Right();

	if(checkLaunch()) {
		skillShotTimer.start(SKILL_SHOT_TIME, AsyncDelay::MILLIS);
		ballSaveTimer.start(BALL_SAVER_TIME, AsyncDelay::MILLIS);
		servo.CloseDoor();
		servoTimer.start(SERVO_TIMER, AsyncDelay::MILLIS);
		lastScore = playerScore;
		skillShotActive = true;
		Msg.ShowScore();
		Sound::Play(soundNames::FAUCET);

		// Wait for servo door to close before changing state
		while(!servoTimer.isExpired()) {
			Flippers::Left();
			Flippers::Right();
		};
		setGameState(gameStates::PLAYING);
	}
}

void playing()
{
	Flippers::Left();
	Flippers::Right();

	checkOrbitSensor();
	checkRollovers();
	checkSkillShot();
	checkStopMagnet();
	checkSpinner();
	if(checkOutlanes()) {
		setGameState(gameStates::NO_MORE_POINTS);
	}
	if(checkBallLost()) {
		setGameState(gameStates::BALL_LOST);
	}
}

void noMorePoints()
{
	resetLeds();
	if(IS_BALL_LOST) {
		setGameState(gameStates::BALL_LOST);
	}
}

void ballLost()
{
	resetLeds();

	if(!ballSaveTimer.isExpired() && ballSaves < MAX_BALL_SAVES) {
		setGameState(gameStates::SAVE_BALL);
	} else {
		incrementScore(BALL_LOST_POINTS);
		Msg.ShowScore();

		if(currentBall < BALLS_PER_GAME) {
			setGameState(gameStates::NEXT_BALL);
		} else {
			setGameState(gameStates::GAME_OVER);
			Serial.println("-----*****-----*****-----*****-----");
			Serial.println();
		}
	}
}

void saveBall()
{
	ballSaves++;
	playerScore = lastScore;
	Msg.ShowReplay();
	Sound::Play(soundNames::SHAKE);
	setGameState(gameStates::BALL_NEAR_HOME);
}

void nextBall()
{
	Msg.ShowBallLost();
	Sound::Play(soundNames::GLUG);
	delay(DEFAULT_DISPLAY_TIME);
	showBallScore(false);
	currentBall++;
	ballSaves = 0;
	extraBallState = extraBallStates::NOEXTRABALL;
	setGameState(gameStates::BALL_NEAR_HOME);
}

void ballNearHome()
{
	delay(BALL_LOST_TIMEOUT);
	Msg.Rotate("_-@-_-@-");
	delay(BALL_NEAR_HOME_TIME);
	setGameState(gameStates::BALL_START);
}

void gameOver()
{
	Msg.ShowEndGame();
	Sound::Play(soundNames::CABINET);
	delay(DEFAULT_DISPLAY_TIME);
	showBallScore(true);
	preStartGame();
	setGameState(gameStates::GAME_START);
}

#pragma endregion --------------------------------------------------------------

#pragma region Auxiliary functions ---------------------------------------------

void incrementScore(ulong points)
{
	playerScore += points * multiplier;
	greasyScore += points * multiplier;

	if(!greasyActive && greasyScore >= GREASY_SCORE) {
		greasyActive = true;
		leds.Flash(childLeds::LEFT_ORBIT, NORMAL_FLASH_LEDS);
	}
}

void resetLeds()
{
	leds.On(childLeds::LIGHTS);

	for(int i = 0; i <= 7; i++) {
		leds.Off((childLeds)i);
	}
}

void setGameState(gameStates state)
{
	gameState = state;
	if(lastGameState != state) {
		Tests::GameState(state);			// Uncomment this line for debug
		lastGameState = state;
	}
}

void preStartGame()
{
	resetLeds();
	Msg.Rotate("oooooo*oooooo******o******");
	//          1234567890123456789012345678901
	servo.CloseDoor();
	delay(TABLE_START_DELAY);
}

void showBallScore(bool gameOver)
{
	if(eobBonus) {
		Msg.ShowBonus();
		delay(DEFAULT_DISPLAY_TIME);
		ulong score = playerScore;
		playerScore = eobBonus;
		Msg.ShowScore();
		playerScore = score;
		delay(DEFAULT_DISPLAY_TIME);
		incrementScore(eobBonus);
	}
	Msg.Show("SCORE");
	delay(DEFAULT_DISPLAY_TIME);
	Msg.ShowScore(gameOver);
	delay(gameOver ? LONG_DISPLAY_TIME : 0);
}

#pragma endregion --------------------------------------------------------------
