/*
	Sound wrapper class
*/

#include "child.h"
#include "sound.h"

void Sound::Play(byte soundIndex)
{
	i2c.Cmd(CHILD_ADDRESS, (int)childCommands::SOUND, soundIndex);
}
