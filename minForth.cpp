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
    , { "MOD", "&$\\" }       // |MOD|&$\\|(a b--c)|FORTH CORE|
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
    , { "SPACE", "b" }        // |SPACE|32,|(--)|FORTH CORE|
    , { "CR", "13,10," }      // |CR|13,10,|(--)|FORTH CORE|
    , { "BL", "32" }          // |BL|32|(--c)|FORTH CORE|
    , { "CELL", "4" }         // |CELL|4|(--n)|FORTH CORE|
    , { "=", "=" }            // |=|=|(a b--f)|FORTH CORE|
    , { "<", "<" }            // |<|<|(a b--f)|FORTH CORE|
    , { ">", ">" }            // |>|>|(a b--f)|FORTH CORE|
    , { "<=", "<=" }          // |<=|>N|(a b--f)|FORTH CORE|
    , { ">=", ">=" }          // |>=|<N|(a b--f)|FORTH CORE|
    , { "<>", "=~" }          // |<>|=N|(a b--f)|FORTH CORE|
    , { "0=", "~" }           // |0=|N|(a b--f)|FORTH CORE|
    , { "ABS", "#0<(_)" }     // |ABS|#0<(_)|(a--b)|FORTH CORE|
    , { "NEGATE", "_" }       // |NEGATE|_|(a--b)|FORTH CORE|
    , { "<<", "L" }           // |<<|L|(a b--c)|FORTH CORE|
    , { ">>", "R" }           // |>>|R|(a b--c)|FORTH CORE|
    , { "ZLEN", "T" }         // |ZLEN|T|(a--n)|FORTH CORE|
    , { ".", ".b"   }         // |.|.b|(n--)|FORTH CORE|
    , { "@", "@" }            // |@|@|(a--n)|FORTH CORE|
    , { "C@", "c@" }          // |C@|c|(a--c)|FORTH CORE|
    , { "W@", "w@" }          // |W@|w|(a--w)|FORTH CORE|
    , { "!", "!" }            // |!|!|(n a--)|FORTH CORE|
    , { "C!", "c!" }          // |C!|C|(n a--)|FORTH CORE|
    , { "W!", "w!" }          // |W!|W|(n a--)|FORTH CORE|
    , { "AND", "b&" }         // |AND|a|(a b--c)|FORTH CORE|
    , { "OR", "b|" }          // |OR|o|(a b--c)|FORTH CORE|
    , { "XOR", "b^" }         // |XOR|x|(a b--c)|FORTH CORE|
    , { "COM", "b~" }         // |COM|~|(a--b)|FORTH CORE|
    , { "NOT", "~" }          // |NOT|N|(a--b)|FORTH CORE|
    , { "1+", "1+" }          // |1+||(a--b)|FORTH CORE|
    , { "1-", "1-" }          // |1-||(a--b)|FORTH CORE|
    , { "+!", "$%@+$!" }      // |+!|$%@+$!|(n a--)|FORTH CORE|
    , { "EXECUTE", "e" }      // |EXECUTE|e|(a--)|FORTH CORE|
    , { "MIN", "%%>($)\\" }   // |MIN|%%>($)\\|(a b--c)|FORTH CORE|
    , { "MAX", "%%<($)\\" }   // |MAX|%%<($)\\|(a b--c)|FORTH CORE|
    , { "RAND", "xR" }        // |RAND|xR|(--n)||
    , { "EXIT", ";" }         // |EXIT|;|(--)|FORTH CORE|
    , { "TIMER", "t" }        // |TIMER|zT|(--n)|FORTH CORE|
    , { "WAIT", "zW" }        // |WAIT|zW|(n--)|FORTH CORE|
    , { "RESET", "xX" }       // |RESET|xX|(--)||
    , { "FOR", "[" }          // |FOR|[|(--)|FORTH CORE|
    , { "I", "i" }            // |I|i|(--)|FORTH CORE|
    , { "+I", "xI" }          // |+I|xI|(n--)|FORTH CORE|
    , { "LEAVE", "xF" }       // |LEAVE|xF|(--)|FORTH CORE|
    , { "NEXT", "]" }         // |NEXT|]|(--)|FORTH CORE|
    , { "BEGIN", "{" }        // |BEGIN|{|(--)|FORTH CORE|
    , { "UNLOOP", "xU" }      // |UNLOOP|uU|(--)|FORTH CORE|
    , { "WHILE", "}" }        // |WHILE|}|(--)|FORTH CORE|
    , { "UNTIL", "~}" }       // |UNTIL|~}|(--)|FORTH CORE|
    , { "KEY", "k@" }         // |KEY|K|(--c)|FORTH CORE|
    , { "KEY?", "k?" }        // |KEY?|?|(--f)|FORTH CORE|
    , { "+TMPS", "l+" }       // |+TMPS|p|(--)|FORTH CORE|
    , { "-TMPS", "l-" }       // |-TMPS|q|(--)|FORTH CORE|
    , { "ZTYPE", "z" }        // |ZTYPE|z|(a--)|FORTH CORE|
    , { "QTYPE", "t" }        // |QTYPE|t|(a--)|FORTH CORE|
    , { ">R", "r<" }          // |>R|Q<|(n--)|FORTH CORE|
    , { "R>", "r>" }          // |R>|Q>|(--n)|FORTH CORE|
    , { "R@", "r@" }          // |R@|Q@|(--n)|FORTH CORE|
    , { "ROT", "r<$r>$" }     // |ROT|Q<$Q>$|(a b c--b c a)|FORTH CORE|
    , { "-ROT", "$r<$r>" }    // |-ROT|$Q<$Q>|(a b c--c a b)|FORTH CORE|
    , { "IF", "(" }           // |IF|(|(f--)|FORTH CORE|
    , { "THEN", ")" }         // |THEN|)|(--)|FORTH CORE|
    , { ".S", "xD" }          // |.S|xS|(--)|FORTH CORE|
    , { "HERE", "xH" }        // |HERE|xH|(--n)|FORTH CORE|
    , { "BASE@", "xB" }       // |BASE@|xB|(n--)||
    , { "BASE!", "xb" }       // |BASE!|xb|(--n)||
    , { "NOP", " " }          // |NOP| |(--)|FORTH CORE|
    // Extensions
