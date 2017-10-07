// 
// 
// 

#include "SSD1306.h"
#include "SwarmManager.h"
#include "InvaderResources.h"

Swarm swarm;
PlayerShot swarmShots[MAX_SWARM_SHOTS];
uint8_t swarmShotOdds;

/************* SWARM MANAGEMENT **************/

// Which of the two invader images to use
bool invaderFirstImage = true;
bool movingLeft = true;


void createSwarm()
{
	for (uint8_t row = 0; row < SWARM_HEIGHT; row++)
	{
		for (uint8_t col = 0; col < SWARM_WIDTH; col++)
		{
			// Type of invader
			if (row < 1)
			{
				swarm.invaders[row][col] = 30;
				swarm.deltaX[row][col] = (SWARM_CELL_WIDTH - INV_30_WIDTH) / 2;
			}
			else if (row < 3)
			{
				swarm.invaders[row][col] = 20;
				swarm.deltaX[row][col] = (SWARM_CELL_WIDTH - INV_20_WIDTH) / 2;
			}
			else
			{
				swarm.invaders[row][col] = 10;
				swarm.deltaX[row][col] = (SWARM_CELL_WIDTH - INV_10_WIDTH) / 2;
			}
			swarm.state[row][col] = STATE_ALIVE;
		}
	}

	swarm.x = 0;
	swarm.y = 10;
}

uint8_t swarmWidthPixels;
uint8_t swarmHeightPixels;
uint8_t swarmWidthCells;
uint8_t swarmHeightCells;
uint8_t swarmLeftPixels;
uint8_t swarmRightPixels;
uint8_t swarmTopPixels;
uint8_t swarmBottomPixels;

uint8_t swarmLeftColumn;
uint8_t swarmRightColumn;
uint8_t swarmTopRow;
uint8_t swarmBottomRow;

// Pixel dimensions of the swarm.
// Allowing for fully empty rows and columns
void calculateSwarmSize()
{
	swarmLeftColumn = SWARM_WIDTH;	// assume no invaders
	swarmRightColumn = 0;
	swarmTopRow = SWARM_HEIGHT;
	swarmBottomRow = 0;

	// How many complete columns?
	for (uint8_t col = 0; col < SWARM_WIDTH; col++)
		for (uint8_t row = 0; row < SWARM_HEIGHT; row++)
		{
			if (swarm.state[row][col] == STATE_ALIVE)
			{
				swarmTopRow = swarmTopRow > row ? row : swarmTopRow;
				swarmBottomRow = swarmBottomRow < row ? row : swarmBottomRow;

				swarmLeftColumn = swarmLeftColumn > col ? col : swarmLeftColumn;
				swarmRightColumn = swarmRightColumn < col ? col : swarmRightColumn;
			}
		}

	swarmWidthCells = swarmRightColumn - swarmLeftColumn + 1;
	swarmWidthPixels = swarmWidthCells * SWARM_CELL_WIDTH;
	swarmLeftPixels = swarmLeftColumn * SWARM_CELL_WIDTH;
	swarmRightPixels = swarmLeftPixels + swarmWidthPixels;

	swarmHeightCells = swarmBottomRow - swarmTopRow + 1;
	swarmHeightPixels = swarmHeightCells * SWARM_CELL_HEIGHT;
	swarmTopPixels = swarmTopRow * SWARM_CELL_HEIGHT;
	swarmBottomPixels = swarmTopPixels + swarmHeightPixels;

	//Serial.print("swc "); Serial.print(swarmWidthCells); Serial.print(" shc "); Serial.println(swarmHeightCells);
}

// Will move down every 4 times left/right travel
uint8_t swarmTravelCount = 0;
uint8_t moveCount = 0;

void moveSwarm()
{
	if (movingLeft)
	{
		if (swarm.x + swarmLeftPixels > 0)
			--swarm.x;
		else
		{
			movingLeft = false;
			++swarm.x;
		}
	}
	else  // moving right
	{
		if ((swarm.x + swarmRightPixels) < (VIEW_WIDTH - 1))
			++swarm.x;
		else
		{
			swarmTravelCount++;
			movingLeft = true;
			--swarm.x;
		}
	}

	// Change animation every 8 moves
	if (++moveCount % 8 == 0)
	{
		moveCount = 0;
		invaderFirstImage = !invaderFirstImage;
		tone(AUDIO_PIN, 200, 100);
	}

	// Move down every two traverses
	if (swarmTravelCount == 2)
	{
		swarmTravelCount = 0;
		swarm.y++;
	}
}

