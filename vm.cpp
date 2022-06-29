#include "shared.h"

byte sp, rsp, lsp, locSP, lb, isError, sb, rb, fsp;
CELL BASE, stks[STK_SZ], locals[LOCALS_SZ];
byte code[CODE_SZ+1], vars[VARS_SZ+1], *y;
CELL lstk[LSTK_SZ+1];
float fstk[FLT_SZ];

void vmReset() {
    lsp = locSP = lb = 0, fsp = 0;
    sb = 2, rb = (STK_SZ-2);
    sp = sb - 1, rsp = rb + 1;
    BASE = 10;
    HERE = LAST = 0;
    VHERE = &vars[0];
    HERE = 2;
    for (int i = 0; i < CODE_SZ; i++) { code[i] = 0; }
    for (int i = 0; i < VARS_SZ; i++) { vars[i] = 0; }
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
    char* cp = (char *)&code[CODE_SZ];
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
    else if (TOS == 0) { printString("-nofp-"); return pc; }
    else if (ir == 'R') { fRead(); }
    else if (ir == 'W') { fWrite(); }
    else if (ir == 'C') { fClose(); }
    return pc;
}

void run(WORD start) {
    byte* pc = CA(start);
    CELL t1, t2;
    lsp = locSP = isError = 0;
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
        case ':': if (*(pc+2) != ';') { rpush(pc - code + 2); }                     // CALL (w/tail-call optimization)
            pc = CA(GET_WORD(pc));                                           break;
        case ';': if (rsp>rb) { pc=0; rsp=rb+1; } else { pc=CA(rpop()); }    break; // RETURN
        case '>': NOS = (NOS > TOS) ? 1 : 0; DROP1;                          break; // >
        case '=': NOS = (NOS == TOS) ? 1 : 0; DROP1;                         break; // =
        case '<': NOS = (NOS < TOS) ? 1 : 0; DROP1;                          break; // <
        case '?': if (pop()==0) { pc=CA(GET_WORD(pc)); } else { pc+=2; }     break; // 0BRANCH
        case '@': TOS = GET_LONG((byte*)TOS);                                break; // FETCH
        case 'C': y=(byte*)TOS; t1=0; while (*(y++)) { ++t1; } push(t1);     break; // COUNT (a--a c)
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
        case '[': lsp += 3; L2 = (CELL)pc;                                          // FOR
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
        case 'c': ir = *(pc++); if (ir=='@') { TOS = *AOS; }
                else if(ir=='!') { *AOS = (byte)NOS; DROP2; }                break; // c@, c!
        case 'd': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { --locals[lb+t1]; }      break; // decLocal
        case 'i': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { ++locals[lb+t1]; }      break; // incLocal
        case 'f': pc = doFile(ir, pc);                                       break; // BINARY ops
        case 'l': ir=*(pc++); if (ir=='+') { lb+=((lb+10)<LOCALS_SZ)?10:0; }        // locals
                else if (ir=='-') { lb-=(lb<10)?0:10; }                      break;
        case 'r': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { push(locals[lb+t1]); }  break; // readLocal
        case 's': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { locals[lb+t1]=pop(); }  break; // setLocal
        case 't': printString((char *)pop());                                break; // QTYPE
        case 'w': ir = *(pc++); if (ir == '@') { TOS = GET_WORD(AOS); }
                else if (ir == '!') { SET_WORD(AOS, (WORD)NOS); DROP2; }     break; // w@, w!
        case 'x': ir=*(pc++); if (ir=='S') { doDotS(); }
                else if (ir=='A') { oVHERE+=pop(); VHERE=oVHERE; }
                else if (ir=='D') { doWords(); }
                else if (ir=='Y') { y=(byte*)pop(); system((char*)y); }
                else if (ir=='Q') { isBye=1; return; }                       break;
        case 'z': pc = doExt(*pc, pc+1);                                     break; // EXT
        case '{': ++lsp; L0=(CELL)pc; if (!TOS) { while (*pc!='}') ++pc; }   break; // BEGIN
        case '}': if (TOS) { pc=(byte*)L0; } else { DROP1; lsp--; }          break; // WHILE
        case '~': TOS = (TOS) ? 0 : 1;                                       break; // NOT (0=)
        default: printStringF("-unk ir: %d (%c)-", ir, ir);                 return;
        }
    }
}
