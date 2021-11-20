// -----------------------------------------------------------------------------

// Dirty Dishes pinball: I²C sound wrapper commands
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#ifndef sound_h
#define sound_h

#include <Arduino.h>
#include <FtModules.h>
#include "child.h"

class Sound
{
  public:
	static void Play(byte soundIndex);
};

#endif // sound_h
