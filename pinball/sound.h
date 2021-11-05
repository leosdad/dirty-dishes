/*
  sound.h
*/

#ifndef sound_h
#define sound_h

#include <Arduino.h>
#include <FtModules.h>

extern "C" FtModules::I2C i2c;

void PlaySound(byte soundIndex);

#endif // sound_h
