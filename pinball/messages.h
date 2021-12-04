// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Display messages
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

#ifndef messages_h
#define messages_h

#include "pinball.h"
#include "display.h"

#define DEFAULT_ROTATE_TIME			200
#define SLOW_FLASH_TIME			600

class Messages
{
  public:
	void Init();
	void Show(char *str);
	void Rotate(char *str, uint time = DEFAULT_ROTATE_TIME);
	void Flash(char *str, uint time = SLOW_FLASH_TIME);
	void ShowBonus();
	void ShowBall();
	void ShowMultiplier();
	void ShowHoldState();
	void ShowScore(bool flash = false);
	void ShowReplay();
	void ShowBallLost();
	void ShowEndGame();

  private:
	void showMultiString(const char *str[], uint nItems, uint *index);
};

#endif // messages_h
