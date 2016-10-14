#ifndef __ANIMATIONS_T__
#define __ANIMATIONS_T__

void initAnimation(uint8_t anim, uint8_t cfg);
void animAnimation(uint8_t anim, uint8_t step);
void switchAnimation(uint8_t anim, uint8_t step);

#define ANIM_TIE_OFF		   0  // mode 0, cfg x
#define ANIM_OUTLINE2		   1  // mode 1, cfg 2
#define ANIM_OUTLINE4		   2  // mode 1, cfg 4
#define ANIM_OUTLINE_ON	   3  // mode 1, cfg 255
#define ANIM_OUTLINE_ON_RB 4  // mode 1, cfg 255
#define ANIM_WHISKERS      5  // mode 3
#define ANIM_PINWHEEL		   6  // mode 4
#define ANIM_NIGHTRIDER	   7  // mode 2
#define ANIM_NIGHTRIDER_RB 8  // mode 2
#define ANIM_MATRIX		     9  // mode 5
#define ANIM_BTANIMATION   10 // mode 6
#define ANIM_RAINBOW		   11 // mode 7
#define ANIM_COUNT         12 // MAX ANIM COUNT

#endif
