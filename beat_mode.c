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

void* beatBoxThread(void* t){
    AudioMixer_readWaveFileIntoMemory(BASE, &base);
    AudioMixer_readWaveFileIntoMemory(HI_HAT, &hi_hat);
    AudioMixer_readWaveFileIntoMemory(SNARE, &snare);
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
    pthread_exit(NULL);
}
