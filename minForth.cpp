// MinForth.cpp : An extremely memory conscious Forth interpreter

#include "Shared.h"
#include <time.h>

typedef struct {
    const char *name;
    const char *op;
} PRIM_T;

// Words that directly map to VM operations
PRIM_T prims[] = {
    // Math
    { "+", "+" }
    , { "-", "-" }
    , { "/", "/" }
    , { "*", "*" }
    , { "<<", "SL" }
    , { ">>", "SR" }
    , { "ABS", "#0<(_)" }
    , { "/MOD", "&" }
    , { "MOD", "b%" }
    , { "NEGATE", "_" }
    // Memory
    , { "@", "@" }
    , { "C@", "c@" }
    , { "W@", "w@" }
    , { "!", "!" }
    , { "C!", "c!" }
    , { "W!", "w!" }
    , { "+!", "$%@+$!" }
    , { "ALLOT", "xA" }
    // Stack
    , { "DROP", "\\" }
    , { "DUP", "#" }
    , { "OVER", "%" }
    , { "SWAP", "$" }
    , { "2DUP", "%%" }
    , { "2DROP", "\\\\" }
    , { "NIP", "$\\" }
    , { "1-", "D" }
    , { "2*", "#+" }
    , { "2/", "2/" }
    , { "ROT", "Q<$Q>$" }
    , { "-ROT", "$Q<$Q>" }
    , { ".S", "xS" }
    , { "1+", "P" }
    , { "TUCK", "$%" }
    , { "2+", "PP" }
    , { ">R", "Q<" }
    , { "R>", "Q>" }
    , { "R@", "Q@" }
    // Input/output
    , { "(.)", "." }
    , { ".", ".32," }
    , { "CR", "13,10," }
    , { "EMIT", "," }
    , { "KEY", "K@" }
    , { "KEY?", "K?" }
    , { "LOAD", "zL"}
    , { "QTYPE", "t" }
    , { "ZTYPE", "Z" }
    , { "COUNT", "C" }
    , { "TYPE", "T" }
    // Logical / flow control
    , { ".IF", "(" }
    , { ".THEN", ")" }
    , { "FOR", "[" }
    , { "I", "I" }
    , { "+I", "M" }
    , { "NEXT", "]" }
    , { "UNLOOP", "^" }
    , { "BEGIN", "{" }
    , { "WHILE", "}" }
    , { "UNTIL", "~}" }
    , { "=", "=" }
    , { "<", "<" }
    , { ">", ">" }
    , { "<=", ">~" }
    , { ">=", "<~" }
    , { "<>", "=~" }
    , { "!=", "=~" }
    , { "0=", "~" }
    , { "EXIT", ";" }
    // Binary/bitwise
    , { "AND", "b&" }
    , { "OR", "b|" }
    , { "XOR", "b^" }
    , { "COM", "b~" }
    // System
    , { "BL", "32" }
    , { "BYE", "xQ" }
    , { "CELL", "4" }
    , { "EXECUTE", "G" }
    , { "MAX", "%%<($)\\" }
    , { "MIN", "%%>($)\\" }
    , { "WAIT", "zW" }
    , { "NOP", " " }
    , { "NOT", "~" }
    , { "RAND", "zR" }
    , { "RESET", "Y" }
    , { "SPACE", "32," }
    , { "SYSTEM", "xY" }
    , { "TIMER", "zT" }
    , { "+TMPS", "l+" }
    , { "-TMPS", "l-" }
    , { "WORDS", "xD" }
#if __BOARD__ != PC
        // Pin operations for dev boards
    , { "pin-input","zPI" }       // open input
    , { "pin-output","zPO" }      // open output
    , { "pin-pullup","zPU" }      // open input-pullup
    , { "analog-read","zAR" }     // analog read
    , { "analog-write","zAW" }    // analog write
    , { "digital-read","zDR" }    // digital read
    , { "digital-write","zDW" }   // digital write
#endif
#ifdef __EDITOR__
    , { "EDIT","zE" }         // |EDIT|zE|(n--)|Edit block n|
#endif
#ifdef __GAMEPAD__
    // Extensions
    , { "gp-button","xGB" }
#endif
    , {0,0}
};

char word[32], *in;
CELL HERE, oHERE, LAST, STATE, tempWords[10];
byte *VHERE, *oVHERE, isBye=0;

void CComma(CELL v) { code[HERE++] = (byte)v; }
void Comma(CELL v) { SET_LONG(&code[HERE], v); HERE += CELL_SZ; }
void WComma(WORD v) { SET_WORD(&code[HERE], v); HERE += 2; }

char lower(char c) { return BTW(c, 'A', 'Z') ? (c + 32) : c; }

byte strEq(const char *x, const char *y) {
    while (*x && *y && (*x == *y)) { ++x; ++y; }
    return (*x || *y) ? 0 : 1;
}

byte strEqI(const char *x, const char *y) {
    while (*x && *y) {
        if (lower(*x) != lower(*y)) { return 0; }
        ++x; ++y;
    }
    return (*x || *y) ? 0 : 1;
}

