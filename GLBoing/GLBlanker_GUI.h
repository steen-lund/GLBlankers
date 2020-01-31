#ifndef _GL_BLANKER_GUI__
#define _GL_BLANKER_GUI__

#include <libraries/blankermodule.h>
#include "blanker.h"

void GUIEventFunc(struct Hook* hook, struct BlankerModuleIFace* Self, struct BlankerPrefsWinGUIEvent* event);
void GUIIdcmpFunc(struct Hook* hook, struct BlankerModuleIFace* Self, struct BlankerPrefsWinIDCMPEvent* event);
void UpdateWindowSettings(struct BlankerData* bd);

#endif
