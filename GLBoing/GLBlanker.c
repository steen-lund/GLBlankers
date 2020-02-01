#define VERSION     3
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "3.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_boing(struct BlankerData* bd);
extern void reshape_boing(int w, int h);
extern void draw_boing();
//extern void release_boing();

void InitBlanker(struct BlankerData* bd)
{
    init_boing(bd);
}

void DeinitBlanker()
{
    //release_boing();
}

void ReshapeBlanker(int width, int height)
{
    reshape_boing(width, height);
}

void DrawBlanker()
{
    draw_boing();
}
