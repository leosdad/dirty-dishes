/*
	Display wrapper class
*/

#include <Arduino.h>
#include <FtModules.h>
#include "display.h"

void DisplayClear()
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdBlank);
}

void DisplayTest()
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdTest);
}

void DisplayShow(char *str)
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdDisplay, str);
}

void DisplayHold(uint ms)
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdHold, lowByte(ms), highByte(ms));
}

void DisplayFlash(uint ms)
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdFlash, lowByte(ms), highByte(ms));
}

void DisplayRotate(uint ms)
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdRotate, ms);
}

void DisplayStop()
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdStop);
}
