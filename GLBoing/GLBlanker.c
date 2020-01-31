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

void ResetSettingsToDefault(struct BlankerData *bd)
{
	uint32 id;

    id = IP96->p96BestModeIDTags(
		P96BIDTAG_NominalWidth,	    640,
		P96BIDTAG_NominalHeight,    480,
	    P96BIDTAG_Depth,		    16,
	    P96BIDTAG_FormatsForbidden, RGBFB_CLUT,
	    TAG_END
    );

	bd->screenmodeID = id;
}
