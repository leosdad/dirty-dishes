// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Tests
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#ifndef tests_h
#define tests_h

class Tests
{
  public:
	static void Leds();
	static void Sounds();
	static void Inputs();
	static void AnalogSensors();
	static void Servo();

  private:
	static void testDigitalSensor(byte sensor, bool *last, char *name);
	static void testAnalogSensor(byte sensor, uint min, uint max, char *name);
};

#endif // tests_h
