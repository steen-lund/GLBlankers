#define VERSION     2
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "2.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_klein(struct BlankerData* bd);
extern void reshape_klein(int w, int h);
extern void draw_klein();

void InitBlanker(struct BlankerData* bd)
{
	init_klein(bd);
}

void DeinitBlanker()
{
	//release_klein();
}

void ReshapeBlanker(int width, int height)
{
	reshape_klein(width, height);
}

void DrawBlanker()
{
	draw_klein();
}
