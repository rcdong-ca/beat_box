#include <stdio.h>
#include <unistd.h>
#include "audioMixer_template.h"


#define BASE "beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav"
#define HI_HAT "beatbox-wav-files/100053__menegass__gui-drum-cc.wav"
#define SNARE "beatbox-wav-files/100059__menegass__gui-drum-snare-soft.wav"
static wavedata_t base;
static wavedata_t hi_hat;
static wavedata_t snare;

int main() {

    AudioMixer_init();
    int count = 0;
    AudioMixer_readWaveFileIntoMemory(BASE, &base);
    AudioMixer_readWaveFileIntoMemory(HI_HAT, &hi_hat);
    AudioMixer_readWaveFileIntoMemory(SNARE, &snare);
    while (count <2) {
        AudioMixer_standardRockBeat(&base, &hi_hat, &snare);
        
        // AudioMixer_queueSound(&hi_hat);
        // AudioMixer_queueSound(&snare);
        sleep(1);
        count++;
    }
    AudioMixer_cleanup();

    
    return 0;
}

void* Joystick_control(void) {
    int beat_pos = 0;

    while(1) {              //add stop conidition that triggers this when audioMixer_cleanup gets called
        int decision = read_Joystick(); //more like it will stay in some while loop until it gets a change. REFACTOR: not efficient
        
        if (decision<1) {   //UP
            int vol = AudioMixer_getVolume();
            vol+=5;
            if (vol > 100)
                vol = 100;
            AudioMixer_setVolume(vol);
        }
        else if (decision <2) { //left decrease the BPM
            double BPM = AudioMixer_getBPM();
            BPM-=5.0;
            AudioMixer_setBPM(BPM);
        }

        else if (decision < 3) {    //right
            double BPM = AudioMixer_getBPM();
            BPM+=5.0;
            AudioMixer_setBPM(BPM);
        }
        else if (decision <4)  {       //down
            int vol = AudioMixer_getVolume();
            vol-=5;
            if (vol < 0)
                vol = 0;
            AudioMixer_setVolume(vol);
        }
        else {
            beat_pos = (beat_pos + 1) % 2; 
            if (beat_pos == 0) {
                //DO not play anything
            }
            else if (beat_pos==1) {
                // play the rock beat
                AudioMixer_standardRockBeat(&base, &hi_hat, &snare);
            }
        }       
        
        
    }

}