#ifndef _NetHandler_H_
#define _NetHandler_H_

#define PORT 3000               //Port listening on the beagle bone
#define MAX_BUFF_SIZE 1024
#define MAX_PACK_SIZE 1024

void* StartReceive(void* t);

#endif