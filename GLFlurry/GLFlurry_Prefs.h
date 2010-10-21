#ifndef __BLANKER_PREFS_H__
#define __BLANKER_PREFS_H__

#include <libraries/application.h>
#include "blanker.h"

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict);
void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict);

#endif
