// File support for generic boards

// file-teensy.h
// File ops on the Teensy are different

LittleFS myFS;

void fileInit() {
    myFS.begin();
    FSInfo fs_info;
    myFS.info(fs_info);
    printString("\r\nLittleFS: initialized");
    printStringF("\r\nTotal Size: %ld bytes, Used: %ld", fs_info.totalBytes, fs_info.usedBytes);
    for (int i = 0; i <= NF; i++) { files[i] = 0; }
    isInit = 1;
}

int openSlot() {
    if (!isInit) { fileInit(); }
    for (int i = 1; i <= NF; i++)
        if (files[i] == 0) { return i; }
    return 0;
}

void fOpen() {               // (name mode--fh)
    CELL mode = pop();
    char* fn = (char*)TOS;
    TOS = 0;
    int i = openSlot();
    if (i) {
        files[i] = myFS.open(fn, (mode) ? "w" : "r");
        if (files[i]) { TOS = i; }
    }
}

void fRead() {               // (fh--c n)
    byte c;
    CELL fh = TOS;
    push(0);
    if (VALIDF(fh)) {
        TOS = files[fh].read(&c, 1);
        NOS = (CELL)c;
    }
}

void fWrite() {              // (c fh--)
    CELL fh = pop();
    byte c = (byte)pop();
    if (VALIDF(fh)) { files[fh].write(&c, 1); }
}

void fClose() {              // (fh--)
    CELL fh = pop();
    if (VALIDF(fh)) {
        files[fh].close();
        files[fh] = 0;
    }
}
