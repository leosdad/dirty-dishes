/*
  leds.h
*/

#ifndef display_h
#define display_h

#include <Arduino.h>
#include <FtModules.h>

#include "Simpletypes.h"
// #include "child.h"

// Constants for other modules

#define SEVENSEGDISPLAY_ADR		0x09
#define DISPLAYCHARS			6

void DisplayClear();
void DisplayTest();
void DisplayShow(char *str);
void DisplayHold(uint ms);
void DisplayFlash(uint ms);
void DisplayRotate(uint ms);
void DisplayStop();

#endif // display_h
