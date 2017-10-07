// Invaders.h

#ifndef _INVADERS_h
#define _INVADERS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/**** Pin mapping ****/
#define START_BUTTON 2
#define LEFT_BUTTON  3
#define RIGHT_BUTTON 4
#define AUDIO_PIN    5


struct PlayerShot
{
	bool active;
	int16_t x;
	int16_t y;
};

/**** Screen view port layout ****/
// Not the same as physical display
#define VIEW_ROTATION 3	// Vertical - top of display to right
#define VIEW_WIDTH 64
#define VIEW_HEIGHT 128

#define SCORE_HIT_BOMB 40

extern PlayerShot currentShot;

extern uint8_t baseX;
extern int score;
extern bool baseHit;

#endif
