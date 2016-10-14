#include <avr/pgmspace.h>
#include "wrist_states.h"
#include "pixel.h"
#include "FastLED.h"

// ******************************************************************
// Color Palette Functionality
// ******************************************************************

// Palettes
const TProgmemPalette16 PurplePalette_p PROGMEM = {
  CRGB::Purple,CRGB::Purple,CRGB::Purple,CRGB::Purple,
  CRGB::Purple,CRGB::Purple,CRGB::Purple,CRGB::Purple,
  CRGB::Purple,CRGB::Purple,CRGB::Purple,CRGB::Purple,
  CRGB::Purple,CRGB::Purple,CRGB::Purple,CRGB::Purple
};

const TProgmemPalette16 RedPalette_p PROGMEM = {
  CRGB::Red,CRGB::Red,CRGB::Red,CRGB::Red,
  CRGB::Red,CRGB::Red,CRGB::Red,CRGB::Red,
  CRGB::Red,CRGB::Red,CRGB::Red,CRGB::Red,
  CRGB::Red,CRGB::Red,CRGB::Red,CRGB::Red
};

const TBlendType Blending[5] = {
  LINEARBLEND,
  LINEARBLEND,
  LINEARBLEND,
  NOBLEND,
  NOBLEND
};

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

CRGBPalette16 backupPalette;
TBlendType    backupBlending;

void BackupPalette(void) {
  backupPalette = currentPalette;
  backupBlending = currentBlending;
}

void RestorePalette(void) {
  currentPalette = backupPalette;
  currentBlending = backupBlending;
}

void LoadPalette(uint8_t palette_id) {
  switch(palette_id) {
    case PALETTE_RAINBOW:
      currentPalette = RainbowColors_p;
      break;
    case PALETTE_PARTY:
      currentPalette = PartyColors_p;
      break;
    case PALETTE_HEAT:
      currentPalette = HeatColors_p;
      break;
    case PALETTE_PURPLE:
      currentPalette = PurplePalette_p;
      break;
    case PALETTE_RED:
      currentPalette = RedPalette_p;
      break;
  }
  currentBlending = Blending[palette_id];
}

// *************************************
// Useful pixel groups for animations
// *************************************

// All of the interior pixels with 1 pixel on all sides
const uint8_t PG_One_Neighbor[PG_ONE_NEIGHBOR_SIZE] PROGMEM = {
	10, 11, 12, 13, 14, 15, 16, 20, 21, 22, 23, 24, 29, 30, 31, 36, 40, 43, 46, 49, 52, 56, 61, 62, 63, 68, 69, 70, 71, 72, 76, 77, 78, 79, 80, 81, 82
};

// Just the exterior pixels
const uint8_t PG_Edge[PG_EDGE_SIZE] PROGMEM = {
  	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 26, 27, 38, 39, 44, 45, 50, 51, 58, 59, 74, 75, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 66, 65, 54, 53, 48, 47, 42, 41, 34, 33, 18, 17
};

// Just the top row pixels
const uint8_t PG_Top_Row[PG_TOP_ROW_SIZE] PROGMEM = {
	0, 17, 18, 33, 34, 41, 42, 47, 48, 53, 54, 65, 66, 83, 84 
};

uint8_t *Pixel_Groups[3] = {
  PG_One_Neighbor,
  PG_Edge,
  PG_Top_Row
};

// Get a pixel from a pixel group
uint8_t PG(uint8_t pg, uint8_t pixel) {
	return pgm_read_byte_near(Pixel_Groups[pg] + pixel);
}

// *************************************
// Lookup tables for pixel placement
// *************************************

// Table for decoding from row and column to pixel
const uint8_t row_col_tbl[NUM_ROW * NUM_COL] PROGMEM = {
	0, 17, 18, 255, 255, 255, 255, 255, 255, 255, 255, 255, 66, 83, 84,
	1, 16, 19, 33,  255, 255, 255, 255, 255, 255, 255, 65,  67, 82, 85,
	2, 15, 20, 32,  34,  255, 255, 255, 255, 255, 54,  64,  68, 81, 86,
	3, 14, 21, 31,  35,  41,  42,  47,  48,  53,  55,  63,  69, 80, 87,
	4, 13, 22, 30,  36,  40,  43,  46,  49,  52,  56,  62,  70, 79, 88,
	5, 12, 23, 29,  37,  39,  44,  45,  50,  51,  57,  61,  71, 78, 89,
	6, 11, 24, 28,  38,  255, 255, 255, 255, 255, 58,  60,  72, 77, 90,
	7, 10, 25, 27,  255, 255, 255, 255, 255, 255, 255, 59,  73, 76, 91,
	8, 9,  26, 255, 255, 255, 255, 255, 255, 255, 255, 255, 74, 75, 92,
};

