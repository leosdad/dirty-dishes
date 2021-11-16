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
	// void Init(FtModules::I2C i2c);
	void Init();

	void waitAnimation(bool (*callback)());
	void redFlashes(int time);
	void allOff(bool lightsOff);

	void On(childLeds led, colorIndex colorIndex);
	void On(childLeds led);
	void Flash(childLeds led, uint time, colorIndex colorIndex);
	void Flash(childLeds led, uint time);
	void OneShot(childLeds led, uint time, colorIndex colorIndex);
	void OneShot(childLeds led, uint time);
	void Off(childLeds led);

  private:
	static FtModules::I2C _i2c;
	byte _colorOffset;
	static const byte nColors = 5;

	const colorIndex animColors[nColors] = {
		colorIndex::WHITE,
		colorIndex::RED,
		colorIndex::YELLOW,
		colorIndex::COOLWHITE,
		colorIndex::ORANGE
	};
};

#endif // leds_h
