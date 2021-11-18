/*
  leds.h
*/

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

  private:
	static FtModules::I2C _i2c;
};

#endif // leds_h
