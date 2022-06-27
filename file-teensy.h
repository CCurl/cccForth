// file-teensy.h
// File ops on the Teensy are different

void fOpen() {
  // (fn md--fh)
  char *md = (char *)pop();
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
