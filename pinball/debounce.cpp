// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Sensor debouncing
// Rubem Pechansky 2021

// Ref.: https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce

// -----------------------------------------------------------------------------

#include "debounce.h"

#pragma region Hardware constants ----------------------------------------------

#pragma endregion --------------------------------------------------------------

#pragma region Hardware variables ----------------------------------------------

uint sensorState[ARDUINO_PINS];
uint lastSensorState[ARDUINO_PINS];
ulong lastDebounceTime[ARDUINO_PINS];

#pragma endregion --------------------------------------------------------------

#pragma region Sensor functions ------------------------------------------------

void Debounce::Read(byte pin, void (*changeStateCallback)() = NULL,
	bool invert = true)
{
	int reading = digitalRead(pin);
	if(invert) {
		reading = !reading;
	}

	if(reading != sensorState[pin]) {
		sensorState[pin] = reading;
		if(reading && changeStateCallback) {
			changeStateCallback();
		}
	}
}

void Debounce::Digital(byte pin, void (*changeStateCallback)() = NULL,
	bool invert = true, ulong debounceDelay = DEFAULT_DEBOUNCE)
{
	int reading = digitalRead(pin);
	if(invert) {
		reading = !reading;
	}

	if(reading != lastSensorState[pin]) {
		lastDebounceTime[pin] = millis();
	}

	if((millis() - lastDebounceTime[pin]) > debounceDelay) {
		if(reading != sensorState[pin]) {
			sensorState[pin] = reading;
			if(reading && changeStateCallback) {
				changeStateCallback();
			}
		}
	}
	lastSensorState[pin] = reading;
}

void Debounce::Analog(byte pin, int min, int max, void (*changeStateCallback)() = NULL,
	bool invert = true, ulong debounceDelay = ANALOG_DEBOUNCE)
{
	int val = analogRead(pin);
	bool reading = val >= min && val < max;

	if(reading != lastSensorState[pin]) {
		lastDebounceTime[pin] = millis();
	}

	if((millis() - lastDebounceTime[pin]) > debounceDelay) {
		if(reading != sensorState[pin]) {
			sensorState[pin] = reading;
			if(reading && changeStateCallback) {
				changeStateCallback();
			}
		}
	}
	lastSensorState[pin] = reading;
}

#pragma endregion --------------------------------------------------------------
