#include "wrist_states.h"
#include "pixel.h"
#include "animations.h"

// All animation functions have three functions:
// 	Initialization - Sets variables
//		- cfg - Configuration for different modes of the same animation
//		- Mode_steps - Number of steps to complete one cycle of this animation
//			A value of 0 is infinite
//		- ms - ms per frame for the animation
//			0 indicates that the animation should run as fast as possible
//	Animation - Performs the animation at the current step
//	Switch - Returns true when the animation can be switched

uint8_t mode_cfg = 0;
uint8_t mode_steps = 0;
uint16_t ms = 0;

#define FPS(x) (1000/x)

// This is a flag for allowing the mode to change
uint8_t mode_change_allowed = 1;
// This is a flag for when a button requests a mode change
uint8_t mode_change_requested = 0;

// Function for blanking the LEDs
void blankLEDs(void) {
	for (uint8_t i = 0; i < NUM_LEDS; i++) {
  		leds[i] = CRGB::Black;
  	}
}

// **********************************************
// * Turns the Bowtie completely off
// **********************************************
void init_TieOff(uint8_t cfg) {
	mode_steps = 1;	
	ms = 0;
	mode_cfg = cfg;
}

void anim_TieOff(uint8_t step) {
	if (step == 0) {
		blankLEDs();
	}
}

bool switch_TieOff(uint8_t step) {
	return true;
}

// **********************************************
// * Performs the outline animation
// * Configurable based on the integer passed in
// **********************************************
void init_Outline(uint8_t cfg) {
	mode_cfg = cfg;
	mode_steps = PG_EDGE_SIZE;
	ms = FPS(50);
	// Set the default palette	
}

void anim_Outline(uint8_t step) {
	uint8_t last_step = 0;
	uint8_t px;

	// This is the always on mode
	if (mode_cfg == 255) {
  		for(uint8_t i = 0; i<PG_EDGE_SIZE; i++) {
			px = PG(PG_EDGE, i);
			setPixel(px, palette_step, 255);
		}
		palette_step++;
		return;
	} 

	// Get the previous step
	if (step == 0)
		last_step = mode_steps;
	else
		last_step = step - 1;

	// Turn off last LEDs
	blankLEDs();

	// Turn on current LEDs
	px = PG(PG_EDGE, step);
	setPixel(px, palette_step, 255);

	// Turn on the 4x LEDs
	if (mode_cfg == 4) {
		if (step < 11) {
			px = PG(PG_EDGE, step + 33);
			setPixel(px, palette_step, 255);
		} else {
			px = PG(PG_EDGE, step - 11);
			setPixel(px, palette_step, 255);
		}

		if (step < 33) {
			px = PG(PG_EDGE, step + 11);
			setPixel(px, palette_step, 255);
		} else {
			px = PG(PG_EDGE, step - 33);
			setPixel(px, palette_step, 255);
		}
	}
	
	// Turn on the 2x LEDs
	if ((mode_cfg == 2) || (mode_cfg == 4)) {
		if (step < 22) {
			px = PG(PG_EDGE, step + 22);
			setPixel(px, palette_step, 255);
		} else {
			px = PG(PG_EDGE, step - 22);
			setPixel(px, palette_step, 255);
		}
	}

	// Increment through the palette
	palette_step++;
	return;
}

bool switch_Outline(uint8_t step) {
	// We return true when we're at step 0 or we're in always on mode
	if ((state == 0) || (mode_cfg == 255))
		return true;
	return false;
}

// **********************************************
// * "Night Rider" Animation
// **********************************************
void init_Nightrider(uint8_t cfg) {
	mode_steps = 80;
	ms = FPS(50);
	cfg = 0;
}

void anim_Nightrider(uint8_t step) {
	const uint8_t idxs[15] = {4,13,22,30,36,40,43,46,49,52,56,62,70,79,88};
	uint8_t idx = 0;
	uint8_t last;

	if (step < 15) {
		// Clear the matrix
		blankLEDs();
  
		// Right to left
		setPixel(idxs[step], palette_step, 255);
		setPixel(idxs[step] - 1, palette_step, 255);
		setPixel(idxs[step] + 1, palette_step, 255);
	}

	// We wait for 25 steps
  	if ((step >= 15) && (step < 40)) {
		// Do nothing...
		;;
	}

	// Increment the color
	if (step == 40)
		palette_step++;

	// Go back
	if ((step >= 40) && (step < 55)) {
		// Clear the matrix
		blankLEDs();

		// Left to Right
		idx = 29 - (step - 25);
		setPixel(idxs[idx], palette_step, 255);
		setPixel(idxs[idx] - 1, palette_step, 255);
		setPixel(idxs[idx] + 1, palette_step, 255);
	}

	// We wait for 25 steps
	if (step >= 55) {
		// Do nothing...
	}

	// Increment the color
	if (step == 79)
		palette_step++;
}

