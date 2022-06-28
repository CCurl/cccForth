#ifndef __FILES__

void noFile() { printString("-noFile-"); }
void fileInit() { }
void fOpen() { noFile(); }
void fRead() { noFile(); }
void fWrite() { noFile(); }
void fClose() { noFile(); }

#else
#include "LittleFS.h"

#define NF 10
#define VALIDF(x) BTW(x,1,NF) && (files[x])

static File files[NF+1];
static int isInit = 0;

#if __BOARD__ == TEENSY4
    #include "file-teensy.h"
#else
    #include "file-generic.h"
#endif // TEENSY4

#endif // __FILES__
