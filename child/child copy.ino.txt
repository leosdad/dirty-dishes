// -----------------------------------------------------------------------------

// Dirty Dishes pinball: child Arduino
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include <Arduino.h>
#include <Wire.h>
#include <AsyncDelay.h>
#include <FtModules.h>
#include <RBD_Servo.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Adafruit_NeoPixel.h>

#include <Simpletypes.h>
#include <child.h>

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

// Pixel colors

const uint32_t colors[] =
{
	0x000000,	// BLACK
	0xFFFF30,	// WHITE
	0xF0FFD8,	// COOLWHITE
	0xFF0000,	// RED
	0x00FF00,	// GREEN
	0x0000FF,	// BLUE
	0x00FFFF,	// CYAN
	0xFF00FF,	// MAGENTA
	0xFF9000,	// YELLOW
	0xFF4000,	// ORANGE
};

// const byte MAXCMD = BUFFER_LENGTH;

// Arduino pins

const byte soundTx = 2;
const byte soundRx = 3;
const byte doorServo = 6;
const byte feederMotor = 8;
const byte rolloverSkillLed = 9;
const byte rollover3Led	= 10;
const byte rollover2Led = 11;
const byte rollover1Led = 12;
const byte lights = 13;
const byte rgbPixelsPin = A3;

// Inputs and outputs

const byte outputs[] = {
	doorServo, feederMotor,
	rolloverSkillLed, rollover1Led, rollover2Led, rollover3Led,
	lights
};

// Default servo values
RBD::Servo rbdServo(doorServo, 544, 2400);

#pragma endregion --------------------------------------------------------------

#pragma region Variables -------------------------------------------------------

uint servoPos = 0;
bool updateServo = true;
byte pixBits = 0;
byte lastPixBits = 0;

// Servo

AsyncDelay servoTimer;

// Sound

SoftwareSerial mySoftwareSerial(soundRx, soundTx);
DFRobotDFPlayerMini myDFPlayer;

// NeoPixels

Adafruit_NeoPixel pixels(NUMPIXELS1, rgbPixelsPin, NEO_GRB + NEO_KHZ800);

struct sLedData {
	uint ledIndex;
	bool isNeoPixel;
	AsyncDelay timer;
	outState flash;
	bool state;
	byte colorIndex;
};

AsyncDelay timers[NLEDS];

sLedData ledData[NLEDS] = {

	// LEDs

	{rollover1Led,     false, timers[0], outState::OFF, false, 0},
	{rollover2Led,     false, timers[1], outState::OFF, false, 0},
	{rollover3Led,     false, timers[2], outState::OFF, false, 0},
	{rolloverSkillLed, false, timers[3], outState::OFF, false, 0},

	// NeoPixels

	{3,					true, timers[4], outState::OFF, false, 1},
	{2,					true, timers[5], outState::OFF, false, 1},
	{1,					true, timers[6], outState::OFF, false, 1},
	{0,					true, timers[7], outState::OFF, false, 1},

	// Luzes dos sensores

	{lights,           false, timers[8], outState::OFF, false, 0},
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
		// digitalWrite(outputs[i], (int)outState::OFF);
	}

	pixels.begin();
	pixels.clear();
	pixels.show();

	closeDoor();

	Wire.begin(CHILD_ADDRESS);
	Wire.onReceive(receiveEvent);

	// Serial.println("Child Arduino is ready");
}

#pragma endregion --------------------------------------------------------------

#pragma region Main loop / game loop -------------------------------------------

void loop()
{
	gameLoop();

	// testServo();
	// testSound();
	// testAllLeds();
	// testOutputs();
}

