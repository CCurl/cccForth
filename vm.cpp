#include "shared.h"

char sp, rsp, lsp, locSP, locBase, isError;
CELL BASE, stk[STK_SZ+1], rstk[STK_SZ+1], locals[LOCALS_SZ];
byte user[USER_SZ+1], vars[VARS_SZ+1];
LOOP_T lstk[LSTK_SZ + 1];

void vmReset() {
    sp = rsp = lsp = locSP = locBase = 0;
    BASE = 10;
    HERE = LAST = 0;
    VHERE = VHERE2 = &vars[0];
    for (int i = 0; i < USER_SZ; i++) { user[i] = 0; }
    for (int i = 0; i < VARS_SZ; i++) { vars[i] = 0; }
    for (int i = 0; i < 10; i++) { tempWords[i] = 0; }
    systemWords();
}

void push(CELL v) { if (sp < STK_SZ) { stk[++sp] = v; } }
CELL pop() { return sp ? stk[sp--] : 0; }

void rpush(CELL v) { if (rsp < STK_SZ) { rstk[++rsp] = v; } }
CELL rpop() { return rsp ? rstk[rsp--] : 0; }

LOOP_T *lpush() { return (lsp < LSTK_SZ) ? &lstk[++lsp] : 0; }
LOOP_T *lpop() { return (lsp) ? &lstk[--lsp] : 0; }

WORD GET_WORD(byte *l) { return *l | (*(l+1) << 8); }
long GET_LONG(byte *l) { return GET_WORD(l) | GET_WORD(l + 2) << 16; }

void SET_WORD(byte *l, WORD v) { *l = (v & 0xff); *(l+1) = (byte)(v >> 8); }
void SET_LONG(byte *l, long v) { SET_WORD(l, v & 0xFFFF); SET_WORD(l + 2, (WORD)(v >> 16)); }

void printBase(CELL num, CELL base) {
    UCELL n = (UCELL) num, isNeg = 0;
    if ((base == 10) && (num < 0)) { isNeg = 1; n = -num; }
    char* cp = (char *)&user[USER_SZ];
    *(cp--) = 0;
    do {
        int x = (n % base) + '0';
        n = n / base;
        *(cp--) = ('9' < x) ? (x+7) : x;
    } while (n);
    if (isNeg) { printChar('-'); }
    printString(cp+1);
}

void doType(byte *a, int l) {
    byte* e = a+l;
    while (a < e) {
        byte c = *(a++);
        if (c == '%') {
            c = *(a++);
            if (c == 'b') { printBase(pop(), 2); }
            else if (c == 'c') { printChar((char)pop()); }
            else if (c == 'd') { printBase(pop(), 10); }
            else if (c == 'n') { printString("\r\n"); }
            else if (c == 'q') { printChar('"'); }
            else if (c == 'x') { printBase(pop(), 16); }
            else printChar((char)c);
        }
        else { printChar((char)c); }
    }
}

void doZType(byte *a) {
    byte *x = a;
    while (*(x++)) {}
    doType(a, x-a);
}

