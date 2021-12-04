// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Sensor check functions
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#ifndef sensors_h
#define sensors_h

#include "pinball.h"

void resetRollovers();

bool checkButtons();
bool checkOrbitSensor();
bool checkRollovers();
bool checkSkillShot();
bool checkStopMagnet();
bool checkSpinner();
bool checkOutlanes();
bool checkBallLost();
bool checkLaunch();

#endif // sensors_h