void gameLoop()
{
	if(pixBits != lastPixBits) {
		pixels.show();
		lastPixBits = pixBits;
		delay(1);
	}

	checkTimers();

	if(servoTimer.isExpired()) {
		updateServo = false;
	}

	if(updateServo) {
		rbdServo.update();
	}
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
				setLed(i, ledData[i].state, -1);
				ledData[i].timer.repeat();
			} else if(ledData[i].flash == outState::ONESHOT) {
				setLed(i, false, -1);
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

// void setLed(uint index, bool on, int colr)
// {
// 	if(ledData[index].isNeoPixel) {
// 		if(on && colr > -1 && colr != ledData[index].colorIndex) {
// 			ledData[index].colorIndex = colr;
// 			bitClear(pixBits, ledData[index].ledIndex);
// 		}
// 		pixels.setPixelColor(ledData[index].ledIndex, on ?
// 			colors[ledData[index].colorIndex] : colors[(int)colorIndex::BLACK]);
// 		lastPixBits = pixBits;
// 		if(on) {
// 			bitSet(pixBits, ledData[index].ledIndex);
// 		} else {
// 			bitClear(pixBits, ledData[index].ledIndex);
// 		}
// 	} else {
// 		digitalWrite(ledData[index].ledIndex, on);
// 	}
// }

void setLed(uint index, bool value, int colr)
{
	sLedData ld = ledData[index];

	if(ld.isNeoPixel) {
		if(value && colr > -1 && colr != ld.colorIndex) {
			ld.colorIndex = colr;
			// bitClear(pixBits, ld.ledIndex);
		}
		pixels.setPixelColor(ld.ledIndex, value ?
			colors[ld.colorIndex] : colors[(int)colorIndex::BLACK]);
		lastPixBits = pixBits;
		if(value) {
			bitSet(pixBits, ld.ledIndex);
		} else {
			bitClear(pixBits, ld.ledIndex);
		}
	} else {
		digitalWrite(ld.ledIndex, value);
	}
}

void processLedCmd(byte index, outState cmd, byte colorIndex, byte time)
{
	switch(cmd) {

		case outState::ON:
			stopTimer(index);
			setLed(index, true, colorIndex);
			break;

		case outState::OFF:
			stopTimer(index);
			setLed(index, false, 0);
			break;

		case outState::FLASH:
			setLed(index, true, colorIndex);
			startTimer(index, time * 100);
			break;

		case outState::ONESHOT:
			setLed(index, true, colorIndex);
			startOneShot(index, time * 100);
			break;
	}
}

#pragma endregion --------------------------------------------------------------

#pragma region I²C functions ---------------------------------------------------

// const char *cmdNames[] = {
// 	"RESET", "PORT", "SERVO", "SOUND", "LED", "MOTOR"
// };

void receiveEvent(int nBytes)
{
	byte cmd[BUFFER_LENGTH];

	for(int count = 0; count < BUFFER_LENGTH; count++) {
		bool available = Wire.available();
		cmd[count] = available ? Wire.read() : '\x0';
		// if(available) {
		// 	if(count == 0) {
		// 		Serial.print(cmdNames[cmd[count] - 0x10]);
		// 	} else {
		// 		Serial.print(cmd[count]);
		// 	}
		// 	Serial.print(" ");
		// }
	}
	// Serial.println("");

	// Received commands

	switch((byte)cmd[0]) {

		case (byte)childCommands::RESET:
			pixels.clear();
			for(int i = 0; i < sizeof outputs; i++) {
 				digitalWrite(outputs[i], outputs[i] == lights ? HIGH : LOW);
			}
			myDFPlayer.stop();
			// Serial.println("---------- RESET");
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
			processLedCmd(cmd[1], (outState)cmd[2], cmd[3], cmd[4]);
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
	updateServo = true;
	rbdServo.moveToDegrees(OPEN_DOOR);
	servoTimer.start(SERVO_TIMER, AsyncDelay::MILLIS);
}

void closeDoor()
{
	updateServo = true;
	rbdServo.moveToDegrees(CLOSED_DOOR);
	servoTimer.start(SERVO_TIMER, AsyncDelay::MILLIS);
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
		rbdServo.moveToDegrees(tsOpen ? OPEN_DOOR : CLOSED_DOOR);
		Serial.println(tsOpen ? "open" : "closed");
		tsOpen = !tsOpen;
		tsLast = ms;
	}

	rbdServo.update();
}

uint cLed = 0;
uint cCol = 1;

void testAllLeds()
{
	setLed(cLed, false, 0);
	cLed = cLed == 8 ? 0 : cLed + 1;
	if(cLed == 8) {
		cCol = cCol == 9 ? 1 : cCol + 1;
	}
	setLed(cLed, true, cCol);
	Serial.print("LED #");
	Serial.println(cLed);
	delay(500);
}

#pragma endregion --------------------------------------------------------------
