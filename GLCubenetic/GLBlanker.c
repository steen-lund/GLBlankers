#define VERSION     2
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "2.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_cube(struct BlankerData* bd);
extern void reshape_cube(int w, int h);
extern void release_cube();
extern void draw_cube();

void InitBlanker(struct BlankerData* bd)
{
	init_cube(bd);
}

void DeinitBlanker()
{
	release_cube();
}

void ReshapeBlanker(int width, int height)
{
	reshape_cube(width, height);
}

void DrawBlanker()
{
	draw_cube();
}
