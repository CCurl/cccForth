// file-teensy.h
// File ops on the Teensy are different

void doFopen() {
  // (fn md--fh)
  char *md = (char *)pop();
  char *fn = (char *)pop();
  push(0);
}
void doFread() {
  // (fh--c n)
  CELL fh = pop();
  push(0);
  push(0);
}
void doFwrite() {
  // (c fh--)
  CELL fh = pop();
  CELL c = pop();
}
void doFclose() {
  // (fh--)
  CELL fh = pop();
}
