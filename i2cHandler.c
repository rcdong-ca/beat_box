// Assume pins already configured for I2C: 
//   (bbg)$ config-pin P9_18 i2c
//   (bbg)$ config-pin P9_17 i2c

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include <time.h>
#include <pthread.h>
#include "beat_mode.h"

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

//I2c Device Info
#define I2C_DEVICE_ADDRESS 0x1C
#define REG_WHO_AM_I 0x0D

#define X_Y_THRESH 12000
#define Z_THRESH 13000
#define Z_G_FORCE 16400

static int i2cHandlerEnd = 1;

int initI2cBus(char* bus, int address){
    int i2cFileDesc = open(bus, O_RDWR);
    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0) {
        perror("I2C: Unable to set I2C device to slave address.");
        exit(1);
    }
    return i2cFileDesc;
}

unsigned char* readI2cReg(int i2cFileDesc, unsigned char regAddr) {
    // To read a register, must first write the address
    int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
    if (res != sizeof(regAddr)) {
        perror("I2C: Unable to write to i2c register.");
        exit(1);
    }// Now read the value and return it
    unsigned char* value = (unsigned char*)malloc(sizeof(char)*7);
   
    res = read(i2cFileDesc, value, 7);
    if (res != sizeof(char)*7) {
        perror("I2C: Unable to read from i2c register");
        exit(1);
    }
    return value;
}


void myI2cget(int i2cFileDesc, unsigned char regAddr) {
//    char buffer[1];
   int fd = open(I2CDRV_LINUX_BUS1, O_RDWR);
   ioctl(i2cFileDesc, I2C_SLAVE, 0x1C);
   write(i2cFileDesc, "\r", 1);
   read(i2cFileDesc, "\0", 1);
   close(fd);

}

void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value){
    unsigned char buff[2];
    buff[0] = regAddr;
    buff[1] = value;
    int res = write(i2cFileDesc, buff, 2);
    if (res != 2) {
        perror("I2C: Unable to write i2c register."); 
        exit(1);
    }
}

void endI2cHandler(void){
    i2cHandlerEnd = 0;
}

void* i2cHandlerInit(void* t){
    //first attempt to read the who am i register
    beatBoxInit();
    struct timespec time1;
    time1.tv_sec = 0;
    time1.tv_nsec = 300000000;
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

    writeI2cReg(i2cFileDesc, 0x2A, 0x01);
    printf("Setting to Active Mode\n");
    printf("Reading the pin\n");

    
    while(i2cHandlerEnd > 0){
         unsigned char* regVal = readI2cReg(i2cFileDesc, 0x00);

        __int16_t x = (regVal[1] << 8 | regVal[2]);
        __int16_t y = (regVal[3] << 8 | regVal[4]);
        __int16_t z = (regVal[5] << 8 | regVal[6]) - Z_G_FORCE;

        if(x < -X_Y_THRESH || x > X_Y_THRESH){
            beatBox_playBase();
            nanosleep(&time1, (struct timespec*)NULL);
            continue;
        }

        if(y < -X_Y_THRESH || y > X_Y_THRESH){
            beatBox_playHiHat();
            nanosleep(&time1, (struct timespec*)NULL);
            continue;
        }

        if(z < -Z_THRESH || z > Z_THRESH){
            beatBox_playSnare();
            nanosleep(&time1, (struct timespec*)NULL);
            continue;
        }

        // printf("x: %d, y: %d, z: %d\n", x, y, z);
        free(regVal);
        regVal = NULL;
    }
    beatBoxCleanup();
    pthread_exit(NULL);
    return 0;
}