bool switch_Nightrider(uint8_t step) {
	if (step == 0)
		return true;
	return false;
}

// **********************************************
// * "Pinwheel" Dots Animation
// **********************************************
void init_Pinwheel(int cfg) {
	mode_cfg = cfg;
	mode_steps = 32;
	ms = FPS(4);
}

uint8_t pw_Center = 0;
uint8_t pw_Color = 0;
uint8_t pw_Edges[4];
uint8_t pw_Quads[4];
void anim_Pinwheel(uint8_t step) {
	// Select a new pinwheel
	if (step == 0) {
		blankLEDs();

		pw_Center = PG(PG_ONE_NEIGHBOR, random(0, PG_ONE_NEIGHBOR_SIZE));
		pw_Color = random(0, 255);

		uint8_t row, col;
		P2C(pw_Center, &row, &col);

		pw_Edges[0] = C2P(row - 1, col);
		pw_Edges[1] = C2P(row + 1, col);
		pw_Edges[2] = C2P(row, col - 1);
		pw_Edges[3] = C2P(row, col + 1);

		pw_Quads[0] = C2P(row - 1, col - 1);
		pw_Quads[1] = C2P(row - 1, col + 1);
		pw_Quads[2] = C2P(row + 1, col - 1);
		pw_Quads[3] = C2P(row + 1, col + 1);
	}

	setPixel(pw_Center, pw_Color, 255);
	if (step % 2) {
		for (uint8_t i=0; i<4; i++) {
			leds[pw_Quads[i]] = CRGB::Black;
			setPixel(pw_Edges[i], pw_Color + 128, 128);
		}
	} else {
		for (uint8_t i=0; i<4; i++) {
			setPixel(pw_Quads[i], pw_Color + 128, 128);
			leds[pw_Edges[i]] = CRGB::Black;
		}
	}
}

void switch_Pinwheel(uint8_t step) {
	if (step == 0)
		return true;
	return false;
}

// **********************************************
// * "Whiskers" Animation
// **********************************************
void init_Whiskers(int cfg) {

}

void anim_Whiskers(uint8_t step) {

}

void switch_Whiskers(uint8_t step) {
	if (step == 0)
		return true;
	return false;
}

// **********************************************
// * "Matrix" Animation
// **********************************************

// 8 simultaneous streaks
uint8_t matrix_vals[8];

void init_Matrix(int cfg) {
	uint8_t r, c, px;

	mode_cfg = cfg;
	mode_steps = 255;
	ms = FPS(10);

	// Set up the matrix
	for (uint8_t i=0; i<8; i++) {
		// Pick a random pixel
		uint8_t px = random(0, PIXEL_CT);
		P2C(px, &r, &c);
		matrix_vals[i] = (r << 4) | c;
	}
}

void anim_Matrix(uint8_t step) {
	uint8_t px, r, c;
	// Clear the screen
	blankLEDs();

	for (uint8_t i=0; i<8; i++) {
		r = (matrix_vals[i] >> 4);
		c = (matrix_vals[i] & 0xF);
		r++;

		// Get a new pixel if we went off
		if (r >= NUM_ROW + 2) {
			px = random(0, PIXEL_CT);
			P2C(px, &r, &c);
		}

		px = C2P(r, c);
		if (px != 255)
			setPixel(px, palette_step, 255);

		px = C2P(r - 1, c);
		if (px != 255)
			setPixel(px, palette_step, 128);
		
		px = C2P(r - 2, c);
		if (px != 255)
			setPixel(px, palette_step, 64);

		// Set the row and column values back
		matrix_vals[i] = (r << 4) | c;
	}
	palette_step++;
}

void switch_Matrix(uint8_t step) {
	return true;
}

// **********************************************
// * "Rainbow" Animation
// **********************************************
void init_Rainbow(int cfg) {
	mode_cfg = cfg;
	mode_steps = 255;
	ms = 0;
}

void anim_Rainbow(uint8_t step) {
	for (uint8_t i=0; i<PIXEL_CT; i++) {
		// Half brightness
		setPixel(i, step + i, 128);
	}
}

