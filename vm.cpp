#include "shared.h"

byte sp, rsp, lsp, locSP, locBase, isError;
CELL BASE, stk[STK_SZ+1], rstk[STK_SZ+1], locals[LOCALS_SZ];
byte code[CODE_SZ+1], vars[VARS_SZ+1];
LOOP_T lstk[LSTK_SZ + 1];

void vmReset() {
    sp = rsp = lsp = locSP = locBase = 0;
    BASE = 10;
    HERE = LAST = 0;
    VHERE = VHERE2 = &vars[0];
    HERE = 2;
    for (int i = 0; i < CODE_SZ; i++) { code[i] = 0; }
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

void doType(byte *a, int l) {
    if (l < 0) { l = 0; while (a[l]) { ++l; } }
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

void run(WORD start) {
    byte *pc = CA(start);
    CELL t1, t2;
    rsp = lsp = locSP = isError = 0;
    while ((pc) && (isError == 0)) {
        byte ir = *(pc++);
        switch (ir) {
        case 0: return;
        case 1: push(*(pc++));                                              break; // BLIT
        case 2: push(GET_WORD(pc)); pc += 2;                                break; // WLIT
        case 4: push(GET_LONG(pc)); pc += 4;                                break; // LIT
        case ' ': /* NOP */                                                 break; // NOP
        case '!': SET_LONG(AOS, NOS); DROP2;                                break; // STORE
        case '"': /* UNUSED */                                              break;
        case '#': push(TOS);                                                break; // DUP
        case '$': t1 = TOS; TOS = NOS; NOS = t1;                            break; // SWAP
        case '%': push(NOS);                                                break; // OVER
        case '&': t1 = NOS; t2 = TOS; NOS = t1 / t2; TOS = t1 % t2;         break; // /MOD
        case '\'': /* UNUSED */                                             break;
        case '(': if (pop() == 0) { while (*pc != ')') { ++pc; } }          break; // Simple IF (NO ELSE)
        case ')': /* Simple THEN */                                         break; // Simple THEN
        case '*': t1 = pop(); TOS *= t1;                                    break; // MULT
        case '+': t1 = pop(); TOS += t1;                                    break; // ADD
        case ',': printChar((char)pop());                                   break; // EMIT
        case '-': t1 = pop(); TOS -= t1;                                    break; // SUB
        case '.': printBase(pop(), BASE);                                   break; // DOT
        case '/': t1 = pop(); TOS /= t1;                                    break; // DIV
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': push(ir-'0');
            while (betw(*pc,'0','9')) { TOS = (TOS*10) + *(pc++) - '0'; }
            break;                                                                 // NUMBER
        case ':': rpush(pc - code + 2); pc = CA(GET_WORD(pc));              break; // CALL
        case ';': pc = CA(rpop());                                          break; // RETURN
        case '>': NOS = (NOS > TOS) ? 1 : 0; pop();                         break; // >
        case '=': NOS = (NOS == TOS) ? 1 : 0; pop();                        break; // =
        case '<': NOS = (NOS < TOS) ? 1 : 0; pop();                         break; // <
        case '?': push(charAvailable());                                    break; // ?KEY
        case '@': TOS = GET_LONG((byte*)TOS);                               break; // FETCH
        case 'A': /* UNUSED */                                              break;
        case 'B': /* UNUSED */                                              break;
        case 'C': *AOS = (byte)NOS; DROP2;                                  break; // CSTORE
        case 'D': --TOS;                                                    break; // 1-
        case 'E': /* UNUSED */                                              break;
        case 'F': /* UNUSED */                                              break;
        case 'G': pc = CA((WORD)pop());                                     break; // EXECUTE
        case 'H': /* UNUSED */                                              break;
        case 'I': push(LOS.f);                                              break; // I
        case 'J': pc = CA(GET_WORD(pc));                                    break; // BRANCH
        case 'K': push(getChar());                                          break; // KEY
        case 'L': NOS = (NOS << TOS); pop();                                break; // LSHIFT
        case 'M': NOS %= TOS; pop();                                        break; // MOD
        case 'N': TOS = (TOS) ? 0 : 1;                                      break; // NOT (0=)
        case 'O': /* UNUSED */                                              break;
        case 'P': ++TOS;                                                    break; // 1+
        case 'Q': ir = *(pc++); if (ir == '<') { rpush(pop()); }                   // >R, R@, R>
                if (ir == '>') { push(rpop()); }
                if (ir == '@') { push(rstk[rsp]); }                         break;
        case 'R': NOS = (NOS >> TOS); pop();                                break; // RSHIFT
        case 'S': /* UNUSED */                                              break;
        case 'T': t1 = TOS; TOS = 0; while (BA(t1)[TOS]) { ++TOS; }         break; // ZLEN
        case 'U': /* UNUSED */                                              break;
        case 'V': /* UNUSED */                                              break;
        case 'W': SET_WORD(AOS, (WORD)NOS); DROP2;                          break; // W!
        case 'X': /* UNUSED */                                              break;
        case 'Y': vmReset();                                               return; // RESET
        case 'Z': doType((byte *)pop(),-1);                                 break; // ZTYPE
        case '[': lpush()->e = CA(GET_WORD(pc)); LOS.s = (pc += 2);                // FOR
            LOS.f = (TOS < NOS) ? TOS : NOS;
            LOS.t = (TOS > NOS) ? TOS : NOS;   DROP2;                       break;
        case '\\': pop();                                                   break; // DROP
        case ']': ++LOS.f; if (LOS.f <= LOS.t) { pc = LOS.s; }                     // NEXT
                else { lpop(); }                                            break;
        case '^': /* UNUSED */                                              break;
        case '_': TOS = -TOS;                                               break; // NEGATE
        case '`': /* UNUSED */                                              break;
        case 'a': t1 = pop(); TOS &= t1;                                    break; // AND
        case 'b': /* UNUSED */                                              break; // SPACE
        case 'c': TOS = *AOS;                                               break; // C@
        case 'd': t1 = *(pc++) - '0'; --locals[locBase + t1];               break; // decTemp
        case 'e': /* UNUSED */                                              break;
        case 'f': /* UNUSED */                                              break;
        case 'g': /* UNUSED */                                              break;
        case 'h': /* UNUSED */                                              break;
        case 'i': t1 = *(pc++) - '0'; ++locals[locBase + t1];               break; // incTemp
        case 'j': if (pop() == 0) { pc = CA(GET_WORD(pc)); }                       // IF (0BRANCH)
                else { pc += 2; }                                           break;
        case 'k': if (lsp) { --lsp; }                                       break; // UNLOOP
        case 'l': if (lsp) { pc = LOS.e; --lsp;  }                          break; // LEAVE
        case 'm': LOS.f += pop();                                           break; // +I
        case 'n': /* UNUSED */                                              break;
        case 'o': t1 = pop(); TOS |= t1;                                    break; // OR
        case 'p': locBase += 10;                                            break; // +tmp
        case 'q': locBase -= 10;                                            break; // -tmp
        case 'r': t1 = *(pc++) - '0'; push(locals[locBase + t1]);           break; // readTemp
        case 's': t1 = *(pc++) - '0'; locals[locBase + t1] = pop();         break; // setTemp
        case 't': printString((char *)pop());                               break; // QTYPE
        case 'u': if (pop() == 0) { pc = LOS.s; } else { lpop(); }          break; // UNTIL
        case 'v': if (pop()) { pc = LOS.s; } else { lpop(); }               break; // WHILE
        case 'w': TOS = GET_WORD(AOS);                                      break; // w@
        case 'x': t1 = pop(); TOS ^= t1;                                    break; // XOR
        case 'y': /* UNUSED */                                              break;
        case 'z': pc = doExt(*pc, pc+1);                                    break; // EXT
        case '{': lpush()->e = CA(GET_WORD(pc)); pc += 2; LOS.s = pc;       break; // BEGIN
        case '}': pc = LOS.s;                                               break; // AGAIN
        case '~': TOS = ~TOS;                                               break; // COM
        default: printStringF("-unk ir: %d (%c)-", ir, ir);                return;
        }
    }
}
