// SSD1306.h

// My extensions to the Adafruit stuff

#ifndef _SSD1306_h
#define _SSD1306_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <avr/pgmspace.h>
#include <Adafruit_GFX.h>
#include "Display\Adafruit_SSD1306.h"

#define OLED_RESET 4
extern Adafruit_SSD1306 display;


#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


// Allows drawing of bitmaps that don't have to
// be multiples of 8 in each dimension.
// Total number of pixes does not need to
// be a multiple of 8.
// Bitmap data in PROGMEM.

void myDrawBitmap_P(int16_t x, int16_t y,
	const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);


#endif

