#ifndef _I2CHANDLER_H
#define _I2CHANDLER_H
#include "beat_mode.h"


static int initI2cBus(char* bus, int address);
static unsigned char* readI2cReg(int i2cFileDesc, unsigned char regAddr) ;
void myI2cget(int i2cFileDesc, unsigned char regAddr);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);
void endI2cHandler(void);
void* i2cHandlerInit(void*);


#endif