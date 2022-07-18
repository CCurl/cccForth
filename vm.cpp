#include "shared.h"

byte sp, rsp, lsp, lb, isError, sb, rb, fsp, *y;
CELL BASE, stks[STK_SZ], locals[LOCALS_SZ];
CELL lstk[LSTK_SZ+1];
float fstk[FLT_SZ];

ST_T st;

void vmReset() {
    lsp = lb = 0, fsp = 0;
    sb = 2, rb = (STK_SZ-2);
    sp = sb - 1, rsp = rb + 1;
    st.LAST = 0;
    st.oHERE = st.HERE = 2;
    st.oVHERE = st.VHERE = 0;
    for (int i = 0; i < CODE_SZ; i++) { st.code[i] = 0; }
    for (int i = 0; i < VARS_SZ; i++) { st.vars[i] = 0; }
    for (int i = 0; i < 10; i++) { tempWords[i] = 0; }
    systemWords();
}

void push(CELL v) { stks[++sp] = v; }
CELL pop() { return stks[sp--]; }

inline void fpush(float v) { fstk[++fsp] = v; }
inline float fpop() { return fstk[fsp--]; }

inline void rpush(CELL v) { stks[--rsp] = v; }
inline CELL rpop() { return stks[rsp++]; }

#ifdef NEEDS_ALIGN
WORD GET_WORD(byte* l) { return *l | (*(l + 1) << 8); }
long GET_LONG(byte* l) { return GET_WORD(l) | GET_WORD(l + 2) << 16; }
void SET_WORD(byte* l, WORD v) { *l = (v & 0xff); *(l + 1) = (byte)(v >> 8); }
void SET_LONG(byte* l, long v) { SET_WORD(l, v & 0xFFFF); SET_WORD(l + 2, (WORD)(v >> 16)); }
#else
WORD GET_WORD(byte* l) { return *(WORD *)l; }
long GET_LONG(byte* l) { return *(long *)l; }
void SET_WORD(byte* l, WORD v) { *(WORD *)l = v; }
void SET_LONG(byte* l, long v) { *(long *)l = v; }
#endif // NEEDS_ALIGN

void printBase(CELL num, CELL base) {
    UCELL n = (UCELL) num, isNeg = 0;
    if ((base == 10) && (num < 0)) { isNeg = 1; n = -num; }
    char* cp = (char *)&st.vars[VARS_SZ];
    *(cp--) = 0;
    do {
        int x = (n % base) + '0';
        n = n / base;
        *(cp--) = ('9' < x) ? (x+7) : x;
    } while (n);
    if (isNeg) { printChar('-'); }
    printString(cp+1);
}

void doDotS() {
    if (sp<sb) { sp=sb-1; }
    printString("(");
    for (int d = sb; d <= sp; d++) {
        if (sb < d) { printChar(' '); }
        printBase(stks[d], BASE);
    }
    printString(")");
}

byte *doType(byte *a, int l, int delim) {
    if (l < 0) { l = 0; while (a[l]!=delim) { ++l; } }
    byte* e = a+l;
    while (a < e) {
        byte c = *(a++);
        if (c == '%') {
            c = *(a++);
            if (c == 'b') { printBase(pop(), 2); }
            else if (c == 'c') { printChar((char)pop()); }
            else if (c == 'd') { printBase(pop(), 10); }
            else if (c == 'f') { printStringF("%g",fpop()); }
            else if (c == 'n') { printString("\r\n"); }
            else if (c == 'q') { printChar('"'); }
            else if (c == 'x') { printBase(pop(), 16); }
            else printChar((char)c);
        }
        else { printChar((char)c); }
    }
    if (delim) { ++e; }
    return e;
}

byte* doFile(CELL ir, byte* pc) {
    ir = *(pc++);
    if (ir == 'O') { fOpen(); }
    else if (ir == 'D') { fDelete(); }
    else if (ir == 'L') { fList(); }
    else if (ir == 's') { fSave(); }
    else if (ir == 'l') { fLoad(); pc = 0; }
    else if (TOS == 0) { printString("-nofp-"); return pc; }
    else if (ir == 'R') { fRead(); }
    else if (ir == 'W') { fWrite(); }
    else if (ir == 'C') { fClose(); }
    return pc;
}

