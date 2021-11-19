/*
	Display functions
*/

#include <Arduino.h>
#include <FtModules.h>
#include "display.h"

void Display::Clear()
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdBlank);
}

void Display::Test()
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdTest);
}

void Display::Show(char *str)
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdDisplay, str);
}

void Display::Hold(uint ms)
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdHold, lowByte(ms), highByte(ms));
}

void Display::Flash(uint ms)
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdFlash, lowByte(ms), highByte(ms));
}

void Display::Rotate(uint ms)
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdRotate, ms);
}

void Display::Stop()
{
	FtModules::I2C::Cmd(SEVENSEGDISPLAY_ADR, FtModules::SevenSegDisplay::cmdStop);
}

void Display::U2s(char *buffer, unsigned long value)
{
	// https://forum.arduino.cc/t/right-justify/93157/10

	for(int i = DISPLAYCHARS - 1; i >= 0; i--) {
		buffer[i] = (value == 0 && i != DISPLAYCHARS - 1) ? ' ' : '0' + value % 10;
		value /= 10;
	}
}
