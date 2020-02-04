#define VERSION     1
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "1.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_crackberg();
extern void reshape_crackberg(int width, int height);
extern void draw_crackberg();

void InitBlanker(struct BlankerData* bd)
{
    init_crackberg(bd);
}

void DeinitBlanker()
{
    //release_crackberg();
}

void ReshapeBlanker(int width, int height)
{
    reshape_crackberg(width, height);
}

void DrawBlanker()
{
    draw_crackberg();
}