void drawSwarm(uint8_t colour)
{
	for (int row = 0; row < SWARM_HEIGHT; row++)
	{
		for (int col = 0; col < SWARM_WIDTH; col++)
		{
			int x = swarm.x + swarm.deltaX[row][col] + SWARM_CELL_WIDTH * col;
			int y = swarm.y + SWARM_CELL_HEIGHT * row;

			if (swarm.state[row][col] == STATE_DEAD)
				continue;

			// Just killed
			if (swarm.state[row][col] == STATE_BLOWN_UP)
			{
				myDrawBitmap_P(x, y, invaderExplode, INV_EXPLODE_WIDTH, INV_EXPLODE_HEIGHT, colour);
				swarm.state[row][col] = STATE_DEAD;
				continue;
			}

			if (swarm.invaders[row][col] == 10)
				myDrawBitmap_P(x, y, invaderFirstImage ? invader10_1 : invader10_2, INV_10_WIDTH, INV_10_HEIGHT, colour);
			else if (swarm.invaders[row][col] == 20)
				myDrawBitmap_P(x, y, invaderFirstImage ? invader20_1 : invader20_2, INV_20_WIDTH, INV_20_HEIGHT, colour);
			else if (swarm.invaders[row][col] == 30)
				myDrawBitmap_P(x, y, invaderFirstImage ? invader30_1 : invader30_2, INV_30_WIDTH, INV_30_HEIGHT, colour);
		}
	}
}

bool useBombImage1 = true;
uint8_t bombImageCounter = 0;

// Swarm can shoot if:
// - it rolls a 1 out of "swarmShotOdds" chance
// - there's a free place to track it (< MAX_SWARM_SHOTS in play)
void canSwarmShoot()
{
	// This is called once per frame, so check
	// for when we change the bomb image
	if (++bombImageCounter > 1)
	{
		useBombImage1 = !useBombImage1;
		bombImageCounter = 0;
	}

	// for each column, check the odds and fire if successful
	for (uint8_t col = swarmLeftColumn; col <= swarmRightColumn; col++)
	{
		uint8_t dieRoll = random(1, swarmShotOdds);
		if (dieRoll != 1)
			continue;

		// Find an empty swarm shot
		for (uint8_t i = 0; i < MAX_SWARM_SHOTS; i++)
		{
			if (!swarmShots[i].active)
			{
				swarmShots[i].active = true;
				// Centre the shot on the column
				swarmShots[i].x = swarm.x + (col * SWARM_CELL_WIDTH) + SWARM_CELL_WIDTH / 2;
				// Make the shot come from the lowest alien in the column
				for (uint8_t row = swarmBottomRow; row >= swarmTopRow; row--)
				{
					if (swarm.state[row][col] == STATE_ALIVE)
					{
						swarmShots[i].y = swarm.y + (row + 1) * SWARM_CELL_HEIGHT;
						break;
					}
				}

				break;
			}
		}
	}
}

void handleSwarmShots(uint8_t color)
{
	for (int s = 0; s < MAX_SWARM_SHOTS; s++)
	{
		if (!swarmShots[s].active)
			continue;

		if (color == WHITE)
			swarmShots[s].y++;

		if (useBombImage1)
			myDrawBitmap_P(swarmShots[s].x, swarmShots[s].y, alienBomb1, ALIEN_BOMB_WIDTH, ALIEN_BOMB_HEIGHT, color);
		else
			myDrawBitmap_P(swarmShots[s].x, swarmShots[s].y, alienBomb2, ALIEN_BOMB_WIDTH, ALIEN_BOMB_HEIGHT, color);

	}
}

void baseHitDetection()
{
	for (int s = 0; s < MAX_SWARM_SHOTS; s++)
	{
		if (!swarmShots[s].active)
			continue;

		// Has a bomb hit a base shot?
		// Consider width of bomb image and 2 pixel margin on vertical (because they can pass without being equal)
		if (currentShot.x >= swarmShots[s].x && currentShot.x <= swarmShots[s].x + ALIEN_BOMB_WIDTH
			&& currentShot.y + PLAYER_SHOT_HEIGHT >= swarmShots[s].y + ALIEN_BOMB_HEIGHT - 1 
			&& currentShot.y + PLAYER_SHOT_HEIGHT <= swarmShots[s].y + ALIEN_BOMB_HEIGHT + 1)
		{
			currentShot.active = false;
			swarmShots[s].active = false;
			score += SCORE_HIT_BOMB;
			tone(AUDIO_PIN, 4000, 80);
			continue;
		}

		if (swarmShots[s].y > VIEW_HEIGHT)
		{
			// Miss
			swarmShots[s].active = false;
			break;
		}

		if (swarmShots[s].y > VIEW_HEIGHT - BASE_HEIGHT - ALIEN_BOMB_HEIGHT)
		{
			// Shot is passing through base's horizontal area
			if (swarmShots[s].x > baseX && swarmShots[s].x < baseX + BASE_WIDTH)
			{
				// A hit
				swarmShots[s].active = false;
				baseHit = true;
			}
		}
	}
}
