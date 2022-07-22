// cccForth.cpp : An extremely memory conscious Forth interpreter

#include "Shared.h"

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
    , { ">R", "R<" }
    , { "R>", "R>" }
    , { "R@", "R@" }
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
    , { "COUNT", "#C" }
    , { "STRLEN", "C" }
    , { "TYPE", "T" }
    // Logical / flow control
    , { ".IF", "(" }
    , { ".THEN", ")" }
    , { "FOR", "[" }
    , { "I", "I" }
    , { "+I", "M" }
    , { "NEXT", "]" }
    , { "UNLOOP-F", "^F" }
    , { "BEGIN", "{" }
    , { "WHILE", "}" }
    , { "UNTIL", "~}" }
    , { "AGAIN", "x}" }
    , { "UNLOOP-W", "^W" }
    , { "=", "=" }
    , { "<", "<" }
    , { ">", ">" }
    , { "<=", ">~" }
    , { ">=", "<~" }
    , { "<>", "=~" }
    , { "!=", "=~" }
    , { "0=", "~" }
    , { "EXIT", ";" }
    // String
    , { "STR-LEN", "Sl" }
    , { "STR-END", "Se" }
    , { "STR-CAT", "Sa" }
    , { "STR-CPY", "Sy" }
    , { "STR-EQ", "S=" }
    , { "STR-EQI", "Si" }
    , { "STR-TRUNC", "St" }
    , { "STR-RTRIM", "Sr" }
    // Binary/bitwise
    , { "AND", "b&" }
    , { "OR", "b|" }
    , { "XOR", "b^" }
    , { "COM", "b~" }
    , { "<<", "bL" }
    , { ">>", "bR" }
    // Float
    , { "Fi", "Fi" }  // In
    , { "Fo", "Fo" }  // Out
    , { "F+", "F+" }  // Add
    , { "F-", "F-" }  // Sub
    , { "F*", "F*" }  // Mult
    , { "F/", "F/" }  // Div
    , { "F<", "F<" }  // LT
    , { "F>", "F>" }  // GT
    , { "F.", "F." }  // PRINT
    , { "FDUP", "F#" }
    , { "FOVER", "F%" }
    , { "FSWAP", "F$" }
    , { "FDROP", "F\\" }
    // System
    , { "BL", "32" }
    , { "BYE", "xQ" }
    , { "CELL", "4" }
    , { "CELLS", "4*" }
    , { "EXECUTE", "G" }
    , { "MAX", "%%<($)\\" }
    , { "MIN", "%%>($)\\" }
    , { "MS", "xW" }
    , { "NOP", " " }
    , { "NOT", "~" }
    , { "RAND", "zR" }
    , { "RESET", "Y" }
    , { "SPACE", "32," }
    , { "SYSTEM", "xY" }
    , { "TIMER", "xT" }
    , { "+TMPS", "l" }
    , { "-TMPS", "m" }
    , { "WORDS", "xD" }
#ifdef __FILES__
    // Extension: FILE operations
    , { "FOPEN", "fO" }
    , { "FGETC", "fR" }
    , { "FPUTC", "fW" }
    , { "FCLOSE", "fC" }
    , { "FDELETE", "fD" }
    , { "FLIST", "fL" }
    , { "FSAVE", "fs" }
    , { "FLOAD", "fl" }
#endif
#ifdef __PIN__
    // Extension: PIN operations ... for dev boards
    , { "pin-in","zPI" }          // open input
    , { "pin-out","zPO" }         // open output
    , { "pin-up","zPU" }          // open input-pullup
    , { "pin!","zPWD" }           // Pin write: digital
    , { "pin@","zPRD" }           // Pin read: digital
    , { "pina!","zPWA" }          // Pin write: analog
    , { "pina@","zPRA" }          // Pin read: analog
#endif
#ifdef __EDITOR__
    // Extension: A simple block editor
    , { "EDIT","zE" }         // |EDIT|zE|(n--)|Edit block n|
#endif
#ifdef __GAMEPAD__
    // Extension: GAMEPAD operations
    , { "gp-button","xGB" }
#endif
    , {0,0}
};

char word[32], *in;
CELL STATE, tempWords[10];
byte isBye=0;

