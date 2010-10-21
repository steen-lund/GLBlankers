#ifndef _GL_FLURRY_GUI__
#define _GL_FLURRY_GUI__

#include <libraries/blankermodule.h>
#include "blanker.h"

uint32 OpenGUILibraries();
void CloseGUILibraries();
BOOL MakeGUI(struct BlankerData *bd, struct BlankerPrefsWindowSetup *bpws);
void DestroyGUI();
void GUIEventFunc(struct Hook *hook, struct BlankerModuleIFace *Self, struct BlankerPrefsWinGUIEvent *event);
void GUIIdcmpFunc(struct Hook *hook, struct BlankerModuleIFace *Self, struct BlankerPrefsWinIDCMPEvent *event);

#endif
