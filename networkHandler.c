#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include "networkHandler.h"
#include "audioMixer_template.h"
#include "beat_mode.h"

int volume_change(char* snd_buffer, int sign) {
    int vol = AudioMixer_getVolume();
    vol += (5 * sign);
    AudioMixer_setVolume(vol);
    vol = AudioMixer_getVolume();
    return snprintf(snd_buffer, MAX_BUFF_SIZE, "vol:%d", vol);
}

int bpm_change(char* snd_buffer, int sign) {
    int bpm = AudioMixer_getBPM();
    bpm += (5 * sign);
    AudioMixer_setBPM(bpm);
    bpm = AudioMixer_getBPM();
    return snprintf(snd_buffer, MAX_BUFF_SIZE, "bpm:%d", bpm);
}

int mode_change(char* recv_buffer, char* snd_buffer, int msg_len) {
    int val = 0;
    // printf("recv buf = %s\n", recv_buffer);
    if (strncmp(recv_buffer, "mode:0", msg_len)==0 ) {      //Set to beat to None!
        AudioMixer_setMode(0);
    }
    else if (strncmp(recv_buffer, "mode:1", msg_len)==0 ) { //set to standard rock beat
        AudioMixer_setMode(1);
        val =1;
    }
    else {                                               // set to custom beat
        AudioMixer_setMode(2);
        val = 2;
    }
    return snprintf(snd_buffer, MAX_BUFF_SIZE, "mode:%d", val);        //Zero will be successes
}

int default_command(char* snd_buff) {
    int vol = AudioMixer_getVolume();
    int bpm = AudioMixer_getBPM();
    int mode = AudioMixer_getMode();
    return snprintf(snd_buff, MAX_BUFF_SIZE, "def=%d:%d:%d", vol, bpm, mode);
}

int set_drum(char* recv_buff, char* snd_buff, int msg_len) {
    if (strncmp(recv_buff, "hi_hat", msg_len) == 0) {
        beatBox_playHiHat();
    }
    else if (strncmp(recv_buff, "snare", msg_len) == 0) {
        beatBox_playSnare();
    }
    else {
        beatBox_playBase();
    }
    return snprintf(snd_buff, MAX_BUFF_SIZE, "DRUM");
}

int stop_command(char* snd_buff) {
    return -1 * snprintf(snd_buff, MAX_BUFF_SIZE, "Program Terminating\n");
}

int handle_packet(char recv_buffer[], char snd_buffer[], int msg_len) {
    int snd_len =0;
    // printf("Handling Packet!!\n");

    if (strncmp(recv_buffer, "default", msg_len) ==0) {
        snd_len = default_command(snd_buffer);
    }
    else if (strncmp(recv_buffer, "vol+", msg_len) ==0 ) {     //volume change msg
        // printf("Volume up command\n");
        snd_len = volume_change(snd_buffer, 1);
    }

    else if (strncmp(recv_buffer, "vol-", msg_len) ==0) { 
        snd_len = volume_change(snd_buffer, -1);
    }

    else if (strncmp(recv_buffer, "bpm+", msg_len) ==0) { //Mode change
        snd_len = bpm_change(snd_buffer, 1);
    }

    else if (strncmp(recv_buffer, "bpm-", msg_len) ==0) { 
        snd_len = bpm_change(snd_buffer, -1);
    }

    else if (strncmp(recv_buffer, "mode:",5) ==0) {
        snd_len = mode_change(recv_buffer, snd_buffer, msg_len);
    }
    else if (strncmp(recv_buffer, "stop\n", msg_len) ==0) {
        snd_len= stop_command(snd_buffer);
    }
    else {
        snd_len = set_drum(recv_buffer, snd_buffer, msg_len);
    }

    //TODO: Add the mode changes and the instrument selection
    
    return snd_len;
}


void* StartReceive(void* t) {
    char recv_buffer[MAX_BUFF_SIZE];
    char send_buffer[MAX_BUFF_SIZE];
    struct sockaddr_in target_addr;

    //create socket fd
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd==0) {
        perror("Socket fail to create");
        exit(-1);
    }
    memset(&target_addr, 0, sizeof(target_addr));

    target_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    target_addr.sin_port = htons(PORT);
    target_addr.sin_family = AF_INET;

    bind(sock_fd, (struct sockaddr*)&target_addr, sizeof(target_addr)); //Dont have to bind C app is a process

    socklen_t target_struct_len;
    int stop_flag = 0;
    char* send_msg = (char*) malloc(sizeof(char)*MAX_PACK_SIZE);
    while(stop_flag >-1) {                                  //have a flag set here to cancel this thread when done
        target_struct_len = sizeof(target_addr);
        // printf("waiting for packets on port %d....\n", PORT);
        int msg_len = recvfrom(sock_fd, (char *)recv_buffer, MAX_BUFF_SIZE, 0, 
                (struct sockaddr*)&target_addr, &target_struct_len);
        //printf("recv buffer = %s\n", recv_buffer);
        int send_len = handle_packet(recv_buffer,send_buffer, msg_len);
        if (send_len <0) {
            stop_flag = send_len;
            send_len*=-1;
        }
        int end = MAX_PACK_SIZE-1;
        int start = 0;


        //TODO: line 171 valgrind errors, mostl likely math wron
        while (send_len > MAX_PACK_SIZE) { //only fthe get array command
            while (send_buffer[end]!=',' && end!=start) {       //end of number
                end--;
            }
            end+=1;
            strncpy(send_msg, send_buffer + start, end - start);
            sendto(sock_fd, send_msg, end - start, 0, (struct sockaddr *)&target_addr, target_struct_len);
            memset(send_msg, 0, MAX_PACK_SIZE);
            send_len = send_len - (end -start);
            start = end;
            end = start + MAX_PACK_SIZE-1;
            //printf("Next packet\n!");
        }
        //  printf("Sending the last packet\n");

        strncpy(send_msg, send_buffer + start, send_len);

        sendto(sock_fd, send_msg, send_len, 0, (struct sockaddr *)&target_addr, target_struct_len);
        sendto(sock_fd, "\n", 1, 0, (struct sockaddr *)&target_addr, target_struct_len);
        memset(recv_buffer, 0, MAX_BUFF_SIZE);
        memset(send_buffer, 0, MAX_BUFF_SIZE);
        memset(send_msg, 0, MAX_PACK_SIZE);
    }
    free(send_msg);
    send_msg = NULL;
    close(sock_fd);
    // printf("networkprogram ends\n");
    pthread_exit(NULL);
}