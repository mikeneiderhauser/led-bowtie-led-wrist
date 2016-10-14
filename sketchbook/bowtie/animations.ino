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
	mode_steps = PX_EDGE_SIZE;
	ms = FPS(50);
	// Set the default palette	
}

void anim_Outline(uint8_t step) {
	uint8_t last_step = 0;
	uint8_t px;

	// This is the always on mode
	if (mode_cfg == 255) {
  		for(uint8_t i = 0; i<PX_EDGE_SIZE; i++) {
			px = PG(PG_EDGE, i);
			setPixel(px, palette_step);
		}
    palette_step++;
		return true;
	} 

	// Get the previous step
	if (step == 0)
		last_step = mode_steps;
	else
		last_step = step - 1;

	// Turn off last LEDs
	px = PG(1, last_step);
	leds[px] = CRGB::Black;

	// Turn off all of the possibly used LEDs, keep it simple
	if (mode_cfg == 4) {
		px = PG(PG_EDGE, last_step + 33);
		leds[px] = CRGB::Black;

		px = PG(PG_EDGE, last_step - 11);
		leds[px] = CRGB::Black;

		px = PG(PG_EDGE, last_step - 33);
		leds[px] = CRGB::Black;

		px = PG(PG_EDGE, last_step + 11);
		leds[px] = CRGB::Black;
	}

	if ((mode_cfg == 2) || (mode_cfg == 4)) {
		px = PG(PG_EDGE, last_step + 22);
		leds[px] = CRGB::Black;

		px = PG(PG_EDGE, last_step - 22);
		leds[px] = CRGB::Black;
	}

	// Turn on current LEDs
	px = PG(PG_EDGE, step);
	setPixel(px, palette_step);

	// Turn on the 4x LEDs
	if (mode_cfg == 4) {
    if (step < 11) {
			px = PG(PG_EDGE, step + 33);
			setPixel(px, palette_step);
		} else {
			px = PG(PG_EDGE, step - 11);
			setPixel(px, palette_step);
		}

    if (step < 33) {
			px = PG(PG_EDGE, step + 11);
			setPixel(px, palette_step);
		} else {
			px = PG(PG_EDGE, step - 33);
			setPixel(px, palette_step);
		}
	}
	
	// Turn on the 2x LEDs
	if ((mode_cfg == 2) || (mode_cfg == 4)) {
    if (step < 22) {
			px = PG(PG_EDGE, step + 22);
			setPixel(px, palette_step);
		} else {
			px = PG(PG_EDGE, step - 22);
			setPixel(px, palette_step);
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
    if (step == 0)
      last = 14;
    else
      last = step - 1;
		// Clear the previous pixels
		leds[idxs[last]] = CRGB::Black;
		leds[idxs[last] - 1] = CRGB::Black;
		leds[idxs[last] + 1] = CRGB::Black;

		// Right to left
		setPixel(idxs[step], palette_step);
		setPixel(idxs[step] - 1, palette_step);
		setPixel(idxs[step] + 1, palette_step);
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
		if (step == 40)
      idx = 15;
    else
      idx = 29 - (last - 40);
   
		// Clear the previous pixels
		leds[idxs[idx]] = CRGB::Black;
		leds[idxs[idx] - 1] = CRGB::Black;
		leds[idxs[idx] + 1] = CRGB::Black;

		// Left to Right
		idx = 29 - (step - 40);
		setPixel(idxs[idx], palette_step);
		setPixel(idxs[idx] - 1, palette_step);
		setPixel(idxs[idx] + 1, palette_step);
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

}

void anim_Pinwheel(uint8_t step) {

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
void init_Matrix(int cfg) {

}

void anim_Matrix(uint8_t step) {

}

void switch_Matrix(uint8_t step) {
	return true;
}

uint8_t BT_Animations[1][1] = {
  { 0 },
};

// **********************************************
// * Play an Animation from BowtieEd
// **********************************************
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
Animation_Init_t Animation_Inits[7] = {
	init_TieOff,
	init_Outline,
	init_Nightrider,
	init_Whiskers,
	init_Pinwheel,
	init_Matrix,
	init_BTAnimation,
//	init_Rainbow
};

typedef void (*Animation_Func_t)(uint8_t);
Animation_Func_t Animation_Funcs[7] = {
	anim_TieOff,
	anim_Outline,
	anim_Nightrider,
	anim_Whiskers,
	anim_Pinwheel,
	anim_Matrix,
	anim_BTAnimation,
//	anim_Rainbow
};

typedef bool (*Animation_Switch_t)(uint8_t);
Animation_Switch_t Animation_Switches[7] = {
	switch_TieOff,
	switch_Outline,
	switch_Nightrider,
	switch_Whiskers,
	switch_Pinwheel,
	switch_Matrix,
	switch_BTAnimation,
//	switch_Rainbow
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