// Table for decoding from pixel to column and row
const uint8_t pixel_tbl[PIXEL_CT] PROGMEM = {

	0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 
	0x80, 0x81, 0x71, 0x61, 0x51, 0x41, 0x31, 0x21, 
	0x11, 0x01, 0x02, 0x12, 0x22, 0x32, 0x42, 0x52, 
	0x62, 0x72, 0x82, 0x73, 0x63, 0x53, 0x43, 0x33, 
	0x23, 0x13, 0x24, 0x34, 0x44, 0x54, 0x64, 0x55, 
	0x45, 0x35, 0x36, 0x46, 0x56, 0x57, 0x47, 0x37, 
	0x38, 0x48, 0x58, 0x59, 0x49, 0x39, 0x2A, 0x3A, 
	0x4A, 0x5A, 0x6A, 0x7B, 0x6B, 0x5B, 0x4B, 0x3B, 
	0x2B, 0x1B, 0x0C, 0x1C, 0x2C, 0x3C, 0x4C, 0x5C, 
	0x6C, 0x7C, 0x8C, 0x8D, 0x7D, 0x6D, 0x5D, 0x4D, 
	0x3D, 0x2D, 0x1D, 0x0D, 0x0E, 0x1E, 0x2E, 0x3E, 
	0x4E, 0x5E, 0x6E, 0x7E, 0x8E };

// Get a row and column from a pixel #
void P2C(uint8_t p, uint8_t *r, uint8_t *c) {
	if (p >= PIXEL_CT) {
		*c = 255;
		*r = 255;
	} else {
		uint8_t idx = pgm_read_byte_near(pixel_tbl + p);
		*r = (idx >> 4) & 0xF;
		*c = idx & 0xF;
	}
}

// Get a pixel # from row and column
uint8_t C2P(uint8_t r, uint8_t c) {
	if ((r >= NUM_ROW) || (c >= NUM_COL))
		return 255;
	return pgm_read_byte_near(row_col_tbl + r * NUM_COL + c);
}

// Set a pixel from a pixel # and color index
void setPixel(uint8_t p, uint8_t c, uint8_t brightness) {
  if (p < PIXEL_CT)
	  leds[p] = ColorFromPalette(currentPalette, c, brightness, currentBlending);
}

// Format for Animations:
// Animation Header:
//	uint8_t frame_ct
//	uint8_t FPS
//	uint8_t palette
//	// Offset from start of animation data (LE 16 bit)
//	uint16_t frame_offsets[frame_ct]
//	frames[frame_ct]

// Frame format: 
//	uint8_t pixel_ct
//	pp_t pixels[pixel_ct]

// Packed Pixel from Matrix Editor
// If the pixel # is 0xFX, then it's a command instead
// typedef struct {
//	// Pixel index
//  	uint8_t pixel;
//	// Palette Color Index
//  	uint8_t color;
// } pp_t;

// Set the color of the companion wrist corsage
#define PP_CMD_SET_WRIST_COLOR	0

// Unpack array of pixels with palette into display array
// If undo is set, turn off the pixel instead of turning it on
void UnpackFrame(uint8_t frame, uint8_t *animation_data, bool undo, uint8_t color_offset) {
	uint8_t i, px, ct, pixel, color;

	uint16_t offset = pgm_read_word_near(animation_data + ((frame << 1) + 3));
	uint8_t size = pgm_read_byte_near(animation_data + offset);
	uint8_t *p = animation_data + offset + 1;

	// Unpack the frame
	for (i=0; i<2*size; i += 2) {
		pixel = pgm_read_byte_near(p + i);

		// It's a command
		if ((pixel & 0xF0) == 0xF0) {
			switch(pixel & 0xF) {
			// FIXME: Implement commands
			default:
				break;
			}
		// Otherwise, it's pixel data
		} else {
			// If undo is enabled, we turn off the LED
			if (undo) {
				leds[pixel] = CRGB::Black;
			// Colors are always maximum brightness...
			} else {
				color = pgm_read_byte_near(p + i + 1) + color_offset;
				leds[pixel] = ColorFromPalette(currentPalette, color, 0xFF, currentBlending);
			}
		}
	}
}

// Load animation frame from progmem
void LoadFrame(uint8_t frame_idx, uint8_t *animation_data, uint8_t color_offset) {
  uint8_t previous = frame_idx - 1;
	if (frame_idx == 0)
		previous = pgm_read_byte_near(animation_data) - 1;

	UnpackFrame(previous, animation_data, true, color_offset);
	UnpackFrame(frame_idx, animation_data, false, color_offset);
}
