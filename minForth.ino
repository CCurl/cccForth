#include <Arduino.h>
#include "shared.h"
#include "file-board.h"

#define AR(x) analogRead(x)
#define DR(x) digitalRead(x)
#define mySerial Serial
#define __SERIAL__

CELL timer() { return millis(); }
CELL getSeed() { return millis(); }

#ifdef __SERIAL__
    int charAvailable() { return mySerial.available(); }
    int getChar() { 
        while (!charAvailable()) {}
        return mySerial.read();
    }
    void printSerial(const char* str) { mySerial.print(str); }
#else
    int charAvailable() { return 0; }
    int getChar() { return 0; }
    void printSerial(const char* str) { }
#endif

#ifdef __GAMEPAD__
int buttonVal[GP_BUTTONS];
#if __BOARD__ == TEENSY4
  #include "gamePad-Teensy.h"
#else
  #include "gamePad.h"
#endif
void gamePadInit() {
    for (int i = 0; i < GP_BUTTONS; i++) {
        buttonVal[i] = 0;
    }
    gpInit();
}
#else
void gamePadInit() {}
WORD doGamePad(byte ir, WORD pc) { printString("-noGamepad-"); return pc; }
#endif

int isOTA = 0;

void printString(const char* str) { 
    if (isOTA) { } //printWifi(str); }
    else { printSerial(str); }
}

void printChar(const char ch) { 
    char b[2] = { ch, 0 };
    printString(b);
}

WORD doPin(WORD pc) {
    CELL pin = pop();
    byte ir = U(pc++);
    switch (ir) {
    case 'I': pinMode(pin, INPUT);          break;
    case 'O': pinMode(pin, OUTPUT);         break;
    case 'U': pinMode(pin, INPUT_PULLUP);   break;
    case 'R': ir = U(pc++);
        if (ir == 'A') { push(analogRead(pin));  }
        if (ir == 'D') { push(digitalRead(pin)); }
        break;
    case 'W': ir = U(pc++);
        if (ir == 'A') { analogWrite(pin,  (int)pop()); }
        if (ir == 'D') { digitalWrite(pin, (int)pop()); }
        break;
    default:
        isError = 1;
        printString("-notPin-");
    }
    return pc;
}

WORD doExt(CELL ir, WORD pc) {
    switch (ir) {
    case 'G': pc = doGamePad(ir, pc);       break;
    case 'N': push(micros());               break;
    case 'P': pc = doPin(pc);               break;
    case 'T': push(millis());               break;
    case 'W': delay(pop());                 break;
    default:
        isError = 1;
        printString("-notExt-");
    }
    return pc;
}

// void loadCode(const char* src) {
//     WORD here = HERE;
//     WORD here1 = HERE;
//     while (*src) {
//         *(here1++) = *(src++);
//     }
//     *here1 = 0;
//     run(here);
// }

// void input_push(FILE *fp) { }
// FILE *input_pop() { return NULL; }

// ********************************************
// * HERE is where you load your default code *
// ********************************************

// #define SOURCE_STARTUP \
//     X(1000, ":CODE CR xIAU xIH 1-[rI c@ #,';=(rI 1+ c@':=(CR))];") \
//     X(1001, ":CR 13,10,;:U xIHxIAU-;") \
//     X(1002, ":REGS 0 xIR 1-[rI xIC* xIAR+@ #s1(CR\"r\" rI 26&$ 26&$ 'A+,'A+,'A+,\": \"r1.)];") \
//     X(1003, ":SI xIUxIFxIR\"%nThis system has %d registers, %d functions, and %d bytes user memory.\";") \
//     X(1004, ":XDOT s2 0s1{r2&$i1} 1 r1[#9>(7+)'0+,]32,;") \
//     X(1005, ":BDOT 2 XDOT;:HDOT 16 XDOT;:DOT 10 XDOT;") \
//     X(1006, ":NN 10&$..32,;:NNN 100&$.NN;") \
//     X(9999, "SI")
// 
//#if __BOARD__ == ESP8266
// #define X(num, val) const char str ## num[] = val;
//#else
//#define X(num, val) const PROGMEM char str ## num[] = val;
//#endif
// SOURCE_STARTUP

// #undef X
// #define X(num, val) str ## num,
// const char *bootStrap[] = {
//     SOURCE_STARTUP
//     NULL
// };

// void loadBaseSystem() {
//     for (int i = 0; bootStrap[i] != NULL; i++) {
//         loadCode(bootStrap[i]);
//     }
// #ifdef __FILES__
//     loadCode("xFL");
// #endif
// }

// NB: tweak this depending on what your terminal window sends for [Backspace]
// E.G. - PuTTY sends a 127 for Backspace
int isBackSpace(char c) { 
  // printStringF("(%d)",c);
  return (c == 127) ? 1 : 0; 
}

void handleInput(char c) {
    static char *s = NULL, *e = NULL;
    if (s == NULL) { s = e = (char *)(VHERE + 6); }
    if (c == 13) {
        *e = 0;
        printString(" ");
        doParse(s);
        s = NULL;
        doOK();
        return;
    }

    if (isBackSpace(c) && (s < e)) {
        e--;
        if (!isOTA) {
          char b[] = {8, 32, 8, 0};
          printString(b);
        }
        return;
    }
    if (c == 9) { c = 32; }
    if (betw(c, 32, 126)) {
        *(e++) = c;
        if (!isOTA) { printChar(c); }
    }
}

void setup() {
#ifdef __SERIAL__
    mySerial.begin(19200);
    while (!mySerial) {}
    delay(500);
    // while (mySerial.available()) { char c = mySerial.read(); }
    doOK();
#endif
    vmReset(); 
    gamePadInit();
//    wifiStart();
//    fileInit();
}

void do_autoRun() {
    int p = doFind("AUTORUN");
    if (0 <= p) {
      DICT_T* dp = DP_AT(p);
      run(getXT((WORD)p, dp));
    }
}

void loop() {
    static int iLed = 0;
    static long nextBlink = 0;
    static int ledState = LOW;
    long curTm = millis();

    if (iLed == 0) {
        doOK();
        iLed = LED_BUILTIN;
        pinMode(iLed, OUTPUT);
    }
    
    if (nextBlink < curTm) {
        ledState = (ledState == LOW) ? HIGH : LOW;
        digitalWrite(iLed, ledState);
        nextBlink = curTm + 1000;
    }

    while (charAvailable()) { 
        isOTA = 0;
        handleInput(getChar()); 
    }
    //while (wifiCharAvailable()) { 
    //    isOTA = 1;
    //    handleInput(wifiGetChar()); 
    //}
    do_autoRun();
}