void switch_Rainbow(uint8_t step) {
	return true;
}


// **********************************************
// * Play an Animation from BowtieEd
// **********************************************
const uint8_t BT_Animations[1][128] PROGMEM = {
	{
		0x4, 0x2, 0x0,
		0xb, 0x0, 0x44, 0x0, 0x5f, 0x0, 0x6a, 0x0, 
		0x1c, 0x2, 0xc0, 0x3, 0xc0, 0x4, 0xc0, 0x5, 0xc0, 0x6, 0xc0, 0xe, 0xc0, 0x16, 0xc0, 0x1f, 0xc0, 0x22, 0xc0, 0x23, 0xc0, 0x24, 0xc0, 0x25, 0xc0, 0x26, 0xc0, 0x2b, 0xc0, 0x2d, 0xc0, 0x2e, 0xc0, 0x2f, 0xc0, 0x31, 0xc0, 0x36, 0xc0, 0x37, 0xc0, 0x38, 0xc0, 0x39, 0xc0, 0x3a, 0xc0, 0x3e, 0xc0, 0x45, 0xc0, 0x47, 0xc0, 0x4d, 0xc0, 0x51, 0xc0,
		0xd, 0x2, 0xc0, 0x3, 0xc0, 0x4, 0xc0, 0x5, 0xc0, 0x6, 0xc0, 0xe, 0xc0, 0x16, 0xc0, 0x1f, 0xc0, 0x22, 0xc0, 0x23, 0xc0, 0x24, 0xc0, 0x25, 0xc0, 0x26, 0xc0,
		0x5, 0x2b, 0xc0, 0x2d, 0xc0, 0x2e, 0xc0, 0x2f, 0xc0, 0x31, 0xc0,
		0xa, 0x36, 0xc0, 0x37, 0xc0, 0x38, 0xc0, 0x39, 0xc0, 0x3a, 0xc0, 0x3e, 0xc0, 0x45, 0xc0, 0x47, 0xc0, 0x4d, 0xc0, 0x51, 0xc0
	},
};

void init_BTAnimation(uint8_t cfg) {
	// The cfg indicates which animation to play
	mode_cfg = cfg;

	// Set the number of frames from the animation data
	mode_steps = pgm_read_byte_near(BT_Animations[cfg]);

	// Set the FPS from the animation data
	ms = FPS(pgm_read_byte_near(BT_Animations[cfg] + 1));
	
	// Set the palette from the animation data
	uint8_t pal = pgm_read_byte_near(BT_Animations[cfg] + 2);
}

void anim_BTAnimation(uint8_t step) {
   LoadFrame(step, BT_Animations[mode_cfg]);
}

bool switch_BTAnimation(uint8_t step) {
	return true;
}

// ****************************************************************************
// Animation modes
// ****************************************************************************
typedef void (*Animation_Init_t)(uint8_t);
Animation_Init_t Animation_Inits[8] = {
	init_TieOff,
	init_Outline,
	init_Nightrider,
	init_Whiskers,
	init_Pinwheel,
	init_Matrix,
	init_BTAnimation,
	init_Rainbow
};

typedef void (*Animation_Func_t)(uint8_t);
Animation_Func_t Animation_Funcs[8] = {
	anim_TieOff,
	anim_Outline,
	anim_Nightrider,
	anim_Whiskers,
	anim_Pinwheel,
	anim_Matrix,
	anim_BTAnimation,
	anim_Rainbow
};

typedef bool (*Animation_Switch_t)(uint8_t);
Animation_Switch_t Animation_Switches[8] = {
	switch_TieOff,
	switch_Outline,
	switch_Nightrider,
	switch_Whiskers,
	switch_Pinwheel,
	switch_Matrix,
	switch_BTAnimation,
	switch_Rainbow
};

// ****************************************************************************
// Animation functions
// ****************************************************************************
void initAnimation(uint8_t anim, uint8_t cfg) {
	blankLEDs();
	(Animation_Inits[anim])(cfg);
}

bool animAnimation(uint8_t anim, uint8_t step) {
	(Animation_Funcs[anim])(step);

	// Update the LEDs
	FastLED.show();
	
	// Delay for the FPS
	if (ms != 0)
		delay(ms);

	return (step == (mode_steps - 1));
}

bool switchAnimation(uint8_t anim, uint8_t step) {
	return (Animation_Switches[anim])(step);
}
