#ifndef _GL_BLOCKTUBE_H__
#define _GL_BLOCKTUBE_H__

#include "blanker.h"

extern void init_blocktube(struct BlankerData* bd);
extern void reshape_blocktube(int width, int height);
extern void release_blocktube(void);
extern void draw_blocktube();

#endif
