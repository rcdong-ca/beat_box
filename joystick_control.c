#include "joystick_control.h"
#include "audioMixer_template.h"
#include <time.h>
#include <pthread.h>


#define SIZE_MAX 512

static int endJoystickThread=1;

FILE* fileOpen(char* filename, char* type){
    FILE *fd = fopen(filename, type);
    if(fd == NULL){
        printf("Unable to open file %s\n", filename);
        exit(-1);
    }
    return fd;
}

void fileClose(FILE* fd){
    fclose(fd);
}

void exportPin(int btn){ //taken from guide
    FILE *pFile = fopen(EXPRT, "w");
    if (pFile == NULL) {
        printf("ERROR: Unable to open export file.\n");
        exit(1);
    }
    // Write to data to the file using fprintf():
    fprintf(pFile, "%d", btn);
    // Close the file using fclose():
    fclose(pFile);

    
}

int readPin(char* filename){
    FILE *fd = fileOpen(filename, "r");
    char buffer[SIZE_MAX];
    fgets(buffer, SIZE_MAX, fd);
    fileClose(fd);
    return atoi(buffer);
}

int pressed(char* file){
    if(readPin(file) == 0){
        return 1;
    }else{
        return 0;
    }
}

void stopJoystickThread(void){
    endJoystickThread = 0;
}

void* joystickthread(void* t){
    //export all pins
    //sleep for 300ms
    struct timespec time1;
    time1.tv_sec = 0;
    time1.tv_nsec = 330000000;
    //printf("Joystick thread...\n");
    endJoystickThread = 1;

    exportPin(JSPUSH);
    nanosleep(&time1, (struct timespec*)NULL); // sleep for 330 millisecond
    exportPin(JSUP);
    nanosleep(&time1, (struct timespec*)NULL); // sleep for 330 millisecond
    exportPin(JSRIGHT);
    nanosleep(&time1, (struct timespec*)NULL); // sleep for 330 millisecond
    exportPin(JSDOWN);
    nanosleep(&time1, (struct timespec*)NULL); // sleep for 330 millisecond
    exportPin(JSLEFT);
    nanosleep(&time1, (struct timespec*)NULL); // sleep for 330 millisecond

    while(endJoystickThread == 1){
        struct timespec time1;
        time1.tv_sec = 0;
        time1.tv_nsec = 150000000; //debounce for 150ms

        if(pressed(JSPSH_FILE) == 1){
            AudioMixer_next();
            printf("New Mode: %d\n", AudioMixer_getMode());
            nanosleep(&time1, (struct timespec*)NULL);
        }
        if(pressed(JSUP_FILE) == 1){
            AudioMixer_setVolume(AudioMixer_getVolume() + 5);
            printf("Volume increased to: %d\n", AudioMixer_getVolume());
            nanosleep(&time1, (struct timespec*)NULL);
        }
        if(pressed(JSDN_FILE) == 1){
            AudioMixer_setVolume(AudioMixer_getVolume() - 5);
            printf("Volume decreased to: %d\n", AudioMixer_getVolume());
            nanosleep(&time1, (struct timespec*)NULL);
        }
        if(pressed(JSRT_FILE) == 1){
            AudioMixer_setBPM(AudioMixer_getBPM() + 5);
            printf("BPM increased to: %lf\n", AudioMixer_getBPM());
            nanosleep(&time1, (struct timespec*)NULL);
        }
        if(pressed(JSLFT_FILE) == 1){
            AudioMixer_setBPM(AudioMixer_getBPM() - 5);
            printf("BPM decreased to: %lf\n", AudioMixer_getBPM());
            nanosleep(&time1, (struct timespec*)NULL);
        }
    }
    printf("Ending Joystick Thread...\n");
    pthread_exit(NULL);
}

