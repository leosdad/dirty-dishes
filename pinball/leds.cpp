// -----------------------------------------------------------------------------

// Dirty Dishes pinball: LEDs
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include "leds.h"

#define ANIMATION_TIME		800

#pragma region LED state functions ---------------------------------------------

void Leds::On(childLeds led)
{
	_i2c.Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led,
		(byte)outState::ON, 0);
}

void Leds::Flash(childLeds led, uint time)
{
	_i2c.Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led,
		(byte)outState::FLASH, time / 100);
}

void Leds::OneShot(childLeds led, uint time)
{
	_i2c.Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led,
		(byte)outState::ONESHOT, time / 100);
}

void Leds::Off(childLeds led)
{
	_i2c.Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led,
		(byte)outState::OFF, 0);
}

#pragma endregion --------------------------------------------------------------

#pragma region Public methods --------------------------------------------------

void Leds::waitAnimation(bool (*callback)())
{
	for(int i = 0; i < 4; i++) {

		childLeds led = (childLeds)(i % 4);

		if(i % 2) {
			On((childLeds)8);
		} else {
			Off((childLeds)8);
		}

		On(led);
		On((childLeds)((int)led + 4));
		ulong msStart = millis();
		while(millis() < msStart + ANIMATION_TIME) {
			if(callback()) {
				allOff(true);
				return;
			}
		}
		Off(led);
		Off((childLeds)((int)led + 4));
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
