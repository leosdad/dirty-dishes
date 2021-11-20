// -----------------------------------------------------------------------------

// Dirty Dishes pinball: LEDs
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#ifndef leds_h
#define leds_h

#include <Arduino.h>
#include <FtModules.h>

#include "Simpletypes.h"
#include "child.h"

class Leds
{
  public:
	void waitAnimation(bool (*callback)());
	void flashes(int time);
	void allOff(bool lightsOff);

	void On(childLeds led);
	void Flash(childLeds led, uint time);
	void OneShot(childLeds led, uint time);
	void Off(childLeds led);
};

#endif // leds_h
