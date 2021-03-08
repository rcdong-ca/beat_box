#ifndef _JOYSTICK_CONTROL_H_
#define _JOYSTICK_CONTROL_H_

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX 512
#define JSUP 26
#define JSDOWN 46 
#define JSLEFT 65
#define JSRIGHT 47
#define JSPUSH 27
#define EXPRT "/sys/class/gpio/export"
#define JSUP_FILE "/sys/class/gpio/gpio26/value"
#define JSDN_FILE "/sys/class/gpio/gpio46/value"
#define JSLFT_FILE "/sys/class/gpio/gpio65/value"
#define JSRT_FILE "/sys/class/gpio/gpio47/value"
#define JSPSH_FILE "/sys/class/gpio/gpio27/value"


FILE* fileOpen(char* filename, char* type);
void fileClose(FILE* fd);

void* joystickthread(void*);
void stopJoystickThread(void);

#endif