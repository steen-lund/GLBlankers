#define VERSION     2
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "2.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "../BlankerStartup/Startup.inl"

extern void init_flurry(struct BlankerData* bd);
extern void release_flurry();
extern void reshape_flurry();
extern void draw_flurry();

void InitBlanker(struct BlankerData* bd)
{
	init_flurry(bd);
}

void DeinitBlanker()
{
	release_flurry();
}

void ReshapeBlanker(int width, int height)
{
	reshape_flurry(width, height);
}

void DrawBlanker()
{
	draw_flurry();
}

void ResetSettingsToDefault(struct BlankerData* bd)
{
	uint32 id;
  
    id = IP96->p96BestModeIDTags(
		P96BIDTAG_NominalWidth,	    640,
		P96BIDTAG_NominalHeight,    480,
		P96BIDTAG_Depth,		    32,
	    P96BIDTAG_FormatsForbidden, RGBFB_CLUT,
	    TAG_END
    );

	bd->screenmodeID	= id;
	bd->style			= 0;
}