void runX(WORD start) {
    byte* pc = CA(start);
    CELL t1, t2;
    lsp = isError = 0;
    if (sp < sb) { sp = sb - 1; }
    if (rsp > rb) { rsp = rb + 1; }
    if (fsp < 0) { fsp = 0; }
    if (9 < fsp) { fsp = 9; }
    while (pc) {
        byte ir = *(pc++);
        switch (ir) {
        case 0: return;
        case 1: push(*(pc++));                                               break; // BLIT
        case 2: push(GET_WORD(pc)); pc += 2;                                 break; // WLIT
        case 4: push(GET_LONG(pc)); pc += 4;                                 break; // LIT
        case ' ': /* NOP */                                                  break; // NOP
        case '!': SET_LONG(AOS, NOS); DROP2;                                 break; // STORE
        case '"': pc = doType(pc, -1, '"');                                  break; // ."
        case '#': push(TOS);                                                 break; // DUP
        case '$': t1 = TOS; TOS = NOS; NOS = t1;                             break; // SWAP
        case '%': push(NOS);                                                 break; // OVER
        case '&': t1 = NOS; t2 = TOS; NOS = t1 / t2; TOS = t1 % t2;          break; // /MOD
        case '\'': push(*(pc++));                                            break; // CHAR
        case '(': if (pop() == 0) { while (*pc != ')') { ++pc; } }           break; // Simple IF (NO ELSE)
        case ')': /* Simple THEN */                                          break; // Simple THEN
        case '*': t1 = pop(); TOS *= t1;                                     break; // MULT
        case '+': t1 = pop(); TOS += t1;                                     break; // ADD
        case ',': printChar((char)pop());                                    break; // EMIT
        case '-': t1 = pop(); TOS -= t1;                                     break; // SUB
        case '.': printBase(pop(), BASE);                                    break; // DOT
        case '/': t1 = pop(); TOS /= t1;                                     break; // DIV
        case '0': case '1': case '2': case '3': case '4': case '5':                 // NUMBER
        case '6': case '7': case '8': case '9': push(ir-'0');
            while (BTW(*pc,'0','9')) { TOS = (TOS*10) + *(pc++) - '0'; }     break;
        case ':': if (*(pc+2) != ';') { rpush(pc-st.code+2); }                      // CALL (w/tail-call optimization)
            pc = CA(GET_WORD(pc));                                           break;
        case ';': if (rsp>rb) { pc=0; rsp=rb+1; } else { pc=CA(rpop()); }    break; // RETURN
        case '>': NOS = (NOS > TOS) ? 1 : 0; DROP1;                          break; // >
        case '=': NOS = (NOS == TOS) ? 1 : 0; DROP1;                         break; // =
        case '<': NOS = (NOS < TOS) ? 1 : 0; DROP1;                          break; // <
        case '?': if (pop()==0) { pc=CA(GET_WORD(pc)); } else { pc+=2; }     break; // 0BRANCH
        case '@': TOS = GET_LONG((byte*)TOS);                                break; // FETCH
        case 'C': y=(byte*)TOS; TOS=0; while (*(y++)) { ++TOS; };            break; // STRLEN (a--c)
        case 'D': --TOS;                                                     break; // 1-
        case 'F': ir = *(pc++); if (ir=='.') { printStringF("%g",fpop()); }         // FLOAT ops
                else if (ir=='#') { fpush(FTOS); }
                else if (ir=='$') { float x=FTOS; FTOS=FNOS; FNOS=x; }
                else if (ir=='%') { fpush(FNOS); }
                else if (ir=='\\') { FDROP; }
                else if (ir=='i') { fpush((float)pop()); }
                else if(ir=='o') { push((CELL)fpop()); }
                else if(ir=='+') { FNOS+=FTOS; FDROP; }
                else if(ir=='-') { FNOS-=FTOS; FDROP; }
                else if(ir=='*') { FNOS*=FTOS; FDROP; }
                else if(ir=='/') { FNOS/=FTOS; FDROP; }
                else if(ir=='<') { push((FNOS<FTOS)?1:0); FDROP; FDROP; }
                else if(ir=='>') { push((FNOS>FTOS)?1:0); FDROP; FDROP; }    break;
        case 'G': pc = CA((WORD)pop());                                      break; // EXECUTE (GOTO)
        case 'I': push(L0);                                                  break; // I
        case 'J': pc = CA(GET_WORD(pc));                                     break; // BRANCH
        case 'K': ir = *(pc++); if (ir=='@') { push(getChar()); }                   // KEY?, KEY
                else if(ir=='?') { push(charAvailable()); }                  break;
        case 'M': L0 += pop();                                               break; // +I
        case 'P': ++TOS;                                                     break; // 1+
        case 'Q': ir = *(pc++); if (ir == '<') { rpush(pop()); }                    // >R, R@, R>
                if (ir == '>') { push(rpop()); }
                if (ir == '@') { push(stks[rsp]); }                          break;
        case 'S': ir = *(pc++); if (ir == 'L') { NOS = (NOS << TOS); }              // LSHIFT, RSHIFT
                else if (ir == 'R') { NOS = (NOS >> TOS); } DROP1;           break;
        case 'T': t1=pop(); y=(byte*)pop(); while (t1--) printChar(*(y++));  break; // TYPE (a c--)
        case 'Y': vmReset();                                                return; // RESET
        case 'Z': doType((byte *)pop(),-1, 0);                               break; // ZTYPE
        case '[': lsp += 3; L2 = (CELL)pc;                                          // FOR (f t--)
            L1 = (TOS > NOS) ? TOS : NOS;
            L0 = (TOS < NOS) ? TOS : NOS; DROP2;                             break;
        case '\\': DROP1;                                                    break; // DROP
        case ']': ++L0; if (L0<L1) { pc=(byte*)L2; } else { lsp-=3; }        break; // NEXT
        case '^': ir = *(pc++); if (ir == 'W') { lsp -= 1; }                        // UNLOOP
                else if (ir == 'F') { lsp =- 3; }                            break;
        case '_': TOS = -TOS;                                                break; // NEGATE
        case '`': /* UNUSED */                                               break;
        case 'b': ir = *(pc++); if (ir == '~') { TOS = ~TOS; }                      // BINARY ops
                else if (ir == '%') { NOS %= TOS; DROP1; }
                else if (ir == '&') { NOS &= TOS; DROP1; }
                else if (ir == '^') { NOS ^= TOS; DROP1; }
                else if (ir == '|') { NOS |= TOS; DROP1; }
                else { --pc; printChar(32); } break;
        case 'c': ir = *(pc++); if (ir=='@') { TOS = *(byte*)(TOS); }
                else if(ir=='!') { *AOS = (byte)NOS; DROP2; }                break; // c@, c!
        case 'd': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { --locals[lb+t1]; }      break; // decLocal
        case 'i': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { ++locals[lb+t1]; }      break; // incLocal
        case 'f': pc = doFile(ir, pc);                                       break; // FILE ops
        case 'l': if ((lb+10)<LOCALS_SZ) { lb+=10; }                         break; // +TMPS
        case 'm': if (lb>9) { lb-=10; }                                      break; // -TMPS
        case 'r': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { push(locals[lb+t1]); }  break; // readLocal
        case 's': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { locals[lb+t1]=pop(); }  break; // setLocal
        case 't': printString((char *)pop());                                break; // QTYPE
        case 'v': t1=GET_LONG(pc); pc+=4; push((CELL)&st.vars[t1]);          break; // VAR-ADDR
        case 'w': ir = *(pc++); if (ir == '@') { TOS = GET_WORD(AOS); }
                else if (ir == '!') { SET_WORD(AOS, (WORD)NOS); DROP2; }     break; // w@, w!
        case 'x': ir=*(pc++); if (ir=='S') { doDotS(); }                            // .S
                else if (ir=='}') { pc=(byte*)L0; }                                 // AGAIN
                else if (ir=='A') { st.oVHERE+=pop(); st.VHERE=st.oVHERE; }         // ALLOT
                else if (ir=='T') { push(timer()); }                                // TIMER
                else if (ir=='Y') { y=(byte*)pop(); system((char*)y); }             // SYSTEM
                else if (ir=='D') { doWords(); }                                    // WORDS
                else if (ir=='W') { doSleep(); }                                    // MS
                else if (ir=='Q') { isBye=1; return; }                       break; // BYE
        case 'z': pc = doExt(*pc, pc+1);                                     break; // EXT
        case '{': ++lsp; L0=(CELL)pc;                                        break; // BEGIN
        case '}': if (pop()) { pc=(byte*)L0; } else { lsp--; }               break; // WHILE
        case '~': TOS = (TOS) ? 0 : 1;                                       break; // NOT (0=)
        default: printStringF("-unk ir: %d (%c)-", ir, ir);                 return;
        }
    }
}

