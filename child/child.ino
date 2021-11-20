// -----------------------------------------------------------------------------

// Dirty Dishes pinball: child Arduino
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include <Arduino.h>
#include <Wire.h>
#include <AsyncDelay.h>
#include <FtModules.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#include "Simpletypes.h"
#include "pb_child.h"

#pragma region Constants -------------------------------------------------------

// Baud rates

#define BAUDRATE			57600
#define DFPLAYER_BAUDRATE	9600

#define CLOSED_DOOR			10
#define OPEN_DOOR			110
#define LED_DEFAULT_TIME	250
#define DEFAULT_VOLUME		12			// 0-30
#define FX_TIMEOUT			10
#define NUMPIXELS1			4

// const byte MAXCMD = BUFFER_LENGTH;

// Arduino pins

const byte soundTx = 2;
const byte soundRx = 3;
const byte servoDoor = 6;
const byte feederMotor = 8;
const byte rolloverSkillLed = 9;
const byte rollover3Led	= 10;
const byte rollover2Led = 11;
const byte rollover1Led = 12;
const byte lights = 13;
const byte leftOrbitLed = A0;
const byte holdLed = A1;
const byte leftOutlaneLed = A2;
const byte rightOutlaneLed = A3;

// Inputs and outputs

const byte outputs[] = {
	servoDoor, feederMotor,
	rolloverSkillLed, rollover1Led, rollover2Led, rollover3Led,
	leftOrbitLed, holdLed, leftOutlaneLed, rightOutlaneLed,
	lights
};

Servo servo;

#pragma endregion --------------------------------------------------------------

#pragma region Variables -------------------------------------------------------

uint servoPos = 0;
byte pixBits = 0;
byte lastPixBits = 0;

// Sound

SoftwareSerial mySoftwareSerial(soundRx, soundTx);
DFRobotDFPlayerMini myDFPlayer;

struct sLedData {
	uint ledIndex;
	AsyncDelay timer;
	outState flash;
	bool state;
};

AsyncDelay timers[NLEDS];

sLedData ledData[NLEDS] = {

	// LEDs

	{rollover1Led,    	timers[0], outState::OFF, false},
	{rollover2Led,    	timers[1], outState::OFF, false},
	{rollover3Led,    	timers[2], outState::OFF, false},
	{rolloverSkillLed,	timers[3], outState::OFF, false},
	{holdLed,			timers[5], outState::OFF, false},
	{rightOutlaneLed,	timers[7], outState::OFF, false},
	{leftOutlaneLed,	timers[6], outState::OFF, false},
	{leftOrbitLed,		timers[4], outState::OFF, false},
	{lights,           	timers[8], outState::OFF, false},
};

#pragma endregion --------------------------------------------------------------

#pragma region Setup -----------------------------------------------------------

void setup()
{
	// Serial.begin(BAUDRATE);
	soundInit();

	// Set up pin modes

	for(int i = 0; i < sizeof outputs; i++) {
		pinMode(outputs[i], OUTPUT);
	}

	servo.attach(servoDoor);
	closeDoor();

	Wire.begin(CHILD_ADDRESS);
	Wire.onReceive(receiveEvent);

	// Serial.println("Child Arduino is ready");
}

#pragma endregion --------------------------------------------------------------

#pragma region Main loop / game loop -------------------------------------------

void loop()
{
	checkTimers();

	// testServo();
	// testAllLeds();
	// testSound();
	// testOutputs();
}

#pragma endregion --------------------------------------------------------------

#pragma region Timer functions -------------------------------------------------

void startTimer(uint index, ulong ms)
{
	ledData[index].flash = outState::FLASH;
	ledData[index].state = true;
	ledData[index].timer.start(ms, AsyncDelay::MILLIS);
}

void startOneShot(uint index, ulong ms)
{
	ledData[index].flash = outState::ONESHOT;
	ledData[index].state = true;
	ledData[index].timer.start(ms, AsyncDelay::MILLIS);
}

void checkTimers()
{
	for(int i = 0; i < NLEDS; i++) {
		if(ledData[i].timer.isExpired() && ledData[i].flash > outState::OFF) {
			if(ledData[i].flash == outState::FLASH) {
				ledData[i].state = !ledData[i].state;
				setLed(i, ledData[i].state);
				ledData[i].timer.repeat();
			} else if(ledData[i].flash == outState::ONESHOT) {
				setLed(i, false);
			}
		}
	}
}

