#ifndef __PIXEL_H__
#define __PIXEL_H__

#define NUM_COL  	15
#define NUM_ROW 	9
#define PIXEL_CT	93

#define PG_ONE_NEIGHBOR_SIZE	37	
#define PG_EDGE_SIZE	44
#define PG_TOP_ROW_SIZE	15

#define PG_ONE_NEIGHBOR 	0
#define PG_EDGE    	 	1
#define PG_TOP_ROW		15

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