void CComma(CELL v) { st.code[st.HERE++] = (byte)v; }
void Comma(CELL v) { SET_LONG(&st.code[st.HERE], v); st.HERE += CELL_SZ; }
void WComma(WORD v) { SET_WORD(&st.code[st.HERE], v); st.HERE += 2; }

char lower(char c) { return BTW(c, 'A', 'Z') ? (c + 32) : c; }

int strLen(const char* str) {
    int l = 0;;
    while (*(str++)) { ++l; }
    return l;
}

int strEq(const char *x, const char *y) {
    while (*x && *y && (*x == *y)) { ++x; ++y; }
    return (*x || *y) ? 0 : 1;
}

int strEqI(const char *x, const char *y) {
    while (*x && *y) {
        if (lower(*x) != lower(*y)) { return 0; }
        ++x; ++y;
    }
    return (*x || *y) ? 0 : 1;
}

char *strCpy(char *d, const char *s) {
    char *x = d;
    while (*s) { *(x++) = *(s++); }
    *x = 0;
    return d;
}

char *strCat(char *d, const char *s) {
    char *x = d+strLen(d);
    strCpy(x, s);
    return d;
}

char *rTrim(char *d) {
    char *x = d+strLen(d);
    while ((d<=x) && (*x<=' ')) { *(x--) = 0; }
    return d;
}

void printStringF(const char *fmt, ...) {
    char *buf = (char*)&st.vars[VARS_SZ-100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 100, fmt, args);
    va_end(args);
    printString(buf);
}

void doExec() {
    if (STATE) {
        st.oHERE = st.HERE;
        st.oVHERE = st.VHERE;
    }
    else {
        CComma(0);
        run((WORD)st.oHERE);
        st.HERE = st.oHERE;
        st.VHERE = st.oVHERE;
    }
}

int isTempWord(const char *nm) {
    return ((nm[0] == 'T') && BTW(nm[1], '0', '9') && (nm[2] == 0));
}

void doCreate(const char *name, byte f) {
    doExec();
    if (isTempWord(name)) {
        tempWords[name[1]-'0'] = st.HERE;
        STATE = 1;
        return;
    }
    DICT_T *dp = DP_AT(st.HERE);
    dp->prev = (byte)(st.HERE - st.LAST);
    dp->flags = f;
    strCpy(dp->name, name);
    st.LAST = st.HERE;
    st.HERE += strLen(name) + 3;
    STATE = 1;
}