void stopTimer(uint index)
{
	if(ledData[index].flash > outState::OFF) {
		ledData[index].timer.expire();
		ledData[index].flash = outState::OFF;
		ledData[index].state = false;
	}
}

#pragma endregion --------------------------------------------------------------

#pragma region LED functions ---------------------------------------------------

void setLed(uint index, bool value)
{
	sLedData ld = ledData[index];
	digitalWrite(ld.ledIndex, value);
}

void processLedCmd(byte index, outState cmd, byte time)
{
	switch(cmd) {

		case outState::ON:
			stopTimer(index);
			setLed(index, true);
			break;

		case outState::OFF:
			stopTimer(index);
			setLed(index, false);
			break;

		case outState::FLASH:
			setLed(index, true);
			startTimer(index, time * 100);
			break;

		case outState::ONESHOT:
			setLed(index, true);
			startOneShot(index, time * 100);
			break;
	}
}

#pragma endregion --------------------------------------------------------------

#pragma region I²C functions ---------------------------------------------------

void receiveEvent(int nBytes)
{
	byte cmd[BUFFER_LENGTH];

	for(int count = 0; count < BUFFER_LENGTH; count++) {
		bool available = Wire.available();
		cmd[count] = available ? Wire.read() : '\x0';
	}

	// Received commands

	switch((byte)cmd[0]) {

		case (byte)childCommands::RESET:
			for(int i = 0; i < sizeof outputs; i++) {
 				digitalWrite(outputs[i], outputs[i] == lights ? HIGH : LOW);
			}
			myDFPlayer.stop();
			break;

		case (byte)childCommands::PORT:
			digitalWrite(cmd[1], cmd[2]);
			break;

		case (byte)childCommands::SERVO:
			switch((byte)cmd[1]) {
				case (byte)servoCmd::CLOSE:
					closeDoor();
					break;
				case (byte)servoCmd::OPEN:
					openDoor();
					break;
			}
			break;

		case (byte)childCommands::SOUND:
			myDFPlayer.play((byte)cmd[1]);
			break;

		case (byte)childCommands::LED:
			processLedCmd(cmd[1], (outState)cmd[2], cmd[3]);
			break;

		case (byte)childCommands::MOTOR:
			digitalWrite(feederMotor, cmd[1] ? HIGH : LOW);
			break;
	}
}

#pragma endregion --------------------------------------------------------------

#pragma region Sound functions -------------------------------------------------

void soundInit()
{
	// https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299

	mySoftwareSerial.begin(DFPLAYER_BAUDRATE);
	if(!myDFPlayer.begin(mySoftwareSerial)) {
		Serial.println("Unable to start MP3 module");
		return;
	}
	Serial.println("MP3 module ready");

	// The default timeout (500) will freeze the Arduino
	myDFPlayer.setTimeOut(FX_TIMEOUT);
	myDFPlayer.volume(DEFAULT_VOLUME);
	myDFPlayer.EQ(0);
}

#pragma endregion --------------------------------------------------------------

#pragma region Servo functions -------------------------------------------------

void openDoor()
{
	servo.write(OPEN_DOOR);
}

void closeDoor()
{
	servo.write(CLOSED_DOOR);
}

#pragma endregion --------------------------------------------------------------

#pragma region Test functions --------------------------------------------------

void testOutputs()
{
	closeDoor();
	testOutput(feederMotor);
	testOutput(rollover1Led);
	testOutput(rollover2Led);
	testOutput(rollover3Led);
	testOutput(rolloverSkillLed);
	openDoor();
	delay(200);
}

void testOutput(uint output)
{
	digitalWrite(output, HIGH);
	delay(200);
	digitalWrite(output, LOW);
}

ulong tsLast = ULONG_MAX;
bool tsOpen = false;

void testServo()
{
	ulong ms = millis();

	if(ms > tsLast + 2000) {
		servo.write(tsOpen ? OPEN_DOOR : CLOSED_DOOR);
		Serial.println(tsOpen ? "open" : "closed");
		tsOpen = !tsOpen;
		tsLast = ms;
	}
}

uint cLed = 0;

void testAllLeds()
{
	setLed(cLed, false);
	cLed = cLed == 8 ? 0 : cLed + 1;
	setLed(cLed, true);
	Serial.print("LED #");
	Serial.println(cLed);
	delay(400);
}

#pragma endregion --------------------------------------------------------------
