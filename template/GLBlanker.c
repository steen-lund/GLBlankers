#define VERSION     3
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "3.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "../BlankerStartup/Startup.inl"

void InitBlanker(struct BlankerData* bd)
{
	init_(bd);
}

void DeinitBlanker()
{
	release_();
}

void ReshapeBlanker(int width, int height)
{
	reshape_(width, height);
}

void DrawBlanker()
{
	draw_();
}
