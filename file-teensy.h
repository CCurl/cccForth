// file-teensy.h
// File ops on the Teensy are different

LittleFS_Program myFS;

static File files[10];
static int fileSp=0, isInit=0;

void fileInit() {
    myFS.begin(1 * 1024 * 1024);
    printString("\r\nLittleFS: initialized");
    printStringF("\r\nBytes Used: %llu, Bytes Total:%llu", myFS.usedSize(), myFS.totalSize());
    for (int i = 0; i < 10; i++) { files[i] = 0; }
    fileSp = 0;
    isInit = 1;
}

void fOpen() {
    if (!isInit) { fileInit();  }
    CELL md = pop();
    char* fn = (char*)pop();
    int i = (fileSp<9) ? fileSp+1 : 0;
    if (i) {
        files[i] = myFS.open(fn, (md) ? FILE_WRITE : FILE_READ);
        if (files[i]) {
            fileSp = i;
        } else {
            i = 0;
            isError = 1;
            printString("-openFail-");
        }
    }
    push(i);
}

void fRead() {
    // (fh--c n)
    byte c;
    CELL fh = TOS; 
    push(0);
    if (BTW(fh, 1, 9)) {
        TOS = files[fh].read(&c, 1);
        NOS = (CELL)c;
    }
}
void fWrite() {
    // (c fh--)
    CELL fh = pop();
    CELL c = pop();
}

void fClose() {
    // (fh--)
    CELL fh = pop();
    if (BTW(fh,1,9)) {
        files[fh].close();
        files[fh] = 0;
    }
    while ((0<fileSp) && (files[fileSp]==0)) { --fileSp; }
}
