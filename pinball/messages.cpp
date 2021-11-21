// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Display messages
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include "messages.h"

#pragma region Game constants ----------------------------------------------

// Time constants

#define ENDGAME_TIME			1500
#define ENDSCORE_TIME			1500
#define DISPLAY_ROTATE_TIME		200
#define SLOW_FLASH_TIME			600
#define END_FLASH_TIME			250

// Multi-messages

uint ballLostCount = 0;
uint replayCount = 0;
uint endGameCount = 0;

const char *ballLostMessages[] = {" OOPS", "UH-OH", " OUT"};
const char *replayMessages[] = {"REPLAY", "AGAIN", "LUCKY", "SAVED"};
const char *endGameMessages[] = {"  BYE", " CIAO", "Ended", "ADIOS", "  End", " LOST"};

#pragma endregion --------------------------------------------------------------

#pragma region Hardware variables ----------------------------------------------

char displayBuffer[DISPLAYCHARS];

#pragma endregion --------------------------------------------------------------

#pragma region Game variables --------------------------------------------------

extern "C" byte currentBall;
extern "C" byte multiplier;
extern "C" byte stopSensorHits;
extern "C" ulong playerScore;

#pragma endregion --------------------------------------------------------------

#pragma region Public methods --------------------------------------------------

void Messages::ShowStartAnimation()
{
	Display::Rotate(DISPLAY_ROTATE_TIME);
	Display::Show("oooooo*oooooo******o******");
	//            1234567890123456789012345678901
}

void Messages::ShowNewBallAnimation()
{
	Display::Clear();
	Display::Rotate(120);
	Display::Show("_-@-_-@-");
}

void Messages::ShowBall()
{
	Display::Stop();
	strcpy(displayBuffer, "BALL  ");
	displayBuffer[5] = '0' + currentBall;
	Display::FlashMessage(displayBuffer, SLOW_FLASH_TIME);
	Display::Hold(1000);
}

void Messages::ShowMultiplier()
{
	strcpy(displayBuffer, "MULT  ");
	displayBuffer[5] = '0' + multiplier;
	Display::Show(displayBuffer);
	Display::Hold(1000);
	Serial.print("*** Multiplier: ");
	Serial.println(multiplier);
}

void Messages::ShowHoldState()
{
	strcpy(displayBuffer, "HOLD  ");
	displayBuffer[5] = '1' + stopSensorHits;
	Display::Show(displayBuffer);
	Display::Hold(700);
	Serial.print("Hold: ");
	Serial.println(stopSensorHits);
}

void Messages::ShowScore(bool flash = false)
{
	if(flash) {
		delay(ENDGAME_TIME);
		Display::U2s(displayBuffer, playerScore);
		Display::FlashMessage(displayBuffer, END_FLASH_TIME);
		delay(ENDSCORE_TIME);
	} else {
		Display::Stop();
		Display::U2s(displayBuffer, playerScore);
		Display::Show(displayBuffer);
	}
}

void Messages::ShowReplay()
{
	showMultiString(replayMessages, NUMITEMS(replayMessages), &replayCount);
}

void Messages::ShowBallLost()
{
	showMultiString(ballLostMessages, NUMITEMS(ballLostMessages), &ballLostCount);
}

void Messages::ShowEndGame()
{
	showMultiString(endGameMessages, NUMITEMS(endGameMessages), &endGameCount);
}

#pragma endregion --------------------------------------------------------------

#pragma region Private methods --------------------------------------------------

void Messages::showMultiString(const char *msg[], uint nItems, uint *index)
{
	Display::Show((char *)msg[*index % nItems]);
	(*index)++;
}

#pragma endregion --------------------------------------------------------------
