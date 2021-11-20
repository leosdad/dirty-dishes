// -----------------------------------------------------------------------------

// Dirty Dishes pinball: I²C sound wrapper commands
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include "sound.h"

#pragma region Methods ---------------------------------------------------------

void Sound::Play(byte soundIndex)
{
	FtModules::I2C::Cmd(CHILD_ADDRESS, (int)childCommands::SOUND, soundIndex);
}

#pragma endregion --------------------------------------------------------------
