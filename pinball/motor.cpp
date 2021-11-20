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
	i2c.Cmd(CHILD_ADDRESS, (int)childCommands::MOTOR, HIGH);
	delay(FEEDBALL_TIME);
	while(digitalRead(feederHomeSensor)) {
		delay(5);
	}
	i2c.Cmd(CHILD_ADDRESS, (int)childCommands::MOTOR, LOW);
}

#pragma endregion --------------------------------------------------------------
