// -----------------------------------------------------------------------------

// Dirty Dishes pinball: Game constants
// Rubem Pechansky 2021

// -----------------------------------------------------------------------------

// General

#define BALLS_PER_GAME			3
#define MAX_FREE_REPLAYS		2
#define MAX_MULTIPLIER			8
#define HOLD_THRESHOLD			3		// No. of stop sensor hits to activate hold
#define BREAK_STREAK			14		// Spinner streak for higher scores

// Points awarded

#define LEFT_ORBIT_POINTS		50
#define HOLD_POINTS				1000
#define HOLD_ACTIVE_POINTS		1250
#define BALL_LOST_POINTS		250
#define OUTLANE_POINTS			800
#define SPINNER_POINTS			25
#define SPINNER_BREAK_POINTS	75
#define ROLLOVER_POINTS			50
#define SKILL_SHOT_POINTS		1500

#define GREASY_SCORE			5000
#define GREASY_BONUS			350

// Time constants

#define TABLE_START_DELAY		1000
#define BALL_SAVER_TIME			3000
#define SKILL_SHOT_TIME			2000
#define BALL_LOST_TIMEOUT		1700	// Time to reach the rear side
#define BALL_NEAR_HOME_TIME		300		// Time to place the ball into playfield
#define HOLD_TIME				5000
#define HOLD_COUNTER_TIME		800
#define RELEASE_TIME			500		// Must be enough to let the ball go
#define MULTIPLIER_RESET_TIME	1500
#define MULTIPLIER_WAIT_TIME	300
#define NORMAL_ONESHOT			800
#define NORMAL_FLASH_LEDS		200
#define LEDS_ANIMATION_TIME		800
#define MSG_END_GAME_TIME		1500
#define MSG_END_SCORE_TIME		1500
#define MSG_END_FLASH_TIME		250
#define DEFAULT_DISPLAY_TIME	500
#define LONG_DISPLAY_TIME		2000
#define SPINNER_STREAK_TIMER	200
