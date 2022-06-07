#ifndef __FORTH_SHARED__
#define __FORTH_SHARED__

#define PC           1
#define TEENSY4      2
#define XIAO         3
#define ESP32_DEV    4
#define ESP8266      5
#define APPLE_MAC    6
#define LINUX        7

#define __BOARD__    TEENSY4

#ifdef _WIN32
  #undef __BOARD__
  #define __BOARD__ PC
  #define __WINDOWS__
  #include <conio.h>
  #define NUM_FUNCS    (500)
  #define CODE_SZ      (16*1024)
  #define VARS_SZ      (48*1024)
  #define STK_SZ        16
  #define LSTK_SZ        8
  #define LOCALS_SZ    160
  #define VMSZ         (64*1024)
  // #define __EDITOR__
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#if __BOARD__ == TEENSY4
  #define CODE_SZ      (48*1024)
  #define VARS_SZ      (96*1024)
  #define STK_SZ        16
  #define LSTK_SZ        8
  #define LOCALS_SZ    160
  #define __FILES__
  #define __EDITOR__
#elif __BOARD__ == XIAO
  #define CODE_SZ      (16*1024)
  #define VARS_SZ      ( 8*1024)
  #define STK_SZ        16
  #define LSTK_SZ        8
  #define LOCALS_SZ    160
  // #define __GAMEPAD__
#endif

#define CELL_SZ      4

#define TOS           stk[sp]
#define NOS           stk[sp-1]
#define AOS           (byte*)TOS
#define LOS           lstk[lsp]
#define DROP2         pop(); pop()
#define CA(l)         (code+l)
#define DP_AT(l)      ((DICT_T *)(&code[l]))
#define betw(x, a, b) ((a<=x)&&(x<=b))
#define BTW(x, a, b) ((a<=x)&&(x<=b))
#define BA(a)         ((byte *)a)

typedef unsigned char byte;
typedef unsigned short WORD;
typedef int CELL;
typedef unsigned long UCELL;
typedef unsigned short USHORT;

typedef union {
    float f[VMSZ / 4];
    int i[VMSZ / 4];
    char b[VMSZ];
} FIB_T;

typedef struct {
    byte flags;
    USHORT xt;
    char name[12];
} DICT_T;

extern byte sp, isError;
extern CELL base, state, last, here, cb, xt;

extern void ps(const char*);
extern void I(int, int, int);
extern void systemWords();
extern void push(CELL);
extern CELL pop();
extern char *stringF(char *, const char*, ...);
extern void pc(int);
extern void pn(CELL, CELL);
extern int strLen(const char *);
extern void run(WORD);
extern void doOK();
extern int doExt(CELL, int);
extern void doEditor();
extern int doFind(const char *);
extern void doParse(const char *);
extern int charAvailable();
extern int getChar();
extern WORD getXT(WORD, DICT_T *);
extern CELL timer();
extern void delay();

// FILEs
extern void fileInit();
extern void fileOpen();
extern void fileRead();
extern void fileWrite();
extern void fileClose();
extern void readBlock();
extern void writeBlock();

#endif
