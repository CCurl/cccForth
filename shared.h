#ifndef __FORTH_SHARED__
#define __FORTH_SHARED__

#define PC           1
#define TEENSY4      2
#define XIAO         3
#define ESP32_DEV    4
#define ESP8266      5

#define __BOARD__    TEENSY4

#ifdef _WIN32
  #undef __BOARD__
  #define __BOARD__ PC
  #define __WINDOWS__
  #define  _CRT_SECURE_NO_WARNINGS
  #include <Windows.h>
  #include <conio.h>
  #define USER_SZ      (64*1024)
  #define VARS_SZ      (128*1024)
  #define STK_SZ        16
  #define LSTK_SZ        8
  #define LOCALS_SZ    128
  #define __EDITOR__
#endif

#include <stdarg.h>
#include <stdio.h>

#if __BOARD__ != PC
  // For TEENSY4
  #define USER_SZ      (48*1024)
  #define VARS_SZ      (48*1024)
  #define STK_SZ        16
  #define LSTK_SZ        8
  #define LSTK_SZ      128
  // #define __EDITOR__
#endif

#define CELL_SZ      4

#define TOS   stk[sp]
#define NOS   stk[sp-1]
#define AOS  (byte*)stk[sp]
#define LOS   lstk[lsp]
#define DROP2 pop(); pop()
#define U(l)  user[l]
#define UA(l) &U(l)
#define DP_AT(l) ((DICT_T *)(&user[l]))
#define betw(x, a, b) ((a<=x)&&(x<=b))

typedef unsigned char byte;
typedef unsigned short WORD;
typedef long CELL;
typedef unsigned long UCELL;
typedef unsigned short USHORT;

typedef struct {
    byte prev;
    byte flags;
    char name[32];
} DICT_T;

typedef struct {
    WORD s, e;
    CELL f, t;
} LOOP_T;

extern char sp, isError;
extern CELL BASE, STATE, LAST, HERE, tempWords[10];
extern byte *VHERE, *VHERE2;
extern byte user[];
extern byte vars[];
extern CELL stk[];
extern CELL rstk[];

extern void vmReset();
extern void systemWords();
extern void push(CELL);
extern CELL pop();
extern void SET_WORD(byte *l, WORD v);
extern void SET_LONG(byte *l, long v);
extern void printString(const char*);
extern void printStringF(const char*, ...);
extern void printChar(char);
extern void printBase(CELL, CELL);
extern int strLen(const char *);
extern void run(WORD);
extern void doOK();
extern WORD doExt(CELL, WORD);
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
