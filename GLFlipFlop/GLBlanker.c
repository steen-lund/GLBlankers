#define VERSION     3
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "3.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_flipflop(struct BlankerData* bd);
extern void reshape_flipflop(int w, int h);
extern void draw_flipflop();
extern void release_flipflop();

void InitBlanker(struct BlankerData* bd)
{
    init_flipflop(bd);
}

void DeinitBlanker()
{
    release_flipflop();
}

void ReshapeBlanker(int width, int height)
{
    reshape_flipflop(width, height);
}

void DrawBlanker()
{
    draw_flipflop();
}
