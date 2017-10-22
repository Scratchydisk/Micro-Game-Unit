/*********************************************************************
Grotsoft Invaders

BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <SPI.h>
#include "SSD1306.h"
#include "SwarmManager.h"
#include <gfxfont.h>
#include <avr/pgmspace.h>
#include "Invaders.h"
#include "InvaderResources.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Display\Adafruit_SSD1306.h"
#include "HighScoreTable.h"

#if _DEBUG
// Debug level optimizations - do not affect debugging
#pragma GCC optimize ("Og")
#else
#pragma GCC optimize ("O3")
#endif


/**** SETTINGS FOR DEMO MODE ****/
// frames per second when running in demo mode
#define DEMO_FPS 20
// Seconds for base to travel full width of display
#define DEMO_BASE_PERIOD 2
// Base delta X per frame
#define DEMO_BASE_DELTA_X	(VIEW_WIDTH / (DEMO_FPS * DEMO_BASE_PERIOD))


/**** Object locations ****/
// Base starts in the middle of the display
uint8_t baseX = VIEW_WIDTH / 2 - BASE_WIDTH / 2;
#define BASE_Y (VIEW_HEIGHT - BASE_HEIGHT)

bool gameStarted = false;

int score = 0;
bool baseHit = false;
uint8_t lives = 3;
uint8_t AUDIO_PIN;

void newGame();
void newTurn();
void gameOver();
void showHighScores();

HighScoreTable highScoreTable;

void setup() {
	randomSeed(analogRead(0));

	// Initialise display

	// by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
	display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);  // initialize with the I2C addr 0x3C
	// init done
	display.setRotation(VIEW_ROTATION);

	// Initialise pins
	pinMode(START_BUTTON, INPUT_PULLUP);
	pinMode(UP_BUTTON, INPUT_PULLUP);
	pinMode(DOWN_BUTTON, INPUT_PULLUP);
	pinMode(LEFT_BUTTON, INPUT_PULLUP);
	pinMode(RIGHT_BUTTON, INPUT_PULLUP);

	// Start the game
	newGame();

	// Clear the buffer.
	display.display();

	gameStarted = true;
}

volatile uint8_t leftButton;
volatile uint8_t rightButton;
volatile uint8_t fireButton;

void newGame()
{
	// Wait for start button (D2)
	bool startPressed = false;
	uint16_t delayCount = 0;
	uint8_t currentScreen = 0;
	baseHit = false;
	AUDIO_PIN = REAL_AUDIO_PIN;

	// Show screens while waiting for start
	// Alternate individual screens with high score table
	// FIRE - starts game with sound
	// UP - starts game witn no sound
	while (!startPressed)
	{
		int startButton = digitalRead(START_BUTTON);
		int upButton = digitalRead(UP_BUTTON);
		int downButton = digitalRead(DOWN_BUTTON);

		// High score reset?
		if (upButton == LOW && downButton == LOW)
		{
			highScoreTable.InitialiseHighScores();
			continue;
		}

		// Silent start
		if (upButton == LOW && downButton == HIGH && startButton == HIGH)
		{
			// Disable audio out
			AUDIO_PIN = 0;
			startPressed = true;
		}

		// If start pressed...
		startPressed = startButton == LOW;

		if (delayCount == 0)
		{
			display.clearDisplay();

			switch (currentScreen)
			{
			case 0:
				display.drawBitmap(0, 0, splashImage, VIEW_WIDTH, VIEW_HEIGHT, WHITE);
				break;
			case 2:
				display.drawBitmap(0, 0, scoreSplashImage, VIEW_WIDTH, VIEW_HEIGHT, WHITE);
				break;
			case 1:
			case 3:
				showHighScores();
				break;
			}

			display.display();

			// Cycle through the screens - currently 4
			currentScreen = (currentScreen + 1) % 4;
		}

		delayCount = (delayCount + 100) % 4000;
		delay(100);
	}

	score = 0;
	lives = 3;

	// Initialise screen
	newTurn();
}

PlayerShot currentShot;

// Fires the base gun from current position
void fire()
{
	// Can only fire one shot at a time
	if (currentShot.active)
		return;

	currentShot.active = true;
	currentShot.x = baseX + BASE_WIDTH / 2;
	currentShot.y = BASE_Y - PLAYER_SHOT_HEIGHT;

	tone(AUDIO_PIN, 2000, 80);
}

void handleShot(uint8_t colour)
{
	if (!currentShot.active)
		return;

	if (colour == WHITE)
		currentShot.y--;

	display.drawLine(currentShot.x, currentShot.y, currentShot.x, currentShot.y - PLAYER_SHOT_HEIGHT, colour);

	currentShot.active = currentShot.y >= 0;
}


void RunDemo()
{

}


void newTurn()
{
	swarmShotOdds = 25;

	currentShot.active = false;

	for (int i = 0; i < MAX_SWARM_SHOTS; i++)
	{
		swarmShots[i].active = false;
	}

	// Draw static items
	display.clearDisplay();

	//	myDrawBitmap_P(6, VIEW_HEIGHT - 10 - SHIELD_HEIGHT, shield, SHIELD_WIDTH, SHIELD_HEIGHT, WHITE);
	//	myDrawBitmap_P(25, VIEW_HEIGHT - 10 - SHIELD_HEIGHT, shield, SHIELD_WIDTH, SHIELD_HEIGHT, WHITE);
	//	myDrawBitmap_P(44, VIEW_HEIGHT - 10 - SHIELD_HEIGHT, shield, SHIELD_WIDTH, SHIELD_HEIGHT, WHITE);

	createSwarm();
}

