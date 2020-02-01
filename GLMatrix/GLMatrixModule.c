#define VERSION     2
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "2.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_matrix(struct BlankerData* bd);
extern void free_matrix();
extern void reshape_matrix(int width, int height);
extern void draw_matrix();

void InitBlanker(struct BlankerData* bd)
{
    init_matrix(bd);
}

void DeinitBlanker()
{
    free_matrix();
}

void ReshapeBlanker(int width, int height)
{
    reshape_matrix(width, height);
}

void DrawBlanker()
{
    draw_matrix();
}
