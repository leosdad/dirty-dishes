/*
  display.h
*/

#ifndef display_h
#define display_h

#include <Arduino.h>
#include <FtModules.h>
#include "Simpletypes.h"

// Constants for other modules

#define SEVENSEGDISPLAY_ADR		0x09
#define DISPLAYCHARS			6

class Display
{
  public:
	static void Init();
	static void FlashMessage(char *str, uint speed);
	static void Clear();
	static void Test();
	static void Show(char *str);
	static void Hold(uint ms);
	static void Flash(uint ms);
	static void Rotate(uint ms);
	static void Stop();
	static void U2s(char *buffer, unsigned long value);
};


#endif // display_h
