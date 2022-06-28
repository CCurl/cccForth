// File support for generic boards

void fileInit() {
    myFS.begin();
    printString("\r\nLittleFS: initialized");
    FSInfo fs_info;
    myFS.info(fs_info);
    printStringF("\r\nLittleFS: Total: %ld", fs_info.totalBytes);
    printStringF("\r\nLittleFS: Used: %ld", fs_info.usedBytes);
}

void fOpen() {
  // (fn md--fh)
  CELL md = pop();
  char *fn = (char *)pop();
  push(0);
}
void fRead() {
  // (fh--c n)
  CELL fh = pop();
  push(0);
  push(0);
}
void fWrite() {
  // (c fh--)
  CELL fh = pop();
  CELL c = pop();
}
void fClose() {
  // (fh--)
  CELL fh = pop();
}