byte ir, * pc;
CELL t1, t2;

void fXXX() { pc=0; }
void f001() { push(*(pc++)); }
void f002() { push(GET_WORD(pc)); pc+=2; }
void f004() { push(GET_LONG(pc)); pc+=4; }
void f032() { /* NOP */}
void f033() { SET_LONG((byte*)TOS, NOS); DROP2; }
void f034() { pc = doType(pc, -1, '"'); }
void f035() { push(TOS); }
void f036() { t1 = TOS; TOS = NOS; NOS = t1; }
void f037() { push(NOS); }
void f038() { t1 = NOS; t2 = TOS; NOS = t1 / t2; TOS = t1 % t2; }
void f039() { push(*(pc++)); }
void f040() { if (pop() == 0) { while (*pc != ')') ++pc; } }
void f041() { /* Simple THEN */ }
void f042() { t1 = pop(); TOS *= t1; }
void f043() { t1 = pop(); TOS += t1; }
void f044() { printChar((char)pop()); }
void f045() { t1 = pop(); TOS -= t1; }
void f046() { printBase(pop(), BASE); }
void f047() { t1 = pop(); TOS /= t1; }
void fNUM() { push(ir-'0'); while (BTW(*pc,'0','9')) { TOS = (TOS*10) + *(pc++) - '0'; } }
void f058() { if (*(pc+2) != ';') { rpush(pc-st.code+2); } pc = CA(GET_WORD(pc)); }
void f059() { if (rsp > rb) { pc = 0; rsp = rb + 1; } else { pc = CA(rpop()); } }
void f060() { NOS = (NOS < TOS) ? 1 : 0; DROP1; }
void f061() { NOS = (NOS == TOS) ? 1 : 0; DROP1; }
void f062() { NOS = (NOS > TOS) ? 1 : 0; DROP1; }
void f063() { if (pop() == 0) { pc = CA(GET_WORD(pc)); } else { pc += 2; } }
void f064() { TOS = GET_LONG((byte*)TOS); }
void f065() { /* A */ }
void f066() { /* B */ }
void f067() { y = (byte*)TOS; TOS=0; while (*(y++)) { ++TOS; }; }
void f068() { --TOS; }
void f069() { /* E */ }
void f070() { ir = *(pc++); if (ir == '.') { printStringF("%g", fpop()); }
    else if (ir == '#') { fpush(FTOS); }
    else if (ir == '$') { float x = FTOS; FTOS = FNOS; FNOS = x; }
    else if (ir == '%') { fpush(FNOS); }
    else if (ir == '\\') { FDROP; }
    else if (ir == 'i') { fpush((float)pop()); }
    else if (ir == 'o') { push((CELL)fpop()); }
    else if (ir == '+') { FNOS += FTOS; FDROP; }
    else if (ir == '-') { FNOS -= FTOS; FDROP; }
    else if (ir == '*') { FNOS *= FTOS; FDROP; }
    else if (ir == '/') { FNOS /= FTOS; FDROP; }
    else if (ir == '<') { push((FNOS < FTOS) ? 1 : 0); FDROP; FDROP; }
    else if (ir == '>') { push((FNOS > FTOS) ? 1 : 0); FDROP; FDROP; } }
