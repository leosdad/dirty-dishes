// -----------------------------------------------------------------------------

// Dirty Dishes pinball: LEDs
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include "leds.h"

#define ANIMATION_TIME		800

// FtModules::I2C _i2c;

void Leds::Init()
{
	_colorOffset = 0;
}

#pragma region LED state functions ---------------------------------------------

void Leds::On(childLeds led)
{
	_i2c.Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led, (byte)outState::ON, 1, 0);
}

void Leds::On(childLeds led, colorIndex colorIndex)
{
	_i2c.Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led, (byte)outState::ON, (byte)colorIndex, 0);
}

void Leds::Flash(childLeds led, uint time)
{
	_i2c.Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led, (byte)outState::FLASH, 1, time / 100);
}

void Leds::Flash(childLeds led, uint time, colorIndex colorIndex)
{
	_i2c.Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led, (byte)outState::FLASH, (byte)colorIndex,
		time / 100);
}

void Leds::OneShot(childLeds led, uint time)
{
	_i2c.Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led, (byte)outState::ONESHOT, 1, time / 100);
}

void Leds::OneShot(childLeds led, uint time, colorIndex colorIndex)
{
	_i2c.Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led, (byte)outState::ONESHOT, (byte)colorIndex,
		time / 100);
}

void Leds::Off(childLeds led)
{
	_i2c.Cmd(CHILD_ADDRESS, (byte)childCommands::LED, (byte)led, (byte)outState::OFF, (byte)colorIndex::BLACK, 0);
}

#pragma endregion --------------------------------------------------------------

#pragma region Public methods --------------------------------------------------

void Leds::waitAnimation(bool (*callback)())
{
	for(int i = 0; i < NLEDS - 1; i++) {

		childLeds led = (childLeds)(i % 4);
		childLeds pix = (childLeds)(4 + i % 4);
		colorIndex color = (colorIndex)(animColors[(_colorOffset + i / 2) % nColors]);

		if(i % 2) {
			On((childLeds)8);
		} else {
			Off((childLeds)8);
		}

		On(led);
		On(pix, color);
		ulong msStart = millis();
		while(millis() < msStart + ANIMATION_TIME) {
			if(callback()) {
				allOff(true);
				return;
			}
		}
		Off(led);
		Flash(pix, ANIMATION_TIME / 2, color);
	}
	_colorOffset++;
}

void Leds::redFlashes(int time)
{
	for(int i = 0; i < NLEDS; i++) {
		Flash((childLeds)i, time, colorIndex::RED);
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