#if __BOARD__ == PC
    , {"BYE","xQ"}            // |BYE|zZ|(--)|FORTH CORE|
    , {"LOAD","xL"}           // |LOAD|zL|(n--)|FORTH CORE|
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
    , { "EDIT","xE" }         // |EDIT|zE|(n--)|Edit block n|
#endif
#ifdef __GAMEPAD__
    // Extensions
    , { "gp-button","xGB" }
#endif
    , {0,0}
};

char word[32], *in, exBuf[256];
byte lastWasCall = 0;
#define STATE state

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

int strLen(const char* str) {
    int l = 0;;
    while (*(str++)) { ++l; }
    return l;
}

char *strCpy(char * dst, const char * src) {
    while (*src) { *(dst++) = *(src++); }
    *dst = 0;
    return dst;
}

char *strCat(char *dst, const char *src) {
    dst += strLen(dst);
    strCpy(dst, src);
    return dst;
}

void printStringF(const char *fmt, ...) {
    char buf[100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 100, fmt, args);
    va_end(args);
    ps(buf);
}

int isTempWord(const char *nm) {
    return ((nm[0] == 'T') && betw(nm[1], '0', '9') && (nm[2] == 0));
}

extern FIB_T st;
extern void E(char *);
CELL xt=0, last=0;
DICT_T dict[100];

void exec() {
    if (exBuf[0]) { E(exBuf); }
    exBuf[0] = 0;
}

void doCreate(const char *name, byte f) {
    exec();
    DICT_T* dp = &dict[++last];
    strCpy(dp->name, name);
    dp->flags = f;
    dp->xt = xt++;
    sprintf(exBuf, ":%c%c", dp->xt/26+'A', dp->xt%26+'A');
}

int doFind(const char *name) {
    int x = last;
    while (x) {
        DICT_T* dp = &dict[x--];
        if (strEq(dp->name, name)) {
            push(dp->xt);
            push(dp->flags);
            return 1;
        }
    }
    return 0;
}

