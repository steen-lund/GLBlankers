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

	bd->screenmodeID	= id;
	bd->holdtime		= 0;
	bd->changetime		= 0;
	bd->dotexture		= TRUE;
	bd->dofog			= FALSE;
	bd->refetchSettings	= TRUE;
}
