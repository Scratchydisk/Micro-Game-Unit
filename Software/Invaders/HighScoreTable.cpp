#include "HighScoreTable.h"



HighScoreTable::HighScoreTable()
{
	// If EEPROM doesn't contain valid
	// data for high scores then initialise
	// it to a starting set.
	if (HighScoresValidForProgram())
	{
		LoadHighScores();
	}
	else
	{
		InitialiseHighScores();
		WriteHighScores();
	}
}

HighScoreTable::~HighScoreTable()
{
}

void HighScoreTable::LoadHighScores()
{
	eeprom_read_block(Scores, (void *)EE_HIGH_SCORES, sizeof(Scores));
}

void HighScoreTable::WriteHighScores()
{
	eeprom_write_block(Scores, (void *)EE_HIGH_SCORES, sizeof(Scores));
}

// Determines if the data in the EEPROM
// is a valid HighScores structure for
// this program.
bool HighScoreTable::HighScoresValidForProgram()
{
	// If the program identifier is valid for this game...

	// Read program identifier bytes
	uint16_t progId = eeprom_read_word(EE_PROG_ID);

	return progId == PROGRAM_IDENTIFIER;
}

int HighScoreTable::IsNewHighScore(uint16_t score)
{
	for (uint8_t i = 0; i < NUM_HIGH_SCORES; i++)
	{
		if (score >= Scores[i].Score)
		{
			// Push scores down to leave slot for current one
			for (uint8_t j = NUM_HIGH_SCORES - 1; j > i; j--)
			{
				Scores[j] = Scores[j - 1];
			}
			Scores[i].Score = score;
			Scores[i].Initial1 = 'A';
			Scores[i].Initial2 = 'A';
			Scores[i].Initial3 = 'A';

			WriteHighScores();

			return i;  // index of new high score
		}
	}
	return -1;
}

void HighScoreTable::InitialiseHighScores()
{
	Scores[0].Initial1 = 'O';
	Scores[0].Initial2 = 'N';
	Scores[0].Initial3 = 'E';
	Scores[0].Score = 500;

	Scores[1].Initial1 = 'T';
	Scores[1].Initial2 = 'W';
	Scores[1].Initial3 = 'O';
	Scores[1].Score = 400;

	Scores[2].Initial1 = 'T';
	Scores[2].Initial2 = 'H';
	Scores[2].Initial3 = 'R';
	Scores[2].Score = 300;

	Scores[3].Initial1 = 'F';
	Scores[3].Initial2 = 'O';
	Scores[3].Initial3 = 'U';
	Scores[3].Score = 200;

	Scores[4].Initial1 = 'F';
	Scores[4].Initial2 = 'I';
	Scores[4].Initial3 = 'V';
	Scores[4].Score = 100;
}

