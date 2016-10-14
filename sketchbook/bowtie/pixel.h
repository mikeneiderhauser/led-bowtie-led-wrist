#ifndef __PIXEL_H__
#define __PIXEL_H__

#define NUM_COL  	15
#define NUM_ROW 	9
#define PIXEL_CT	93

#define PX_ONE_NEIGHBOR_SIZE	37	
#define PX_EDGE_SIZE	44

uint8_t PG(uint8_t pg, uint8_t pixel);
void P2C(uint8_t p, uint8_t *r, uint8_t *c);
uint8_t C2P(uint8_t r, uint8_t c);
void UnpackFrame(uint8_t frame, uint8_t *animation_data, bool undo);
void LoadFrame(uint8_t frame_idx, uint8_t *animation_data);

#define PALETTE_RAINBOW 0
#define PALETTE_PARTY	1
#define PALETTE_HEAT	2
#define PALETTE_PURPLE	3
#define PALETTE_RED 	4

#endif
