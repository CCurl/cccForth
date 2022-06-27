#ifndef __FILES__

void noFile() { printString("-noFile-"); }
void fOpen() { noFile(); }
void fRead() { noFile(); }
void fWrite() { noFile(); }
void fClose() { noFile(); }

#else

#if __BOARD__ == TEENSY4
    #include "file-teensy.h"
#else
    #include "file-generic.h"
#endif // TEENSY4

#endif // __FILES__