int doWords() {
    int x = last;
    char buf[8];
    while (x) {
        DICT_T* dp = &dict[x--];
        CELL xt=dp->xt;
        ps(dp->name);
        sprintf(buf, "(%c%c) ", xt/26+'A', xt%26+'A');
        ps(buf);
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
    char buf[4];
    sprintf(buf,"%c%c", xt/26+'A', xt%26+'A');
    strCat(exBuf, buf);
    return 1;
}

int doNumber() {
    int l = strLen(exBuf);
    char c = (l) ? exBuf[l-1] : 0;
    if (betw(c,'0','9')) { strCat(exBuf, " "); }
    char buf[16];
    sprintf(buf, "%d", pop());
    strCat(exBuf, buf);
    return 1;
}

int isNum(const char *wd) {
    CELL x = 0;
    int b = base, isNeg = 0, lastCh = '9';
    if ((wd[0]=='\'') && (wd[2]==wd[0]) && (wd[3]==0)) { push(wd[1]); return 1; }
    if (*wd == '#') { b = 10;  ++wd; }
    if (*wd == '$') { b = 16;  ++wd; }
    if (*wd == '%') { b = 2;  ++wd; lastCh = '1'; }
    if (b < 10) { lastCh = '0' + b - 1; }
    if ((*wd == '-') && (b == 10)) { isNeg = 1;  ++wd; }
    if (*wd == 0) { return 0; }
    while (*wd) {
        char c = *(wd++);
        int t = -1;
        if (betw(c, '0', lastCh)) { t = c - '0'; }
        if ((b == 16) && (betw(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((b == 16) && (betw(c, 'a', 'f'))) { t = c - 'a' + 10; }
        if (t < 0) { return 0; }
        x = (x * b) + t;
    }
    if (isNeg) { x = -x; }
    push(x);
    return 1;
}

int doPrim(const char *wd) {
    // Words minForth can map directly into its VML (Virtual Machine Language)
    const char *vml = NULL;

    for (int i = 0; prims[i].op && (!vml); i++) {
        if (strEqI(prims[i].name, wd)) { vml = prims[i].op; }
    }

    if (!vml) { return 0; } // Not found

    strCat(exBuf, vml);
    return 1;
}

int doQuote() {
    char buf[2] = { 0,0 };
    strCat(exBuf, "|");
    in++;
    while (*in && (*in != '"')) {
        buf[0] = *(in++);
        strCat(exBuf, buf);
    }
    ++in;
    strCat(exBuf, "|");
    return 1;
}

int doDotQuote() {
    char buf[2] = {0,0};
    strCat(exBuf, "\"");
    in++;
    while (*in && (*in != '"')) {
        buf[0]=*(in++);
        strCat(exBuf, buf);
    }
    ++in;
    strCat(exBuf, "\"");
    return 1;
}

int doParseWord(char *wd) {
    byte lwc = lastWasCall;
    lastWasCall = 0;

    if (doPrim(wd))       {  return 1; }
    if (doFind(wd))       {  return execWord(); }
    if (isNum(wd))        {  return doNumber(); }
    if (strEq(wd, ".\"")) {  return doDotQuote(); }
    if (strEq(wd, "\""))  {  return doQuote(); }

    if (strEq(wd, ":")) {
        if (getWord(wd) == 0) { return 0; }
        doCreate(wd, 0);
        return 1;
    }

    if (strEq(wd, "(")) {
        while (*in && (*in != ')')) { ++in; }
        if (*in == ')') { ++in; }
        return 1;
    }

    if (strEq(wd, ";")) {
        strCat(exBuf, ";");
        exec();
        STATE = 0;
        return 1;
    }

    if (strEqI(wd, "IMMEDIATE")) { dict[last].flags |= 1; return 1; }
    // if (strEqI(wd, "ALLOT")) { VHERE += pop();              return 1; }
    if (strEqI(wd, "WORDS")) { return doWords(); }

    if (strEqI(wd, "VARIABLE")) {
        strCpy(wd, "CONSTANT");
    }

    if (strEqI(wd, "CONSTANT")) {
        if (getWord(wd)) {
            doCreate(wd, 0);
            doNumber();
            strCat(exBuf, ";");
            exec();
            STATE = 0;
            return 1;
        }
        else { return 0; }
    }

    STATE = 0;
    printStringF("[%s]??", wd);
    return 0;
}

extern FIB_T st;
extern void R(int);
int doS2(const char *l) {
    if (l[0]!='s') return 0;
    if (l[1]!=':') return 0;
    l += 2;
    int x = 1000, y=x;
    while (*l) { st.b[y++]=*(l++); }
    st.b[y]=0;
    R(x);
    return 1;
}

void doParse(const char *line) {
    if (doS2(line)) { return; }
    in = (char*)line;
    int len = getWord(word);
    while (0 < len) {
        if (strEq(word, "//")) { return; }
        if (strEq(word, "\\")) { return; }
        if (doParseWord(word) == 0) { return; }
        len = getWord(word);
    }
}

extern void doDotS();
void doOK() {
    if (STATE) { ps(" ... "); return; }
    ps("\r\nOK ");
    doDotS();
    pc('>');
}

char *rtrim(char *str) {
    char *cp = str;
    while (*cp) { ++cp; }
    --cp;
    while ((str <= cp) && (*cp <= ' ')) { *(cp--) = 0; }
    return str;
}

void systemWords() {
    char cp[32];
    exBuf[0] = 0;
    sprintf(cp, ": cb %lu ;", cb);       doParse(cp);
    sprintf(cp, ": vmsz %d ;", VMSZ);    doParse(cp);
}

#if __BOARD__ == PC
FILE* input_fp = NULL;
FILE* fpStk[10];
byte fpSP = 0;

void fpPush(FILE* v) { if (fpSP < 9) { fpStk[++fpSP] = v; } }
FILE* fpPop() { return (fpSP) ? fpStk[fpSP--] : 0 ; }

void doLoad(int blk) {
    char fn[16];
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

int doExt(CELL ir, int pc) {
    switch (ir) {
    case 'E': doEditor();                break;
    case 'L': doLoad(pop());             break;
    case 'R': push(doRand());            break;
    case 'S': doDotS();                  break;
    case 'T': push(clock());             break;
    // case 'W': if (TOS) { Sleep(TOS); } pop();   break;
    case 'Z': isBye = 1;                 break;
    default: ps("-unk ext-");
    }
    return pc;
}

void pc(int c) { putc(c, stdout); }
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
    char tib[128];;
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
    exec();
}

int main()
{
    printf("MinForth v0.0.1 - Chris Curl\n");
    if (sizeof(&here) > CELL_SZ) {
        printf("ERROR: CELL cannot support a pointer!");
        exit(1);
    }
    I(500,549,550,4000);
    doLoad(0);
    while (!isBye) { loop(); }
}

#endif
