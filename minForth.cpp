// MinForth.cpp : An extremely memory conscious Forth interpreter

#include "Shared.h"
#include <time.h>

extern int isBye;

typedef struct {
    const char *name;
    const char *op;
} PRIM_T;

// Words that directly map to VM operations
PRIM_T prims[] = {
    { "+", "+" }              // |+|+|(a b--c)|FORTH CORE|
    , { "-", "-" }            // |-|-|(a b--c)|FORTH CORE|
    , { "/", "/" }            // |/|/|(a b--c)|FORTH CORE|
    , { "*", "*" }            // |*|*|(a b--c)|FORTH CORE|
    , { "/MOD", "&" }         // |/MOD|&|(a b--q r)|FORTH CORE|
    , { "MOD", "M" }          // |MOD|M|(a b--c)|FORTH CORE|
    , { "SWAP", "$" }         // |SWAP|$|(a b--b a)|FORTH CORE|
    , { "DROP", "\\" }        // |DROP|\\|(a b--a)|FORTH CORE|
    , { "OVER", "%" }         // |OVER|%|(a b--a b a)|FORTH CORE|
    , { "DUP", "#" }          // |DUP|#|(a--a a)|FORTH CORE|
    , { "NIP", "$\\" }        // |NIP|$\\|(a b--b)|FORTH CORE|
    , { "TUCK", "$%" }        // |TUCK|$%|(a b--b a b)|FORTH CORE|
    , { "2DUP", "%%" }        // |2DUP|%%|(a b--a b a b)|FORTH CORE|
    , { "2DROP", "\\\\" }     // |2DROP|\\\\|(a b--)|FORTH CORE|
    , { "EMIT", "," }         // |EMIT|,|(c--)|FORTH CORE|
    , { "(.)", "." }          // |(.)|.|(n--)|FORTH CORE|
    , { "SPACE", "32," }      // |SPACE|32,|(--)|FORTH CORE|
    , { "CR", "13,10," }      // |CR|13,10,|(--)|FORTH CORE|
    , { "BL", "32" }          // |BL|32|(--c)|FORTH CORE|
    , { "CELL", "4" }         // |CELL|4|(--n)|FORTH CORE|
    , { "=", "=" }            // |=|=|(a b--f)|FORTH CORE|
    , { "<", "<" }            // |<|<|(a b--f)|FORTH CORE|
    , { ">", ">" }            // |>|>|(a b--f)|FORTH CORE|
    , { "<=", ">N" }          // |<=|>N|(a b--f)|FORTH CORE|
    , { ">=", "<N" }          // |>=|<N|(a b--f)|FORTH CORE|
    , { "<>", "=N" }          // |<>|=N|(a b--f)|FORTH CORE|
    , { "!=", "=N" }          // |!=|=N|(a b--f)|FORTH CORE|
    , { "0=", "N" }           // |0=|N|(a b--f)|FORTH CORE|
    , { "ABS", "#0<(_)" }     // |ABS|#0<(_)|(a--b)|FORTH CORE|
    , { "NEGATE", "_" }       // |NEGATE|_|(a--b)|FORTH CORE|
    , { "<<", "L" }           // |<<|L|(a b--c)|FORTH CORE|
    , { ">>", "R" }           // |>>|R|(a b--c)|FORTH CORE|
    , { "ZLEN", "T" }         // |ZLEN|T|(a--n)|FORTH CORE|
    , { ".", ".32," }         // |.|.32,|(n--)|FORTH CORE|
    , { "@", "@" }            // |@|@|(a--n)|FORTH CORE|
    , { "C@", "c" }           // |C@|c|(a--c)|FORTH CORE|
    , { "W@", "w" }           // |W@|w|(a--w)|FORTH CORE|
    , { "!", "!" }            // |!|!|(n a--)|FORTH CORE|
    , { "C!", "C" }           // |C!|C|(n a--)|FORTH CORE|
    , { "W!", "W" }           // |W!|W|(n a--)|FORTH CORE|
    , { "AND", "a" }          // |AND|a|(a b--c)|FORTH CORE|
    , { "OR", "o" }           // |OR|o|(a b--c)|FORTH CORE|
    , { "XOR", "x" }          // |XOR|x|(a b--c)|FORTH CORE|
    , { "COM", "~" }          // |COM|~|(a--b)|FORTH CORE|
    , { "NOT", "N" }          // |NOT|N|(a--b)|FORTH CORE|
    , { "1+", "P" }           // |1+|P|(a--b)|FORTH CORE|
    , { "2+", "PP" }          // |2+|PP|(a--b)|FORTH CORE|
    , { "4+", "PPPP" }        // |4+|PPPP|(a--b)|FORTH CORE|
    , { "+!", "$%@+$!" }      // |+!|$%@+$!|(n a--)|FORTH CORE|
    , { "1-", "D" }           // |1-|D|(a--b)|FORTH CORE|
    , { "I", "I" }            // |I|I|(--c)|FORTH CORE|
    , { "+I", "m" }           // |+I|m|(n--)|FORTH CORE|
    , { "EXECUTE", "G" }      // |EXECUTE|G|(a--)|FORTH CORE|
    , { "MIN", "%%>($)\\" }   // |MIN|%%>($)\\|(a b--c)|FORTH CORE|
    , { "MAX", "%%<($)\\" }   // |MAX|%%<($)\\|(a b--c)|FORTH CORE|
    , { "RAND", "zR" }        // |RAND|zR|(--n)|FORTH CORE|
    , { "EXIT", ";" }         // |EXIT|;|(--)|FORTH CORE|
    , { "TIMER", "zT" }       // |TIMER|zT|(--n)|FORTH CORE|
    , { "WAIT", "zW" }        // |WAIT|zW|(n--)|FORTH CORE|
    , { "RESET", "Y" }        // |RESET|Y|(--)|FORTH CORE|
    , { "UNLOOP", "k" }       // |UNLOOP|k|(--)|FORTH CORE|
    , { "LEAVE", "l" }        // |LEAVE|l|(--)|FORTH CORE|
    , { "KEY", "K" }          // |KEY|K|(--c)|FORTH CORE|
    , { "KEY?", "?" }         // |KEY?|?|(--f)|FORTH CORE|
    , { "+TMPS", "p" }        // |+TMPS|p|(--)|FORTH CORE|
    , { "-TMPS", "q" }        // |-TMPS|q|(--)|FORTH CORE|
    , { "ZTYPE", "Z" }        // |ZTYPE|Z|(a--)|FORTH CORE|
    , { "QTYPE", "t" }        // |QTYPE|t|(a--)|FORTH CORE|
    , { ">R", "Q<" }          // |>R|Q<|(n--)|FORTH CORE|
    , { "R>", "Q>" }          // |R>|Q>|(--n)|FORTH CORE|
    , { "R@", "Q@" }          // |R@|Q@|(--n)|FORTH CORE|
    , { "ROT", "Q<$Q>$" }     // |ROT|Q<$Q>$|(a b c--b c a)|FORTH CORE|
    , { "-ROT", "$Q<$Q>" }    // |-ROT|$Q<$Q>|(a b c--c a b)|FORTH CORE|
    , { ".IF", "(" }          // |.IF|(|(f--)|FORTH CORE|
    , { ".THEN", ")" }        // |.THEN|)|(--)|FORTH CORE|
    , { ".S", "zS" }          // |.S|zS|(--)|FORTH CORE|
    , { "NOP", "" }           // |NOP||(--)|FORTH CORE|
    // Extensions
#if __BOARD__ == PC
    , {"BYE","zZ"}            // |BYE|zZ|(--)|FORTH CORE|
    , {"LOAD","zL"}           // |LOAD|zL|(n--)|FORTH CORE|
#else
        // Pin operations for dev boards
    , { "pin-output","zPO" }      // open output
    , { "pin-input","zPI" }       // open input
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
byte lastWasCall = 0;
byte *VHERE, *VHERE2;
CELL HERE, LAST, STATE, tempWords[10];

void CComma(CELL v) { code[HERE++] = (byte)v; }
void Comma(CELL v) { SET_LONG(&code[HERE], v); HERE += CELL_SZ; }
void WComma(WORD v) { SET_WORD(&code[HERE], v); HERE += 2; }

char lower(char c) { return betw(c, 'A', 'Z') ? (c + 32) : c; }

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

int isTempWord(const char *nm) {
    return ((nm[0] == 'T') && betw(nm[1], '0', '9') && (nm[2] == 0));
}

void doCreate(const char *name, byte f) {
    if (isTempWord(name)) {
        tempWords[name[1] - '0'] = HERE;
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
    if (isTempWord(name) && (tempWords[name[1] - '0'])) {
        push(tempWords[name[1] - '0']);
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

int doWords() {
    CELL l = (WORD)LAST;
    while (l) {
        DICT_T *dp = DP_AT(l);
        printString(dp->name);
        printChar(' ');
        if (l == dp->prev) break;
        l -= dp->prev;
    }
    return 1;
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

int execWord() {
    CELL f = pop(), xt = pop();
    if ((STATE) && (f == 0)) {
        CComma(':');
        WComma((WORD)xt);
        lastWasCall = 1;
    } else { run((WORD)xt); }
    return 1;
}

int doNumber(int compile) {
    if (compile) {
        CELL num = pop();
        if ((num & 0xFF) == num) {
            CComma(1);
            CComma(num & 0xff);
        }
        else if ((num & 0xFFFF) == num) {
            CComma(2);
            WComma((WORD)num);
        }
        else {
            CComma(4);
            Comma(num);
        }
    }
    return 1;
}

int doNumber2(int compile) {
    if (compile) {
        if (betw(code[HERE-1],'0','9')) { CComma(' '); }
        char buf[16];
        sprintf(buf, "%d", pop());
        for (int i=0; buf[i]; i++) { CComma(buf[i]); }
    }
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
        if (betw(c, '0', lastCh)) { t = c - '0'; }
        if ((base == 16) && (betw(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((base == 16) && (betw(c, 'a', 'f'))) { t = c - 'a' + 10; }
        if (t < 0) { return 0; }
        x = (x * base) + t;
    }
    if (isNeg) { x = -x; }
    push(x);
    return 1;
}

char *isRegOp(const char *wd, char *out) {
    if (betw(wd[0], 'r', 's') && betw(wd[1], '0', '9') && (!wd[2])) { return (char *)wd; }
    if ((wd[0] == 'i')        && betw(wd[1], '0', '9') && (!wd[2])) { return (char *)wd; }
    if ((wd[0] == 'd')        && betw(wd[1], '0', '9') && (!wd[2])) { return (char *)wd; }

    return 0;
}

int doPrim(const char *wd) {
    // Words minForth can map directly into its VML (Virtual Machine Language)
    char x[3];
    const char *vml = vml = isRegOp(wd, x);

    for (int i = 0; prims[i].op && (!vml); i++) {
        if (strEqI(prims[i].name, wd)) { vml = prims[i].op; }
    }

    if (!vml) { return 0; } // Not found

    if (STATE) {
        if (betw(vml[0],'0','9') && betw(code[HERE-1],'0','9')) { CComma(' '); }
        for (int j = 0; vml[j]; j++) { CComma(vml[j]); }
    } else {
        byte* x = CA(HERE + 10);
        for (int j = 0; vml[j]; j++) { *(x++) = vml[j]; }
        *x = ';';
        run((WORD)HERE + 10);
    }
    return 1;
}

int doQuote(int isDotQ) {
    in++;
    if (STATE) { VHERE2 = VHERE; }
    push((CELL)VHERE2);
    while (*in && (*in != '"')) { *(VHERE2++) = *(in++); }
    *(VHERE2++) = 0;
    if (*in) { ++in; }
    if (STATE) {
        doNumber(STATE);
        VHERE = VHERE2;
        if (isDotQ) { CComma('Z'); }
    }
    return 1;
}

int doDotQuote() {
    doQuote(1);
    if (STATE == 0) {
        byte* cp = CA(HERE+10);
        *(cp++) = 'Z';
        *(cp++) = ';';
        run((WORD)(HERE+10));
    }
    return 1;
}

int doParseWord(char *wd) {
    byte lwc = lastWasCall;
    lastWasCall = 0;

    if (doPrim(wd))       {  return 1; }
    if (doFind(wd))       {  return execWord(); }
    if (isNum(wd))        {  return doNumber(STATE); }
    if (strEq(wd, ".\"")) {  return doDotQuote(); }
    if (strEq(wd, "\""))  {  return doQuote(0); }

    if (strEq(wd, ":")) {
        if (getWord(wd) == 0) { return 0; }
        doCreate(wd, 0);
        STATE = 1;
        return 1;
    }

    if (strEq(wd, "(")) {
        while (*in && (*in != ')')) { ++in; }
        if (*in == ')') { ++in; }
        return 1;
    }

    if (strEq(wd, ";")) {
        STATE = 0;
        // Simple tail-call optimization
        if (lwc && (code[HERE - 3] == ':')) { code[HERE - 3] = 'J'; }
        else { CComma(';'); }
        return 1;
    }

    if (strEqI(wd, "IMMEDIATE")) { DP_AT(LAST)->flags |= 1; return 1; }
    if (strEqI(wd, "ALLOT")) { VHERE += pop();              return 1; }
    if (strEqI(wd, "WORDS")) { return doWords(); }

    if (strEqI(wd, "IF")) {
        CComma('j');
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

    if (strEqI(wd, "FOR")) {
        CComma('[');
        push(HERE);
        WComma(0);
        return 1;
    }

    if (strEqI(wd, "NEXT")) {
        CComma(']');
        CELL tgt = pop();
        SET_WORD(CA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "BEGIN")) {
        CComma('{');
        push(HERE);
        WComma(0);
        return 1;
    }

    if (strEqI(wd, "AGAIN")) {
        CComma('}');
        CELL tgt = pop();
        SET_WORD(CA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "WHILE")) {
        CComma('v');
        CELL tgt = pop();
        SET_WORD(CA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "UNTIL")) {
        CComma('u');
        CELL tgt = pop();
        SET_WORD(CA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "VARIABLE")) {
        // NOTE: variable is just a CONSTANT to VHERE
        push((CELL)VHERE);
        VHERE += CELL_SZ;
        strCpy(wd, "CONSTANT");
    }

    if (strEqI(wd, "CONSTANT")) {
        if (getWord(wd)) {
            doCreate(wd, 0);
            doNumber(1);
            CComma(';');
            return 1;
        }
        else { return 0; }
    }

    if (strEqI(wd, "FORGET")) {
        // Forget the last word
        HERE = LAST;
        LAST -= code[LAST];
        return 1;
    }

    STATE = 0;
    VHERE2 = VHERE;
    printStringF("[%s]??", wd);
    return 0;
}

extern FIB_T st;
extern void R(int);
int doS2(const char *l) {
    if (l[0]!='s') return 0;
    if (l[1]!='2') return 0;
    if (l[2]!=':') return 0;
    if (l[3]!=' ') return 0;
    l += 4;
    int x = 1000, y=x;
    while (*l) {
        st.b[y++]=*(l++);
    }
    st.b[y]=0;
    R(x);
    return 1;
}

void doParse(const char *line) {
    if (doS2(line)) { return; }
    in = (char*)line;
    int len = getWord(word);
    while (0 < len) {
        if (VHERE2 < VHERE) { VHERE2 = VHERE; }
        if (strEq(word, "//")) { return; }
        if (strEq(word, "\\")) { return; }
        if (doParseWord(word) == 0) { return; }
        len = getWord(word);
    }
}

void doDotS() {
    printString("(");
    for (int d = 1; d <= sp; d++) {
        if (1 < d) { printChar(' '); }
        printBase(stk[d], BASE);
    }
    printString(")");
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
FILE* input_fp = NULL;
FILE* fpStk[10];
byte fpSP = 0;

void fpPush(FILE* v) { if (fpSP < 9) { fpStk[++fpSP] = v; } }
FILE* fpPop() { return (fpSP) ? fpStk[fpSP--] : 0 ; }

void doLoad(int blk) {
    char *fn = (char*)(VHERE + 100);
    sprintf(fn, "./block-%03d.4th", blk);
    FILE* fp = fopen(fn, "rt");
    if (fp) {
        if (input_fp) { fpPush(input_fp); }
        input_fp = fp;
    }
}

long doRand() {
    static long seed = 0;
    if (seed==0) { seed = clock(); }
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
    case 'S': doDotS();                         break;
    case 'T': push(clock());             break;
    // case 'W': if (TOS) { Sleep(TOS); } pop();   break;
    case 'Z': isBye = 1;                        break;
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
        return;
    }
}

extern void I();
int main()
{
    printf("MinForth v0.0.1 - Chris Curl\n");
    if (sizeof(&HERE) > CELL_SZ) {
        printf("ERROR: CELL cannot support a pointer!");
        exit(1);
    }
    I();
    vmReset();
    doLoad(0);
    while (!isBye) { loop(); }
}

#endif
