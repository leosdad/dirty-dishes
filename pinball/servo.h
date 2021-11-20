// -----------------------------------------------------------------------------

// Dirty Dishes pinball: I²C servo wrapper commands
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#ifndef servo_h
#define servo_h

#include "pinball.h"

class Servo
{
  public:
	void CloseDoor();
	void OpenDoor();
};

#endif // servo_h
