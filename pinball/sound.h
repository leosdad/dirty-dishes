/*
  sound.h
*/

#ifndef sound_h
#define sound_h

#include <Arduino.h>
#include <FtModules.h>

extern "C" FtModules::I2C i2c;

class Sound
{
  public:
	static void Play(byte soundIndex);
};

#endif // sound_h
