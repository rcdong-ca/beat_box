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

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

//I2c Device Info
#define I2C_DEVICE_ADDRESS 0x1C
#define REG_WHO_AM_I 0x0D

//Turn off, on GPIO pins!
// void set_GPIO_pin(int pin_num, int val) {
//     FILE* fd;
//     if (pin_num == 61)
//         fd = fopen(GPIO_61, "w");
//     else if  (pin_num==44)
//         fd = fopen(GPIO_44, "w");
//     else {
//         printf("Incorrect GPIO num: %d\n", pin_num);
//         return;
//     }
//     fprintf(fd,"%d", val);
//     fclose(fd);
//     return ;
// }

static int initI2cBus(char* bus, int address){
    int i2cFileDesc = open(bus, O_RDWR);
    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0) {
        perror("I2C: Unable to set I2C device to slave address.");
        exit(1);
    }
    return i2cFileDesc;
}

static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr) {
    // To read a register, must first write the address
    int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
    if (res != sizeof(regAddr)) {
        perror("I2C: Unable to write to i2c register.");
        exit(1);
    }// Now read the value and return it
    char* value = (char*)malloc(sizeof(char)*7);
   
    res = read(i2cFileDesc, value, 7);
    if (res != sizeof(char)*7) {
        perror("I2C: Unable to read from i2c register");
        exit(1);
    }
    for (int i =0; i<7;i++) {
        printf("val = %02x  ", value[i]);
    }
    printf("\n");
    return *value;
}


void myI2cget(int i2cFileDesc, unsigned char regAddr) {
   char buffer[1];
   int fd = open(I2CDRV_LINUX_BUS1, O_RDWR);
   ioctl(i2cFileDesc, I2C_SLAVE, 0x1C);
   write(i2cFileDesc, "\r", 1);
   read(i2cFileDesc, "\0", 1);
   close(fd);

}

static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value){
    unsigned char buff[2];
    buff[0] = regAddr;
    buff[1] = value;
    int res = write(i2cFileDesc, buff, 2);
    if (res != 2) {
        perror("I2C: Unable to write i2c register."); 
        exit(1);
        }
}

int main() {
    //first attempt to read the who am i register
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

    writeI2cReg(i2cFileDesc, 0x2A, 0x01);
    printf("Setting to Active Mode\n");
    printf("Reading the pin\n");
    //unsigned char regVal = readI2cReg(i2cFileDesc, 0x29);
    // myI2cget(i2cFileDesc, 0x2A);
    //printf("Reg OUT-A = 0x%02x\n", regVal);
    return 0;
}
