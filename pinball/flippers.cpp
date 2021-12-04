// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Flipper methods
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#include "flippers.h"

#pragma region Hardware constants ----------------------------------------------

// Parameters for L298N and 19.5 VDC power supply

#define MAX_POWER_MS			50
#define HOLD_PWM				40

#pragma endregion --------------------------------------------------------------

#pragma region Enums -----------------------------------------------------------

// Flipper states

enum class flipperStates
{
	IDLE = 0,
	STROKE,
	HOLD,
	HOLDING,
};

#pragma endregion --------------------------------------------------------------

#pragma region Hardware variables ----------------------------------------------

flipperStates leftFlipperState = flipperStates::IDLE;
flipperStates rightFlipperState = flipperStates::IDLE;

// Time variables

ulong leftButtonPreviousMs;
ulong rightPreviousMs;

#pragma endregion --------------------------------------------------------------

#pragma region Methods ---------------------------------------------------------

void Flippers::Reset()
{
	digitalWrite(leftFlipper, LOW);
	digitalWrite(rightFlipper, LOW);
}

void Flippers::Left()
{
	if(leftFlipperState == flipperStates::IDLE) {
		if(LEFT_BUTTON_ON) {
			digitalWrite(leftFlipper, HIGH);
			leftButtonPreviousMs = millis();
			leftFlipperState = flipperStates::STROKE;
		}
	} else if(leftFlipperState == flipperStates::STROKE) {
		if(LEFT_BUTTON_OFF) {
			digitalWrite(leftFlipper, LOW);
			leftFlipperState = flipperStates::IDLE;
		} else if(millis() - leftButtonPreviousMs >= MAX_POWER_MS) {
			leftFlipperState = flipperStates::HOLD;
		}
	} else if(leftFlipperState == flipperStates::HOLD) {
		if(LEFT_BUTTON_ON) {
			analogWrite(leftFlipper, HOLD_PWM);
		}
		leftFlipperState = flipperStates::HOLDING;
	} else if(leftFlipperState == flipperStates::HOLDING) {
		if(LEFT_BUTTON_OFF) {
			digitalWrite(leftFlipper, LOW);
			leftFlipperState = flipperStates::IDLE;
		}
	}
}

void Flippers::Right()
{
	if(rightFlipperState == flipperStates::IDLE) {
		if(RIGHT_BUTTON_ON) {
			digitalWrite(rightFlipper, HIGH);
			rightPreviousMs = millis();
			rightFlipperState = flipperStates::STROKE;
		}
	} else if(rightFlipperState == flipperStates::STROKE) {
		if(RIGHT_BUTTON_OFF) {
			digitalWrite(rightFlipper, LOW);
			rightFlipperState = flipperStates::IDLE;
		} else if(millis() - rightPreviousMs >= MAX_POWER_MS) {
			rightFlipperState = flipperStates::HOLD;
		}
	} else if(rightFlipperState == flipperStates::HOLD) {
		if(RIGHT_BUTTON_ON) {
			analogWrite(rightFlipper, HOLD_PWM);
		}
		rightFlipperState = flipperStates::HOLDING;
	} else if(rightFlipperState == flipperStates::HOLDING) {
		if(RIGHT_BUTTON_OFF) {
			digitalWrite(rightFlipper, LOW);
			rightFlipperState = flipperStates::IDLE;
		}
	}
}

#pragma endregion --------------------------------------------------------------