char *strCpy(char *d, const char *s) {
    while (*s) { *(d++) = *(s++); }
    *d = 0;
    return d;
}

int strLen(const char *str) {
    int l = 0;;
    while (*(str++)) { ++l; }
    return l;
}

void printStringF(const char *fmt, ...) {
    char *buf = (char*)&code[CODE_SZ-100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 100, fmt, args);
    va_end(args);
    printString(buf);
}

void doExec() {
    if (STATE) {
        oHERE = HERE;
        oVHERE = VHERE;
    }
    else {
        CComma(0);
        run((WORD)oHERE);
        HERE = oHERE;
    }
}

int isTempWord(const char *nm) {
    return ((nm[0] == 'T') && BTW(nm[1], '0', '9') && (nm[2] == 0));
}

void doCreate(const char *name, byte f) {
    if (isTempWord(name)) {
        tempWords[name[1]-'0'] = HERE;
        return;
    }
    DICT_T *dp = DP_AT(HERE);
    dp->prev = (byte)(HERE - LAST);
    dp->flags = f;
    strCpy(dp->name, name);
    LAST = HERE;
    HERE += strLen(name) + 3;
}

int doFind(const char *name) {
    // Temporary word?
    if (isTempWord(name) && (tempWords[name[1]-'0'])) {
        push(tempWords[name[1]-'0']);
        push(0);
        return 1;
    }

    // Regular lookup
    CELL def = (WORD)LAST;
    while (def) {
        DICT_T* dp = DP_AT(def);
        if (strEq(dp->name, name)) {
            push(def + strLen(dp->name) + 3);
            push(dp->flags);
            return 1;
        }
        if (def == dp->prev) break;
        def -= dp->prev;
    }
    return 0;
}

void doWords() {
    CELL l = (WORD)LAST, n = 0;
    while (l) {
        DICT_T *dp = DP_AT(l);
        printString(dp->name);
        if ((++n)%10==0) { printChar('\n'); }
        else { printChar(9); }
        if (l == dp->prev) break;
        l -= dp->prev;
    }
}

int getWord(char *wd) {
    while (*in && (*in < 33)) { ++in; }
    int l = 0;
    while (*in && (32 < *in)) {
        *(wd++) = *(in++);
        ++l;
    }
    *wd = 0;
    return l;
}

int doNumber() {
    CELL num = pop();
    if ((BTW(num,32,126))) {
        CComma('\'');
        CComma(num);
    } else if ((num & 0xFF) == num) {
        CComma(1);
        CComma(num);
    }
    else if ((num & 0xFFFF) == num) {
        CComma(2);
        WComma((WORD)num);
    }
    else {
        CComma(4);
        Comma(num);
    }
    return 1;
}

int doNumber2() {
    if (TOS < 0) { return doNumber(); }
    char buf[16];
    sprintf(buf, "%d", pop());
    if (HERE && BTW(code[HERE - 1], '0', '9')) { CComma(' '); }
    for (int i=0; buf[i]; i++) { CComma(buf[i]); }
    return 1;
}