void run(WORD start) {
    WORD pc = start;
    CELL t1, t2;
    rsp = lsp = locSP = isError = 0;
    while (isError == 0) {
        byte ir = U(pc++);
        switch (ir) {
        case 0: return;
        case '#': push(TOS);                                                break; // DUP
        case '%': push(NOS);                                                break; // OVER
        case '$': t1 = TOS; TOS = NOS; NOS = t1;                            break; // SWAP
        case '\\': pop();                                                   break; // DROP
        case '+': t1 = pop(); TOS += t1;                                    break; // ADD
        case '-': t1 = pop(); TOS -= t1;                                    break; // SUB
        case '*': t1 = pop(); TOS *= t1;                                    break; // MULT
        case '/': t1 = pop(); TOS /= t1;                                    break; // DIV
        case '.': printBase(pop(), BASE);                                   break; // DOT
        case ':': rpush(pc + 2); pc = GET_WORD(UA(pc));                     break; // CALL
        case ';': pc = (WORD)rpop();                                        break; // RETURN
        case '1': push(U(pc++));                                            break; // BLIT
        case '2': push(GET_WORD(UA(pc))); pc += 2;                          break; // WLIT
        case '4': push(GET_LONG(UA(pc))); pc += 4;                          break; // LIT
        case '@': TOS = GET_LONG((byte*)TOS);                               break; // FETCH
        case '!': SET_LONG(AOS, NOS); DROP2;                                break; // STORE
        case 'C': *AOS = (byte)NOS; DROP2;                                  break; // CSTORE
        case 'G': pc = (WORD)pop();                                         break; // EXECUTE
        case 'I': push(LOS.f);                                              break; // I
        case 'J': pc = GET_WORD(UA(pc));                                    break; // BRANCH
        case 'L': NOS = (NOS << TOS); pop();                                break; // LSHIFT
        case 'N': TOS = (TOS) ? 0 : 1;                                      break; // NOT (0=)
        case 'R': NOS = (NOS >> TOS); pop();                                break; // RSHIFT
        case 'W': SET_WORD(AOS, (WORD)NOS); DROP2;                          break; // W!
        case 'Y': vmReset();                                               return; // RESET
        case 'Z': doZType((byte *)pop());                                   break; // ZTYPE
        case '~': TOS = ~TOS;                                               break; // COM
        case '=': NOS = (NOS == TOS) ? 1 : 0; pop();                        break; // =
        case '>': NOS = (NOS > TOS) ? 1 : 0; pop();                         break; // >
        case '<': NOS = (NOS < TOS) ? 1 : 0; pop();                         break; // <
        case ',': printChar((char)pop());                                   break; // EMIT
        case '&': t1 = NOS; t2 = TOS; NOS = t1 / t2; TOS = t1 % t2;         break; // /MOD
        case '^': if (lsp) { pc = LOS.e; }                                  break; // BREAK
        case '[': lpush()->e = GET_WORD(UA(pc)); pc += 2;                          // FOR
            LOS.s = pc;
            LOS.f = TOS < NOS ? TOS : NOS;
            LOS.t = TOS > NOS ? TOS : NOS; DROP2;                           break;
        case ']': ++LOS.f; if (LOS.f <= LOS.t) { pc = LOS.s; }                     // NEXT
                else { lpop(); }                                            break;
        case 'a': t1 = pop(); TOS &= t1;                                    break; // AND
        case 'b': printChar(' ');                                           break; // SPACE
        case 'c': TOS = *AOS;                                               break; // C@
        case 'd': TOS--;                                                    break; // 1-
        case 'i': TOS++;                                                    break; // 1+
        case 'j': if (pop() == 0) { pc = GET_WORD(UA(pc)); }                       // IF (0BRANCH)
                else { pc += 2; }                                           break;
        case 'k': push(' ');                                                break; // BL
        case 'm': LOS.f += pop();                                           break; // +I
        case 'n': printString("\r\n");                                      break; // CR
        case 'o': t1 = pop(); TOS |= t1;                                    break; // OR
        case 'p': locBase += 10;                                            break; // +tmp
        case 'q': locBase -= 10;                                            break; // -tmp
        case 'r': t1 = U(pc++) - '0'; push(locals[locBase + t1]);           break; // readTemp
        case 's': t1 = U(pc++) - '0'; locals[locBase + t1] = pop();         break; // setTemp
        case 't': push(timer());                                            break; // TIMER
        case 'v': if (pop()) { pc = LOS.s; }                                break; // WHILE
        case 'u': if (pop() == 0) { pc = LOS.s; }                           break; // UNTIL
        case 'w': TOS = GET_WORD(AOS);                                      break; // w@
        case 'x': t1 = pop(); TOS ^= t1;                                    break; // XOR
        case '{': lpush()->e = GET_WORD(UA(pc)); pc += 2; LOS.s = pc;       break; // BEGIN
        case '}': pc = LOS.s;                                               break; // AGAIN
        default: pc = doExt(ir, pc);                                        break;
        }
    }
}
