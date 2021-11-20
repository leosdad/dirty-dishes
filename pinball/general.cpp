// -----------------------------------------------------------------------------

// Dirty Dishes pinball: I²C general wrapper commands
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include "general.h"

#pragma region Methods ---------------------------------------------------------

void General::Reset()
{
	FtModules::I2C::Cmd(CHILD_ADDRESS, (int)childCommands::RESET);
}

#pragma endregion --------------------------------------------------------------
