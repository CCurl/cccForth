// Support for the PC
// NOTE: this is a .inc file because the Arduino IDE only knows about .CPP and .H files
// If it were a .CPP file, it would need a #if around the entire thing.
// Making it a .INC file avoids that need.

FILE *input_fp = NULL;
FILE *fpStk[10];
byte fpSP = 0;

CELL doTimer() { return clock(); }
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


byte *doExt(CELL ir, byte *pc) {
    switch (ir) {
    case 'E': doEditor();                       break;
    case 'F': return doFile(ir, pc);
    case 'L': doLoad(pop());                    break;
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
        fclose(fp);
    }
}

void loop() {
    char *tib = (char *)CA(st.HERE+32);
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (input_fp) {
        if (fgets(tib, 128, fp) == tib) {
            doParse(rTrim(tib));
        } else {
            fclose(input_fp);
            input_fp = fpPop();
        }
    } else {
        doOK();
        fgets(tib, 128, stdin);
        doHistory(tib);
        doParse(rTrim(tib));
    }
}

int main(int argc, char *argv[]) {
    if (sizeof(&st.HERE) > CELL_SZ) {
        printf("ERROR: CELL cannot support a pointer!");
        exit(1);
    }
    vmReset();
    if (argc > 1) { input_fp = myFopen(argv[1], "rt"); }
    else { doLoad(0); }
    while (!isBye) { loop(); }
    return 0;
}
