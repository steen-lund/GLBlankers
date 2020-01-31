/*
 * This example is best viewed with a TabSize of 4 and using GoldEd's Folding methods.
 * Feel free to use/misuse this example as you se fit.
 */

#define VERSION     2
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "2.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#include "blanker.h"
#include "yarandom.h"
#include "glmatrix.h"
#include "GLMatrixModule_GUI.h"

#include "../BlankerStartup/Startup.inl"

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


/// ResetSettingsToDefault
void ResetSettingsToDefault( struct BlankerData *bd )
{
   uint32 id;

	id = IP96->p96BestModeIDTags(
	   P96BIDTAG_NominalWidth,     640,
	   P96BIDTAG_NominalHeight,    480,
	   P96BIDTAG_Depth,		       16,
	   P96BIDTAG_FormatsForbidden, RGBFB_CLUT,
	   TAG_END
	);

   bd->density		   = 20;
   bd->speed		   = 100;
   bd->encoding 	   = 0;
   bd->screenmodeID	   = id;
   bd->fog			   = TRUE;
   bd->wave			   = TRUE;
   bd->rotate		   = TRUE;
   bd->showTime	   = FALSE;
   bd->refetchSettings = TRUE;
}
///

/* -- The End -- */