void f071() { pc = CA((WORD)pop()); }
void f072() { /* H */ }
void f073() { push(L0); }
void f074() { pc = CA(GET_WORD(pc)); }
void f075() { ir = *(pc++); if (ir == '@') { push(getChar()); } 
    else if (ir == '?') { push(charAvailable()); } }
void f076() { /* L */ }
void f077() { L0 += pop(); }
void f078() { /* N */ }
void f079() { /* O */ }
void f080() { ++TOS; }
void f081() { ir = *(pc++); if (ir == '<') { rpush(pop()); }
    if (ir == '>') { push(rpop()); }
    if (ir == '@') { push(stks[rsp]); } }
void f082() { /* R */ }
void f083() { ir = *(pc++); if (ir == 'L') { NOS = (NOS << TOS); }
    else if (ir == 'R') { NOS = (NOS >> TOS); } DROP1; }
void f084() { t1 = pop(); y = (byte*)pop(); while (t1--) printChar(*(y++)); }
void f085() { /* U */ }
void f086() { /* V */ }
void f087() { /* W */ }
void f088() { /* X */ }
void f089() { vmReset(); }
void f090() { doType((byte*)pop(), -1, 0); }
void f091() { lsp += 3; L2 = (CELL)pc;
    L1 = (TOS > NOS) ? TOS : NOS;
    L0 = (TOS < NOS) ? TOS : NOS; DROP2; }
