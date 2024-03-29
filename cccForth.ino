#include <Arduino.h>
#include "cccForth.h"

#define AR(x) analogRead(x)
#define DR(x) digitalRead(x)
#define mySerial Serial
#define __SERIAL__

CELL doTimer() { return millis(); }
void doSleep() { delay(pop()); }
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
byte *doGamePad(byte ir, byte *pc) { printString("-noGamepad-"); return pc; }
#endif

int isOTA = 0;

void printString(const char* str) { 
    if (isOTA) { } //printWifi(str); }
    else { printSerial(str); }
}

void printChar(char ch) { 
    char b[2] = { ch, 0 };
    printString(b);
}

#include "file-board.h"

byte *doExt(CELL ir, byte *pc) {
    CELL pin;
    switch (ir) {
    case 'G': pc = doGamePad(ir, pc);           break;  // zG<x>
    case 'N': push(micros());                   break;  // zN (--n)
    case 'P': pin = pop(); ir = *(pc++);                // Pin operations
        switch (ir) {
        case 'I': pinMode(pin, INPUT);                           break;  // zPI (p--)
        case 'O': pinMode(pin, OUTPUT);                          break;  // zPO (p--)
        case 'U': pinMode(pin, INPUT_PULLUP);                    break;  // zPU (p--)
        case 'R': ir = *(pc++);
            if (ir == 'A') { push(analogRead(pin));  }                   // zPRA (p--n)
            if (ir == 'D') { push(digitalRead(pin)); }           break;  // zPRD (p--n)
        case 'W': ir = *(pc++);
            if (ir == 'A') { analogWrite(pin,  (int)pop()); }            // zPWA (n p--)
            if (ir == 'D') { digitalWrite(pin, (int)pop()); }    break;  // zPWD (n p--)
        default:
            isError = 1;
            printString("-notPin-");
        }                                       break;
    default:
        isError = 1;
        printString("-notExt-");
    }
    return pc;
}

// ********************************************
// * HERE is where you load your default code *
// ********************************************
void loadCode() {
    // doParse(": T0 dup 32 >= .if dup '~' <= .if emit exit .then .\" (%d)\" ;");
    // doParse(": .code cb here over + 1- for i c@ T0 next ;");
    // doParse("cr words");
}

// NB: tweak this depending on what your terminal window sends for [Backspace]
// E.G. - PuTTY sends a 127 for Backspace
int isBackSpace(char c) { 
  // printStringF("(%d)",c);
  return (c == 127) ? 1 : 0; 
}

char tib[128];

void handleInput(char c) {
    static char *e = NULL;

    if (e == NULL) { e = tib; }
    if (c == 13) {
        *e = 0;
        printString(" ");
        doParse(rTrim(tib));
        e = NULL;
        doOK();
        return;
    }

    if (isBackSpace(c) && (tib < e)) {
        e--;
        if (!isOTA) {
          char b[] = {8, 32, 8, 0};
          printString(b);
        }
        return;
    }
    if (c == 9) { c = 32; }
    if (BTW(c, 32, 126)) {
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
#endif
    vmReset();
    loadCode();
    printString("cccForth v0.0.1 - Chris Curl\r\n");
    doOK();
    gamePadInit();
//    wifiStart();
    fileInit();
}

void doAutoRun() {
    if (doFind("AUTORUN")) {
      pop();
      run((WORD) pop());
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
    doAutoRun();
}
