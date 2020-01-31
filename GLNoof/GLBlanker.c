#define VERSION     2
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "2.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_noof(struct BlankerData* data);
extern void reshape_noof(int w, int h);
extern void draw_noof();

void InitBlanker(struct BlankerData* bd)
{
	init_noof(bd);
}

void DeinitBlanker()
{
	//release_();
}

void ReshapeBlanker(int width, int height)
{
	reshape_noof(width, height);
}

void DrawBlanker()
{
	draw_noof();
}
