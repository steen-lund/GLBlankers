#define VERSION     2
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "2.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_blocktube(struct BlankerData* bd);
extern void release_blocktube();
extern void reshape_blocktube(int width, int height);
extern void draw_blocktube();

void InitBlanker(struct BlankerData* bd)
{
    init_blocktube(bd);
}

void DeinitBlanker()
{
    release_blocktube();
}

void ReshapeBlanker(int width, int height)
{
    reshape_blocktube(width, height);
}

void DrawBlanker()
{
    draw_blocktube();
}