void invaderHitDetection()
{
	if (!currentShot.active)
		return;

	// work out offset into swarm
	int shotX = currentShot.x - swarm.x;
	// tip of shot counts but we track base
	int shotY = currentShot.y - PLAYER_SHOT_HEIGHT - swarm.y;

	/*if (shotY >= swarmHeightPixels || shotY < 0)
		return;

	if (shotX < 0 || shotX >= swarmWidthPixels)
		return;
*/
	int row = shotY / SWARM_CELL_HEIGHT;
	int col = shotX / SWARM_CELL_WIDTH;

	if (row < 0 || row >= SWARM_HEIGHT)
		return;

	if (col < 0 || col >= SWARM_WIDTH)
		return;

	if (swarm.state[row][col] == STATE_ALIVE)
	{
		swarm.state[row][col] = STATE_BLOWN_UP;
		currentShot.active = false;
		score += swarm.invaders[row][col];
		tone(AUDIO_PIN, 500, 80);
	}
}
/************* END SWARM MANAGEMENT **************/

void animateBase()
{
	int y = VIEW_HEIGHT - BASE_HEIGHT;
	int w = BASE_WIDTH;
	int h = BASE_HEIGHT;

	bool goingRight = true;
	bool invImage1 = true;

	while (1)
	{
		//if (invImage1)
		//	myDrawBitmap(baseX / 2, 10, inv20_1, INV_20_WIDTH, INV_20_HEIGHT, WHITE);
		//else
		//	myDrawBitmap(baseX / 2, 10, inv20_2, INV_20_WIDTH, INV_20_HEIGHT, WHITE);

		myDrawBitmap_P(baseX, y, baseIcon, BASE_WIDTH, BASE_HEIGHT, WHITE);
		display.display();

		myDrawBitmap_P(baseX, y, baseIcon, BASE_WIDTH, BASE_HEIGHT, BLACK);
		//if (invImage1)
		//	myDrawBitmap(baseX / 2, 10, inv20_1, INV_20_WIDTH, INV_20_HEIGHT, BLACK);
		//else
		//	myDrawBitmap(baseX / 2, 10, inv20_2, INV_20_WIDTH, INV_20_HEIGHT, BLACK);

		if (baseX % 4 == 0)
			invImage1 = !invImage1;

		delay(50);

		if (goingRight)
		{
			if (baseX < VIEW_WIDTH - w)
				baseX += DEMO_BASE_DELTA_X;
			else
				goingRight = false;
		}
		else
		{
			if (baseX > 0)
				baseX -= DEMO_BASE_DELTA_X;
			else
				goingRight = true;
		}
	}
}


/************ GAME CODE ********/



void updateText(uint16_t color)
{
	display.setTextColor(color);
	display.setTextSize(1);
	display.setCursor(0, 0);
	display.print(score);
}


void loop() {
	while (1)
	{
		if (!gameStarted)
			continue;

		leftButton = digitalRead(LEFT_BUTTON);
		rightButton = digitalRead(RIGHT_BUTTON);
		fireButton = digitalRead(START_BUTTON);

		if (leftButton == LOW && baseX > 0)
			baseX--;
		if (rightButton == LOW && baseX < (VIEW_WIDTH - BASE_WIDTH))
			baseX++;
		if (fireButton == LOW)
			fire();

		// Draw base
		if (baseHit)
		{
			tone(AUDIO_PIN, 100, 80);
			baseHit = false;
			score < 50 ? 0 : score -= 50;
			myDrawBitmap_P(baseX, BASE_Y, invaderExplode, INV_EXPLODE_WIDTH, INV_EXPLODE_HEIGHT, WHITE);
		}
		else
			myDrawBitmap_P(baseX, BASE_Y, baseIcon, BASE_WIDTH, BASE_HEIGHT, WHITE);

		// Draw lives
		for (uint8_t x = VIEW_WIDTH - BASE_WIDTH, l = lives; l > 0; l--, x -= BASE_WIDTH + 1)
		{
			myDrawBitmap_P(x, 0, baseIcon, BASE_WIDTH, BASE_HEIGHT, WHITE);
		}

		calculateSwarmSize();
		drawSwarm(WHITE);
		handleShot(WHITE);
		canSwarmShoot();
		handleSwarmShots(WHITE);
		updateText(WHITE);

		display.display();

		display.clearDisplay();

		// Hit detection here (so a dead invader is removed above)
		invaderHitDetection();
		baseHitDetection();

		if (lives == 0)
		{
			gameOver();
		}

		moveSwarm();

		delay(10);
	}
}

void showHighScores()
{
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0, 10);
	display.println("Top Scores");

	display.setCursor(10, 40);
	for (uint8_t i = 0; i < NUM_HIGH_SCORES; i++)
	{
		HighScore hs = highScoreTable.Scores[i];
		display.print(hs.Initial1); 
		display.print(hs.Initial2);
		display.print(hs.Initial3);
		display.print(' ');
		display.println(hs.Score);
		display.println();
	}
}

void gameOver()
{
	display.setTextColor(WHITE);
	display.setTextSize(1);
	display.setCursor(10, VIEW_HEIGHT / 2);
	display.println("GAME OVER");
	for (uint8_t i = 1; i < 4; i++)
	{
		delay(400);
		display.invertDisplay(true);
		delay(400);
		display.invertDisplay(false);
	}

	// Is this a high score?
	int index = highScoreTable.IsNewHighScore(score);
	showHighScores();
	delay(4000);

	newGame();
}
