// SwarmManager.h

#ifndef _SWARMMANAGER_h
#define _SWARMMANAGER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Invaders.h"

#if _DEBUG
// Debug level optimizations - do not affect debugging
#pragma GCC optimize ("Og")
#else
#pragma GCC optimize ("O3")
#endif

#define MAX_SWARM_SHOTS 4

// Holds the shots fired by the swarm
extern PlayerShot swarmShots[MAX_SWARM_SHOTS];


#define SWARM_WIDTH 4
#define SWARM_HEIGHT 5
#define SWARM_CELL_WIDTH 14
#define SWARM_CELL_HEIGHT 11
#define SWARM_COUNT (SWARM_WIDTH * SWARM_HEIGHT)

#define STATE_DEAD 0
#define STATE_ALIVE 1
#define STATE_BLOWN_UP 2

struct Swarm
{
	int x;
	int y;

	uint8_t invaders[SWARM_HEIGHT][SWARM_WIDTH];		// invader id (value, 10, 20 or 30)
	uint8_t state[SWARM_HEIGHT][SWARM_WIDTH];	// 0 dead, 1 alive, 2 just killed
	uint8_t deltaX[SWARM_HEIGHT][SWARM_WIDTH];
};

extern Swarm swarm;
extern uint8_t swarmShotOdds;

void drawSwarm(uint8_t colour);
void moveSwarm();
void calculateSwarmSize();
void createSwarm();
void canSwarmShoot();
void handleSwarmShots(uint8_t color);
void baseHitDetection();

#endif

