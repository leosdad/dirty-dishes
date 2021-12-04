// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Display messages
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include "messages.h"
#include "game.h"

#pragma region Game constants ----------------------------------------------

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

extern byte currentBall;
extern byte multiplier;
extern byte stopSensorHits;
extern ulong playerScore;

#pragma endregion --------------------------------------------------------------

#pragma region Public methods --------------------------------------------------

void Messages::Init()
{
	Display::Init();
}

void Messages::Show(char *str)
{
	Display::Clear();
	Display::Stop();
	Display::Show(str);
}

void Messages::Rotate(char *str, uint time = DEFAULT_ROTATE_TIME)
{
	Display::Clear();
	Display::Show(str);
	Display::Rotate(time);
}

void Messages::Flash(char *str, uint time = SLOW_FLASH_TIME)
{
	Display::Stop();
	Display::Show(str);
	Display::Flash(time);
}

void Messages::ShowBonus()
{
	Display::Stop();
	strcpy(displayBuffer, "BONUS");
	Display::Show(displayBuffer);
	Display::Flash(SLOW_FLASH_TIME);
}

void Messages::ShowBall()
{
	Display::Stop();
	strcpy(displayBuffer, "BALL  ");
	displayBuffer[5] = '0' + currentBall;
	Flash(displayBuffer, SLOW_FLASH_TIME);
	// Display::Hold(1000);
}

void Messages::ShowMultiplier()
{
	strcpy(displayBuffer, "MULT  ");
	displayBuffer[5] = '0' + multiplier;
	Display::Show(displayBuffer);
	// Display::Hold(1000);
	// Serial.print("*** Multiplier: ");
	// Serial.println(multiplier);
}

void Messages::ShowHoldState()
{
	strcpy(displayBuffer, "HOLD  ");
	displayBuffer[5] = '1' + stopSensorHits;
	Display::Show(displayBuffer);
	// Display::Hold(700);
	// Serial.print("Hold: ");
	// Serial.println(stopSensorHits);
}

void Messages::ShowScore(bool flash = false)
{
	if(flash) {
		delay(MSG_END_GAME_TIME);
		Display::U2s(displayBuffer, playerScore);
		Flash(displayBuffer, MSG_END_FLASH_TIME);
		delay(MSG_END_SCORE_TIME);
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

void Messages::showMultiString(const char *str[], uint nItems, uint *index)
{
	Display::Show((char *)str[*index % nItems]);
	(*index)++;
}

#pragma endregion --------------------------------------------------------------
