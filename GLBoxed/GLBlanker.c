#define VERSION     2
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "2.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_boxed(struct BlankerData* bd);
extern void reshape_boxed(int w, int h);
extern void draw_boxed();
extern void release_boxed();

void InitBlanker(struct BlankerData* bd)
{
    init_boxed(bd);
}

void DeinitBlanker()
{
    release_boxed();
}

void ReshapeBlanker(int width, int height)
{
    reshape_boxed(width, height);
}

void DrawBlanker()
{
    draw_boxed();
}
