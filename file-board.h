#ifndef __FILES__
void fileInit() {}
byte *doFile(CELL ir, byte *pc) { return pc; }
#else

#if __BOARD__ == TEENSY4
    #include "file-teensy.h"
#else
    #include "file-generic.h"
#endif // TEENSY4

byte *doFile(CELL ir, byte *pc) {
    ir = *(pc++);
    if (ir=='O') { doFopen(); }
    if (TOS==0) { printString("-nf-"); return pc; }
    else if (ir=='R') { doFread(); }
    else if (ir=='W') { doFwrite(); }
    else if (ir=='C') { doFclose(); }
    return pc;
}

#endif // __FILES__
