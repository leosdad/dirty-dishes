// -----------------------------------------------------------------------------

// Dirty Dishes pinball: I²C servo wrapper commands
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include "servo.h"

#pragma region Methods ---------------------------------------------------------

void Servo::CloseDoor()
{
	FtModules::I2C::Cmd(CHILD_ADDRESS, (int)childCommands::SERVO,
		(int)servoCmd::CLOSE);
}

void Servo::OpenDoor()
{
	FtModules::I2C::Cmd(CHILD_ADDRESS, (int)childCommands::SERVO,
		(int)servoCmd::OPEN);
}

#pragma endregion --------------------------------------------------------------
