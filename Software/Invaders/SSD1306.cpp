// 
// 
// 

#include "SSD1306.h"
#include <avr/pgmspace.h>
#include <Adafruit_GFX.h>
#include "Display\Adafruit_SSD1306.h"


Adafruit_SSD1306 display(OLED_RESET);


// Allows drawing of bitmaps that don't have to
// be multiples of 8 in each dimension.
// Total number of pixes does not need to
// be a multiple of 8.
void myDrawBitmap(int16_t x, int16_t y,
	uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {

	int16_t numPixels = w * h;
	int16_t byteWidth = (numPixels + 7) / 8; // Round up to whole bytes

	uint8_t byte = 0;
	uint8_t xOffset = 0;
	uint16_t pixelCount = 0;	// number of pixels processed so far

								// Iterate through all full bytes
	for (int byteIndex = 0; byteIndex < byteWidth; byteIndex++)
	{
		byte = bitmap[byteIndex];
		// Iterate over each bit in byte
		for (uint8_t bit = 0; bit < 8; bit++)
		{
			if (byte & 0x80)
				display.writePixel(x + xOffset, y, color);

			byte <<= 1;

			if (xOffset < w - 1)
				xOffset++;
			else
			{
				xOffset = 0;
				y++;
			}
			// Stop processing when all image pixels processed
			if (++pixelCount == numPixels)
				break;
		}
	}
}

// Allows drawing of bitmaps that don't have to
// be multiples of 8 in each dimension.
// Total number of pixes does not need to
// be a multiple of 8.
// Bitmap data in PROGMEM.

void myDrawBitmap_P(int16_t x, int16_t y,
	const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {

	int16_t numPixels = w * h;
	int16_t byteWidth = (numPixels + 7) / 8; // Round up to whole bytes

	uint8_t byte = 0;
	uint8_t xOffset = 0;
	uint16_t pixelCount = 0;	// number of pixels processed so far

								// Iterate through all full bytes
	for (int byteIndex = 0; byteIndex < byteWidth; byteIndex++)
	{
		byte = pgm_read_byte_near(bitmap + byteIndex);
		// Iterate over each bit in byte
		for (uint8_t bit = 0; bit < 8; bit++)
		{
			if (byte & 0x80)
				display.writePixel(x + xOffset, y, color);

			byte <<= 1;

			if (xOffset < w - 1)
				xOffset++;
			else
			{
				xOffset = 0;
				y++;
			}
			// Stop processing when all image pixels processed
			if (++pixelCount == numPixels)
				break;
		}
	}
}