int doFind(const char *name) {
    // Temporary word?
    if (isTempWord(name) && (tempWords[name[1]-'0'])) {
        push(tempWords[name[1]-'0']);
        push(0);
        return 1;
    }

    // Regular lookup
    CELL def = (WORD)st.LAST;
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

int doSee(const char* wd) {
    if (!doFind(wd)) { return 1; }
    CELL def = (WORD)st.LAST;
    CELL prevDef = st.oHERE;
    int found = 0;
    DROP2;
    while (def && (!found)) {
        DICT_T* dp = DP_AT(def);
        if (strEq(dp->name, wd)) {
            found = 1;
            break;
        }
        prevDef = def;
        def -= dp->prev;
    }

    if (def < prevDef) {
        printStringF("%s: ", wd);
        for (int i = def; i < prevDef; i++) {
            byte c = st.code[i];
            if (BTW(c, 32, 126)) { printChar(c); }
            else { printStringF("(%d)",c); }
        }
    }
    return 1;
}

void doWords() {
    CELL l = (WORD)st.LAST, n = 0;
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

int doNumber(int t) {
    CELL num = pop();
    if (t=='v') {
        CComma('v');
        Comma(num);
    } else if ((BTW(num,32,126))) {
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

char *sprintF(char *dst, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(dst, 256, fmt, args);
    va_end(args);
    return dst;
}

int doNumber2() {
    if (TOS < 0) { return doNumber(0); }
    char buf[16];
    sprintF(buf, "%ld", pop());
    if (st.HERE && BTW(st.code[st.HERE-1],'0','9')) { CComma(' '); }
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
    // Words cccForth can map directly into its VML (Virtual Machine Language)
    const char *vml = isRegOp(wd);

    for (int i = 0; prims[i].op && (!vml); i++) {
        if (strEqI(prims[i].name, wd)) { vml = prims[i].op; }
    }

    if (!vml) { return 0; } // Not found

    if (BTW(vml[0],'0','9') && BTW(st.code[st.HERE-1],'0','9')) { CComma(' '); }
    for (int j = 0; vml[j]; j++) { CComma(vml[j]); }
    return 1;
}

int doQuote() {
    in++;
    push((CELL)st.VHERE);
    doNumber('v');
    while (*in && (*in != '"')) { st.vars[st.VHERE++] = *(in++); }
    st.vars[st.VHERE++] = 0;
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
        push(st.HERE);
        WComma(0);
        return 1;
    }

    if (strEqI(wd, "ELSE")) {
        CELL tgt = pop();
        CComma('J');
        push(st.HERE);
        WComma(0);
        SET_WORD(CA(tgt), (WORD)st.HERE);
        return 1;
    }

    if (strEqI(wd, "THEN")) {
        CELL tgt = pop();
        SET_WORD(CA(tgt), (WORD)st.HERE);
        return 1;
    }

    if (strEqI(wd, "VARIABLE")) {
        if (getWord(wd)) {
            push((CELL)st.oVHERE);
            st.oVHERE += CELL_SZ;
            st.VHERE = st.oVHERE;
            doCreate(wd, 0);
            doNumber('v');
            CComma(';');
            doExec();
            STATE = 0;
            return 1;
        }
        else { return 0; }
    }

    if (strEqI(wd, "CONSTANT")) {
        if (getWord(wd)) {
            doCreate(wd, 0);
            doNumber(0);
            CComma(';');
            doExec();
            STATE = 0;
            return 1;
        }
        else { return 0; }
    }

    if (strEqI(wd, "LOOKUP")) {
        doExec();
        if (getWord(wd) == 0) { return 0; }
        push(doFind(wd));
        return 1;
    }

    if (strEqI(wd, "FORGET")) {
        // Forget the last word
        st.oHERE = st.HERE = st.LAST;
        st.LAST -= st.code[st.LAST];
        return 1;
    }

    if (strEqI(wd, "SEE")) {
        doExec();
        if (getWord(wd) == 0) { return 0; }
        return doSee(wd);
    }

    STATE = 0;
    st.HERE = st.oHERE;
    st.VHERE = st.oVHERE;
    printStringF("[%s]??", wd);
    return 0;
}

bool isASM(const char* ln) {
    if ((ln[0]=='s') && (ln[1]==':') && (ln[2]==' ')) {
        run((byte*)ln-st.code+3);
        return 1;
    }
    return 0;
}

void doParse(const char *line) {
    in = (char*)line;
    if (isASM(line)) { return; }
    int len = getWord(word);
    while (0 < len) {
        if (st.HERE < st.oHERE) { st.HERE = st.oHERE; }
        if (st.VHERE < st.oVHERE) { st.VHERE = st.oVHERE; }
        if (doParseWord(word) == 0) { return; }
        len = getWord(word);
    }
    doExec();
}

void doOK() {
    if (STATE) { printString(" ... "); return; }
    printString("\r\nOK ");
    doDotS();
    printString("> ");
}

void systemWords() {
    BASE = 10;
    char *cp = (char*)(&st.vars[VARS_SZ-32]);
    sprintF(cp, ": cb %lu ;", (UCELL)st.code);     doParse(cp);
    sprintF(cp, ": vb %lu ;", (UCELL)st.vars);     doParse(cp);
    sprintF(cp, ": csz %d ;", CODE_SZ);            doParse(cp);
    sprintF(cp, ": vsz %d ;", VARS_SZ);            doParse(cp);
    sprintF(cp, ": ha %lu ;", (UCELL)&st.oHERE);   doParse(cp);
    sprintF(cp, ": la %lu ;", (UCELL)&st.LAST);    doParse(cp);
    sprintF(cp, ": va %lu ;", (UCELL)&st.oVHERE);  doParse(cp);
    sprintF(cp, ": base %lu ;", (UCELL)&BASE);     doParse(cp);
    sprintF(cp, ": >in %lu ;", (UCELL)&in);        doParse(cp);
}

#if __BOARD__ == PC
#include "pc.inc"
#endif
