#include <avr/pgmspace.h>

#define NUM_COL  	15
#define NUM_ROW 	9
#define PIXEL_CT	93

// Table for decoding from row and column to pixel
const uint8_t row_col_tbl[NUM_ROW][NUM_COL] PROGMEM = {
	{ 0, 17, 18, 255, 255, 255, 255, 255, 255, 255, 255, 255, 66, 83, 84 },
	{ 1, 16, 19, 33, 255, 255, 255, 255, 255, 255, 255, 65, 67, 82, 85 },
	{ 2, 15, 20, 32, 34, 255, 255, 255, 255, 255, 54, 64, 68, 81, 86 },
	{ 3, 14, 21, 31, 35, 41, 42, 47, 48, 53, 55, 63, 69, 80, 87 },
	{ 4, 13, 22, 30, 36, 40, 43, 46, 49, 52, 56, 62, 70, 79, 88 },
	{ 5, 12, 23, 29, 37, 39, 44, 45, 50, 51, 57, 61, 71, 78, 89 },
	{ 6, 11, 24, 28, 38, 255, 255, 255, 255, 255, 58, 60, 72, 77, 90 },
	{ 7, 10, 25, 27, 255, 255, 255, 255, 255, 255, 255, 59, 73, 76, 91 },
	{ 8, 9, 26, 255, 255, 255, 255, 255, 255, 255, 255, 255, 74, 75, 92 },
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
	int idx = pgm_read_byte_near(&(pixel_tbl[p]));
	if (idx == 0xFFFF){
		*c = 255;
		*r = 255;
	} else {
		*c = (idx >> 4) & 0xFF;
		*r = idx & 0xFF;
	}
}

// Get a pixel # from row and column
uint8_t C2P(uint8_t r, uint8_t c) {
	return pgm_read_byte_near(&(row_col_tbl[r][c]));
}

// Format for Animations:
// Animation Header:
//	uint8_t frame_ct
//	uint8_t FPS
//	// Offset from start of animation data (BE 16 bit)
//	uint16_t frame_offsets[]
//	frames[]

// Frames[x]: 
//	uint8_t size
//	pp_t[]

// Packed Pixel from Matrix Editor
// If the pixel # is 0xFX, then it's a command instead
// typedef struct {
//  uint8_t pixel;
//  uint8_t brightness:4;
//  uint8_t color:4;
// } pp_t;

// Pixel off
#define PP_CMD_BLACK		0
// Pixel full on
#define PP_CMD_WHITE		1

// Set the color of the companion wrist corsage
#define PP_CMD_SET_WRIST_COLOR	2

// Unpack array of pixels with palette into display array
void UnpackFrame(uint8_t px_ct, uint8_t *p) {
	uint8_t i, px, ct;
	// For now, black out the frame first
	for (ct=0; ct<PIXEL_CT; ct++)
		leds[ct] = CRGB::Black;

	// Unpack the frame
	for (i=0; i<2*px_ct; i += 2) {
		// It's a command
		if (p[i] & 0xF0 == 0xF0) {
			switch(p[i] & 0xF) {
			case PP_CMD_BLACK:
				leds[pixel] = CRGB::Black;
				break;
			case PP_CMD_WHITE:
				leds[pixel] = CRGB::White;
				break;
			default:
				break;
			}
		// Otherwise, it's pixel data
		} else {
			// Color the pixel
			uint8_t pixel = p[i];
			uint8_t color = p[i+1] & 0xF;
			uint8_t brightness = (p[i+1] >> 4) & 0xF;
			leds[pixel] = ColorFromPalette(currentPalette, color, brightness, currentBlending);
		}
	}
}

// Load animation frame
void LoadFrame(uint8_t frame_idx, uint8_t *animation_data) {
	// FIXME: Put the data into PROGMEM
	uint16_t offset = ((uint16_t)(animation_data[frame_idx * 2 + 2]) << 8) | animation_data[frame_idx * 2 + 3];
	uint8_t size = animation_data[offset];
	UnpackFrame(size, &(animation_data[offset + 1]));
}
