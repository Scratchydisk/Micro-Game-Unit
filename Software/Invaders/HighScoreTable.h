#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/*********************
EEPROM MEMORY MAP

Start	Length		Type		Purpose
-----	------		----		-------
0		2			uint		Program identifier - stops different programs trying to read wrong data
2		sizeof()	HighScore[]	High scores array
	
**********************/

// Address of Progarm ID uint in EEPROM
#define EE_PROG_ID 0
#define EE_HIGH_SCORES 2

/**** Program Identifier *****/
#define PROGRAM_IDENTIFIER	1

#define NUM_HIGH_SCORES 5

struct HighScore
{
	char Initial1;
	char Initial2;
	char Initial3;
	int Score;
};

class HighScoreTable
{
public:
	HighScoreTable();
	~HighScoreTable();

	HighScore Scores[NUM_HIGH_SCORES];
	// Resets the high scores table to its default
	void InitialiseHighScores();
	void WriteHighScores();
	// -1 if not a new high score
	// otherwise the index of the new high score entry
	int IsNewHighScore(uint16_t score);

private:
	bool HighScoresValidForProgram();
	void LoadHighScores();
};

