#ifndef _BEAT_MODE_H_
#define _BEAT_MODE_H_
#include "audioMixer_template.h"


void beatBoxInit(void);
void* beatBoxThread(void*);
void beatBoxSingle(wavedata_t *psound);
void beatBox_playBase(void);
void beatBox_playHiHat(void);
void beatBox_playSnare(void);
void beatBoxCleanup(void);

#endif