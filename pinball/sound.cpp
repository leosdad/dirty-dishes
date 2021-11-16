/*
	Sound wrapper class
*/

#include "child.h"
#include "sound.h"

void PlaySound(byte soundIndex)
{
	i2c.Cmd(CHILD_ADDRESS, (int)childCommands::SOUND, soundIndex);
}
