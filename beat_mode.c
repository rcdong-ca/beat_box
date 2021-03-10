#include "beat_mode.h"
#include "audioMixer_template.h"
#include <time.h>
#include <pthread.h>

//NOTE: 0 --> Base, 1 --> Snare, 2-->Hi-Hat
#define BASE "beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav"
#define HI_HAT "beatbox-wav-files/100053__menegass__gui-drum-cc.wav"
#define SNARE "beatbox-wav-files/100059__menegass__gui-drum-snare-soft.wav"
static wavedata_t base;
static wavedata_t hi_hat;
static wavedata_t snare;

void beatBoxInit(void){
    AudioMixer_readWaveFileIntoMemory(BASE, &base);
    AudioMixer_readWaveFileIntoMemory(HI_HAT, &hi_hat);
    AudioMixer_readWaveFileIntoMemory(SNARE, &snare);
}

void beatBoxCleanup(void){
    AudioMixer_freeWaveFileData(&base);
    AudioMixer_freeWaveFileData(&hi_hat);
    AudioMixer_freeWaveFileData(&snare);
}

void* beatBoxThread(void* t){
    beatBoxInit();
    while(1){
        if(AudioMixer_getMode() ==0){
            AudioMixer_noBeat();
        }
        if(AudioMixer_getMode() == 1){
            AudioMixer_standardRockBeat(&base, &hi_hat, &snare);
        }
        if(AudioMixer_getMode() == 2){
            AudioMixer_customBeat(&base, &hi_hat, &snare);
        }
    }
    beatBoxCleanup();
    pthread_exit(NULL);
}

void beatBoxSingle(wavedata_t *psound){
    AudioMixer_queueSound(psound);
}

void beatBox_playBase(void){
    beatBoxSingle(&base);
}

void beatBox_playHiHat(void){
    beatBoxSingle(&hi_hat);
}

void beatBox_playSnare(void){
    beatBoxSingle(&snare);
}