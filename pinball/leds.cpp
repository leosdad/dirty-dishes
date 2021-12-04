// -----------------------------------------------------------------------------

// Dirty Dishes pinball: LEDs
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include "leds.h"
#include "game.h"

#pragma region LED state functions ---------------------------------------------

void Leds::On(childLeds led)
{
	FtModules::I2C::Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led,
		(byte)outState::ON, 0);
}

void Leds::Flash(childLeds led, uint time)
{
	FtModules::I2C::Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led,
		(byte)outState::FLASH, time / 100);
}

void Leds::OneShot(childLeds led, uint time)
{
	FtModules::I2C::Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led,
		(byte)outState::ONESHOT, time / 100);
}

void Leds::Off(childLeds led)
{
	FtModules::I2C::Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led,
		(byte)outState::OFF, 0);
}

#pragma endregion --------------------------------------------------------------

#pragma region Public methods --------------------------------------------------

int lLed = -1;
int cLed = 0;
ulong lastAnimMs = 0;

void Leds::waitAnimation()
{
	if(millis() > lastAnimMs + LEDS_ANIMATION_TIME) {

		Off((childLeds)lLed);
		On((childLeds)cLed);
		Off((childLeds)(lLed + 4));
		On((childLeds)(cLed + 4));

		if(cLed % 2) {
			On((childLeds)8);
		} else {
			Off((childLeds)8);
		}

		lastAnimMs = millis();
		lLed = cLed;
		cLed = cLed == 3 ? 0 : cLed + 1;
	}
}

void Leds::flashes(int time)
{
	for(int i = 0; i < NLEDS; i++) {
		Flash((childLeds)i, time);
	}
}

void Leds::allOff(bool lightsOff)
{
	for(int i = 0; i < NLEDS; i++) {
		if(lightsOff || (childLeds)i != childLeds::LIGHTS) {
			Off((childLeds)i);
		}
	}
}

#pragma endregion --------------------------------------------------------------
