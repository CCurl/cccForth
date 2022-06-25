// Support for the PC
// NOTE: this is a .inc file because the Arduino IDE only knows about .CPP and .H files
// If it were a .CPP file, it would need a #if around the entire thing.
// Making it a .INC file avoids that need.

FILE *input_fp = NULL;
FILE *fpStk[10];
byte fpSP = 0;

void fpPush(FILE *v) { if (fpSP < 9) { fpStk[++fpSP] = v; } }
FILE *fpPop() { return (fpSP) ? fpStk[fpSP--] : 0 ; }

void doLoad(int blk) {
    char *fn = (char*)(VHERE + 100);
    sprintf(fn, "./block-%03d.4th", blk);
    FILE *fp = fopen(fn, "rt");
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

byte *doFile(CELL ir, byte *pc) {
    ir = *(pc++);
    if (ir=='O') { ir=pop(); TOS=(CELL)fopen((char*)TOS,ir?"wb":"rb"); }
    if (TOS==0) { printString("-nf-"); return pc; }
    else if (ir=='R') { char b; ir=fread(&b,1,1,(FILE*)TOS); TOS=b; push(ir); }
    else if (ir=='W') { CELL b=NOS; fwrite(&b,1,1,(FILE*)TOS); DROP2; }
    else if (ir=='C') { fclose((FILE*)pop()); }
    return pc;
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
    FILE* fp = fopen("history.txt", "at");
    if (fp) {
        fputs(txt, fp);
        fclose(fp);
    }
}

void loop() {
    char *tib = (char *)CA(HERE+32);
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (input_fp) {
        if (fgets(tib, 128, fp) == tib) {
            doParse(rtrim(tib));
        } else {
            fclose(input_fp);
            input_fp = fpPop();
        }
    } else {
        doOK();
        fgets(tib, 128, stdin);
        doHistory(tib);
        doParse(rtrim(tib));
    }
}

int main(int argc, char *argv[]) {
    if (sizeof(&HERE) > CELL_SZ) {
        printf("ERROR: CELL cannot support a pointer!");
        exit(1);
    }
    vmReset();
    if (argc > 1) { input_fp = fopen(argv[1], "rt"); }
    else { doLoad(0); }
    while (!isBye) { loop(); }
    return 0;
}
