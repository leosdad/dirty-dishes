// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Display messages
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#ifndef messages_h
#define messages_h

#include "pinball.h"
#include "display.h"

class Messages
{
  public:
	void ShowStartAnimation();
	void ShowNewBallAnimation();
	void ShowBall();
	void ShowMultiplier();
	void ShowHoldState();
	void ShowScore(bool flash = false);
	void ShowReplay();
	void ShowBallLost();
	void ShowEndGame();

  private:
	void showMultiString(const char *msg[], uint nItems, uint *index);
};

#endif // messages_h
