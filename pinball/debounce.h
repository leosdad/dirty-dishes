// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Sensor debouncing
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#ifndef debounce_h
#define debounce_h

#include "pinball.h"

// Time constants

#define DEFAULT_DEBOUNCE		5
#define ANALOG_DEBOUNCE			50

class Debounce
{
  public:
	static void Read(byte pin,
		void (*changeStateCallback)() = NULL,
		bool invert = true);
	static void Digital(byte pin,
		void (*changeStateCallback)() = NULL,
		bool invert = true, ulong debounceDelay = DEFAULT_DEBOUNCE);
	static void Analog(byte pin, int min, int max,
		void (*changeStateCallback)() = NULL,
		bool invert = true, ulong debounceDelay = ANALOG_DEBOUNCE);
};

#endif // debounce_h