int isNum(const char *wd) {
    CELL x = 0;
    int base = BASE, isNeg = 0, lastCh = '9';
    if ((wd[0]=='\'') && (wd[2]==wd[0]) && (wd[3]==0)) { push(wd[1]); return 1; }
    if (*wd == '#') { base = 10;  ++wd; }
    if (*wd == '$') { base = 16;  ++wd; }
    if (*wd == '%') { base = 2;  ++wd; lastCh = '1'; }
    if (base < 10) { lastCh = '0' + base - 1; }
    if ((*wd == '-') && (base == 10)) { isNeg = 1;  ++wd; }
    if (*wd == 0) { return 0; }
    while (*wd) {
        char c = *(wd++);
        int t = -1;
        if (BTW(c, '0', lastCh)) { t = c - '0'; }
        if ((base == 16) && (BTW(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((base == 16) && (BTW(c, 'a', 'f'))) { t = c - 'a' + 10; }
        if (t < 0) { return 0; }
        x = (x * base) + t;
    }
    if (isNeg) { x = -x; }
    push(x);
    return 1;
}

char *isRegOp(const char *wd) {
    if ((wd[0] == 'r') && BTW(wd[1], '0', '9') && (!wd[2])) { return (char*)wd; }
    if ((wd[0] == 's') && BTW(wd[1], '0', '9') && (!wd[2])) { return (char*)wd; }
    if ((wd[0] == 'i') && BTW(wd[1], '0', '9') && (!wd[2])) { return (char*)wd; }
    if ((wd[0] == 'd') && BTW(wd[1], '0', '9') && (!wd[2])) { return (char*)wd; }
    return 0;
}

int doPrim(const char *wd) {
    // Words minForth can map directly into its VML (Virtual Machine Language)
    const char *vml = isRegOp(wd);

    for (int i = 0; prims[i].op && (!vml); i++) {
        if (strEqI(prims[i].name, wd)) { vml = prims[i].op; }
    }

    if (!vml) { return 0; } // Not found

    if (BTW(vml[0],'0','9') && BTW(code[HERE-1],'0','9')) { CComma(' '); }
    for (int j = 0; vml[j]; j++) { CComma(vml[j]); }
    return 1;
}

int doQuote() {
    in++;
    push((CELL)VHERE);
    doNumber();
    while (*in && (*in != '"')) { *(VHERE++) = *(in++); }
    *(VHERE++) = 0;
    if (*in) { ++in; }
    return 1;
}

int doDotQuote() {
    ++in;
    CComma('"');
    while (*in && (*in != '"')) { CComma(*(in++)); }
    CComma('"');
    if (*in) { ++in; }
    return 1;
}

int doWord() {
    CELL flg = pop();
    CELL xt = pop();
    if (flg) {
        doExec();
        run((WORD)xt);
    } else {
        CComma(':');
        WComma((WORD)xt);
    }
    return 1;
}

int doParseWord(char *wd) {
    if (strEq(word, "//")) { return 0; }
    if (strEq(word, "\\")) { return 0; }
    if (doPrim(wd))        { return 1; }
    if (doFind(wd))        { return doWord(); }
    if (isNum(wd))         { return doNumber2(); }
    if (strEq(wd, ".\""))  { return doDotQuote(); }
    if (strEq(wd, "\""))   { return doQuote(); }

    if (strEq(wd, ":")) {
        doExec();
        if (getWord(wd) == 0) { return 0; }
        doCreate(wd, 0);
        STATE = 1;
        return 1;
    }

    if (strEq(wd, ";")) {
        CComma(';');
        doExec();
        STATE = 0;
        return 1;
    }

    if (strEq(wd, "(")) {
        while (*in && (*in != ')')) { ++in; }
        if (*in == ')') { ++in; }
        return 1;
    }

    if (strEqI(wd, "IF")) {
        CComma('?');
        push(HERE);
        WComma(0);
        return 1;
    }

    if (strEqI(wd, "ELSE")) {
        CELL tgt = pop();
        CComma('J');
        push(HERE);
        WComma(0);
        SET_WORD(CA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "THEN")) {
        CELL tgt = pop();
        SET_WORD(CA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "VARIABLE")) {
        // NOTE: variable is just a CONSTANT to oVHERE
        push((CELL)oVHERE);
        oVHERE += CELL_SZ;
        VHERE = oVHERE;
        strCpy(wd, "CONSTANT");
    }

    if (strEqI(wd, "CONSTANT")) {
        if (getWord(wd)) {
            doCreate(wd, 0);
            doNumber();
            CComma(';');
            STATE = 1;
            doExec();
            STATE = 0;
            return 1;
        }
        else { return 0; }
    }

    if (strEqI(wd, "FORGET")) {
        // Forget the last word
        oHERE = HERE = LAST;
        LAST -= code[LAST];
        return 1;
    }

    STATE = 0;
    printStringF("[%s]??", wd);
    return 0;
}

bool isASM(const char* ln) {
    if ((ln[0]=='s') && (ln[1]==':') && (ln[2]==' ')) {
        run((byte*)ln-code+3);
        return 1;
    }
    return 0;
}

const char* xln;
void doParse(const char *line) {
    xln = line;
    in = (char*)line;
    if (isASM(line)) { return; }
    int len = getWord(word);
    while (0 < len) {
        if (HERE < oHERE) { HERE = oHERE; }
        if (VHERE < oVHERE) { VHERE = oVHERE; }
        if (doParseWord(word) == 0) { return; }
        len = getWord(word);
    }
    doExec();
}

void doOK() {
    if (STATE) { printString(" ... "); return; }
    printString("\r\nOK ");
    doDotS();
    printString(">");
}

char *rtrim(char *str) {
    char *cp = str;
    while (*cp) { ++cp; }
    --cp;
    while ((str <= cp) && (*cp <= ' ')) { *(cp--) = 0; }
    return str;
}

void systemWords() {
    oHERE = HERE;
    oVHERE = VHERE;
    char *cp = (char*)(VHERE + 6);
    sprintf(cp, ": cb %lu ;", (UCELL)code);     doParse(cp);
    sprintf(cp, ": vb %lu ;", (UCELL)vars);     doParse(cp);
    sprintf(cp, ": csz %d ;", CODE_SZ);         doParse(cp);
    sprintf(cp, ": vsz %d ;", VARS_SZ);         doParse(cp);
    sprintf(cp, ": ha %lu ;", (UCELL)&HERE);    doParse(cp);
    sprintf(cp, ": la %lu ;", (UCELL)&LAST);    doParse(cp);
    sprintf(cp, ": va %lu ;", (UCELL)&VHERE);   doParse(cp);
    sprintf(cp, ": base %lu ;", (UCELL)&BASE);  doParse(cp);
}

#if __BOARD__ == PC
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

byte *doExt(CELL ir, byte *pc) {
    switch (ir) {
    case 'E': doEditor();                       break;
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

#endif
