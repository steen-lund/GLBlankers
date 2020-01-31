#define VERSION     2
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "2.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_hypertorus(struct BlankerData* bd);
extern void reshape_hypertorus(int width, int height);
//extern void change_hypertorus(struct BlankerData* bd);
extern void draw_hypertorus();

void InitBlanker(struct BlankerData* bd)
{
	init_hypertorus(bd);
}

void DeinitBlanker()
{
}

void ReshapeBlanker(int width, int height)
{
	reshape_hypertorus(width, height);
}

void DrawBlanker()
{
	draw_hypertorus();
}