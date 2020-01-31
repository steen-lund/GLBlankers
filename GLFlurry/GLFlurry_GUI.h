#pragma once

#include "blanker.h"
#include <interfaces/blankermodule.h>

void GUIEventFunc(struct Hook *hook, struct BlankerModuleIFace *Self, struct BlankerPrefsWinGUIEvent *event);
void GUIIdcmpFunc(struct Hook *hook, struct BlankerModuleIFace *Self, struct BlankerPrefsWinIDCMPEvent *event);