void f092() { DROP1; }
void f093() { ++L0; if (L0 < L1) { pc = (byte*)L2; } else { lsp -= 3; } }
void f094() { ir=*(pc++); if (ir=='W') { lsp-=1; } else if (ir=='F') { lsp=-3; } }
void f095() { TOS = -TOS; }
void f096() { /* ` */ }
void f097() { /* a */ }
void f098() { ir = *(pc++); if (ir == '~') { TOS = ~TOS; }
    else if (ir == '%') { NOS %= TOS; DROP1; }
    else if (ir == '&') { NOS &= TOS; DROP1; }
    else if (ir == '^') { NOS ^= TOS; DROP1; }
    else if (ir == '|') { NOS |= TOS; DROP1; }
    else { --pc; printChar(32); } }
void f099() { ir=*(pc++); if (ir=='@') { TOS=*AOS; } else if (ir=='!') { *AOS=(byte)NOS; DROP2; } }
void f100() { t1=*(pc++)-'0'; if (BTW(t1,0,9)) { --locals[lb+t1]; } }
void f101() { /* e */ }
void f102() { pc = doFile(ir, pc); }
void f103() { /* g */ }
void f104() { /* h */ }
void f105() { t1=*(pc++)-'0'; if (BTW(t1,0,9)) { ++locals[lb+t1]; } }
void f106() { /* j */ }
void f107() { /* k */ }
void f108() { if ((lb+10)<LOCALS_SZ) { lb+=10; } }
void f109() { if (lb>9) { lb-=10; } }
void f110() { /* n */ }
void f111() { /* o */ }
void f112() { /* p */ }
void f113() { /* q */ }
void f114() { t1=*(pc++)-'0'; if (BTW(t1,0,9)) { push(locals[lb+t1]); } }
void f115() { t1=*(pc++)-'0'; if (BTW(t1,0,9)) { locals[lb+t1]=pop(); } }
void f116() { printString((char*)pop()); }
void f117() { /* u */ }
void f118() { t1=GET_LONG(pc); pc+=4; push((CELL)&st.vars[t1]); }
void f119() { ir = *(pc++); if (ir=='@') { TOS=GET_WORD(AOS); }
    else if (ir=='!') { SET_WORD(AOS, (WORD)NOS); DROP2; } }
void f120() { ir=*(pc++); if (ir=='S') { doDotS(); }                    // .S
    else if (ir=='}') { pc=(byte*)L0; }                                 // AGAIN
    else if (ir=='A') { st.oVHERE+=pop(); st.VHERE=st.oVHERE; }         // ALLOT
    else if (ir=='T') { push(timer()); }                                // TIMER
    else if (ir=='Y') { y=(byte*)pop(); system((char*)y); }             // SYSTEM
    else if (ir=='D') { doWords(); }                                    // WORDS
    else if (ir=='W') { doSleep(); }                                    // MS
    else if (ir=='Q') { isBye=1; return; } }
void f121() { /* y */ }
void f122() { pc=doExt(*pc, pc+1); }
void f123() { ++lsp; L0=(CELL)pc; }
void f124() { /* | */ }
void f125() { if (pop()) { pc=(byte*)L0; } else { lsp--; } }
void f126() { TOS=(TOS)?0:1; }
void f127() {}

void (*q[128])() = { 
    fXXX,f001,f002,fXXX,f004,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,
    fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,fXXX,
    f032,f033,f034,f035,f036,f037,f038,f039,f040,f041,f042,f043,f044,f045,f046,f047,
    fNUM,fNUM,fNUM,fNUM,fNUM,fNUM,fNUM,fNUM,fNUM,fNUM,f058,f059,f060,f061,f062,f063,
    f064,f065,f066,f067,f068,f069,f070,f071,f072,f073,f074,f075,f076,f077,f078,f079,
    f080,f081,f082,f083,f084,f085,f086,f087,f088,f089,f090,f091,f092,f093,f094,f095,
    f096,f097,f098,f099,f100,f101,f102,f103,f104,f105,f106,f107,f108,f109,f110,f111,
    f112,f113,f114,f115,f116,f117,f118,f119,f120,f121,f122,f123,f124,f125,f126,f127
    };

void run(WORD start) {
    pc = CA(start);
    lsp = isError = 0;
    if (sp < sb) { sp = sb - 1; }
    if (rsp > rb) { rsp = rb + 1; }
    if (fsp < 0) { fsp = 0; }
    if (9 < fsp) { fsp = 9; }
    while (pc) {
        ir = *(pc++);
        q[ir]();
    }
}
