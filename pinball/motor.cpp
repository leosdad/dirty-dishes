// -----------------------------------------------------------------------------

// Dirty Dishes pinball: I²C motor wrapper commands
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include "motor.h"

#pragma region Constants -------------------------------------------------------

#define FEEDBALL_TIME			100

#pragma endregion --------------------------------------------------------------

#pragma region Methods ---------------------------------------------------------

void Motor::FeedBall()
{
	FtModules::I2C::Cmd(CHILD_ADDRESS, (int)childCommands::MOTOR, HIGH);
	delay(FEEDBALL_TIME);
	while(digitalRead(feederHomeSensor)) {
		delay(5);
	}
	FtModules::I2C::Cmd(CHILD_ADDRESS, (int)childCommands::MOTOR, LOW);
}

#pragma endregion --------------------------------------------------------------
