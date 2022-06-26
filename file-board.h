#ifndef __FILES__
void fileInit() {}
#else

#if __BOARD__ == TEENSY4
    #include "file-teensy.h"
#else
    #include "file-generic.h"
#endif // TEENSY4


#endif // __FILES__
