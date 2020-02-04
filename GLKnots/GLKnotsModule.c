#define VERSION     2
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "2.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

void init_knot(struct BlankerData* bd);
void reshape_knot(int width, int height);
void draw_knot();

void InitBlanker(struct BlankerData* bd)
{
    init_knot(bd);
}

void DeinitBlanker()
{}

void ReshapeBlanker(int width, int height)
{
    reshape_knot(width, height);
}

void DrawBlanker()
{
    draw_knot();
}
