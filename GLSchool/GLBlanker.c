#define VERSION     2
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "2.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_glschool(struct BlankerData* bd);
extern void reshape_glschool(int w, int h);
extern void draw_glschool();
extern void release_glschool();

void InitBlanker(struct BlankerData* bd)
{
	init_glschool(bd);
}

void DeinitBlanker()
{
	release_glschool();
}

void ReshapeBlanker(int width, int height)
{
	reshape_glschool(width, height);
}

void DrawBlanker()
{
	draw_glschool();
}
