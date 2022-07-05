// Support for the PC
// NOTE: this is a .inc file because the Arduino IDE only knows about .CPP and .H files
// If it were a .CPP file, it would need a #if around the entire thing.
// Making it a .INC file avoids that need.

FILE *input_fp = NULL;
FILE *fpStk[10];
byte fpSP = 0;

CELL timer() { return clock(); }
void doSleep() { Sleep(pop()); }

void fpPush(FILE *v) { if (fpSP < 9) { fpStk[++fpSP] = v; } }
FILE *fpPop() { return (fpSP) ? fpStk[fpSP--] : 0 ; }

FILE *myFopen(const char *fn, const char *md) {
    FILE *fp;
#ifdef _WIN32
    fopen_s(&fp, fn, md);
#else
    FILE *fp = fopen(fn, md);
#endif
    return fp;
}

void fOpen() {
    CELL mode = pop();
    char *fn = (char *)pop();
    FILE *fp = myFopen(fn, mode ? "wb" : "rb");
    push((CELL)fp);
}

void fClose() {
    CELL fp = pop();
    if (fp) { fclose((FILE *)fp); }
}

void fRead() {
    FILE *fp = (FILE *)pop();
    push(0); push(0);
    if (fp && (!feof(fp))) {
        NOS = fgetc(fp);
        TOS = 1;
    }
}

void fWrite() {
    FILE *fp = (FILE *)pop();
    CELL val = pop();
    if (fp) { fputc(val, fp); }
}

void fDelete() {
}

void fList() {
}

void fSave() {
    FILE *fp = myFopen("system.ccc", "wb");
    if (fp) {
        fwrite(&st, sizeof(st), 1, fp);
        fclose(fp);
        printString("-saved-");
    } else { printString("-error-"); }
}

void fLoad() {
    FILE *fp = myFopen("system.ccc", "rb");
    if (fp) {
        vmReset();
        fread(&st, sizeof(st), 1, fp);
        fclose(fp);
        printString("-loaded-");
    } else { printString("-error-"); }
}

void doLoad(int blk) {
    char *fn = (char*)(&st.vars[VARS_SZ-32]);
    sprintF(fn, "./block-%03d.4th", blk);
    FILE *fp = myFopen(fn, "rt");
    if (fp) {
        if (input_fp) { fpPush(input_fp); }
        input_fp = fp;
    }
}

long doRand() {
    static long seed = clock();
    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed <<  5);
    return seed;
}

byte *doExt(CELL ir, byte *pc) {
    switch (ir) {
    case 'E': doEditor();                       break;
    case 'F': return doFile(ir, pc);
    case 'L': doLoad(pop());                    break;
    case 'R': push(doRand());                   break;
    case 'T': push(clock());                    break;
    case 'W': if (TOS) { Sleep(TOS); } pop();   break;
    default: printString("-unk ext-");
    }
    return pc;
}

void printString(const char *cp) { fputs(cp, stdout); }
void printChar(char c) { putc(c, stdout); }
int charAvailable(void) { return _kbhit(); }
int getChar(void) { return _getch(); }

void doHistory(const char *txt) {
    FILE* fp = myFopen("history.txt", "at");
    if (fp) {
        fputs(txt, fp);
        fputs("\n", fp);
        fclose(fp);
    }
}

int isBS(char c) {
    return c==8 ? 1 : 0;
}

void handleChar(char c) {
    static int tib = 0, tibEnd = 0;
    if (!tib) { tibEnd = tib = CODE_SZ-128; }
    if ((c==13) || (c==10)) {
        st.code[tibEnd] = 0;
        char *l = (char*)CA(tib);
        if (!input_fp) {
            printChar(' ');
            doHistory(l); 
        }
        doParse(l);
        tibEnd = tib;
        if (!input_fp) { doOK(); }
        return;
    }
    if (c == 9) { c = 32; }
    if (BTW(c,32,126)) {
        st.code[tibEnd++] = c;
        if (!input_fp) { printChar(c); }
        return;
    }
    if (isBS(c) && (tib < tibEnd)) {
        --tibEnd;
        char bs[4] = {8,32,8,0};
        printString(bs);
    }
}

void setup() {
    printString("cccForth v0.0.1 - Chris Curl\n");
    vmReset();
    if (!input_fp) { doOK(); }
}

void loop() {
    char *tib = (char *)CA(st.HERE+32);
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (input_fp) {
        if (!feof(input_fp)) { handleChar(fgetc(input_fp)); }
        else {
            fclose(input_fp);
            input_fp = fpPop();
            if (!input_fp) { doOK(); }
        }
    } else {
        while (charAvailable()) { handleChar(getChar()); }
    }
    WORD ar = GET_WORD(st.code);
    if (ar) { run(ar); }
}

int main(int argc, char *argv[]) {
    if (sizeof(&st.HERE) > CELL_SZ) {
        printf("ERROR: CELL cannot support a pointer!");
        exit(1);
    }
    if (argc > 1) { input_fp = myFopen(argv[1], "rt"); }
    else { doLoad(0); }
    setup();

    while (!isBye) { loop(); }
    return 0;
}
