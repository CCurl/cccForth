#include <Arduino.h>
#include "shared.h"

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

byte *doPin(byte *pc) {
    CELL pin = pop();
    byte ir = *(pc++);
    switch (ir) {
    case 'I': pinMode(pin, INPUT);          break;
    case 'O': pinMode(pin, OUTPUT);         break;
    case 'U': pinMode(pin, INPUT_PULLUP);   break;
    case 'R': ir = *(pc++);
        if (ir == 'A') { push(analogRead(pin));  }
        if (ir == 'D') { push(digitalRead(pin)); }
        break;
    case 'W': ir = *(pc++);
        if (ir == 'A') { analogWrite(pin,  (int)pop()); }
        if (ir == 'D') { digitalWrite(pin, (int)pop()); }
        break;
    default:
        isError = 1;
        printString("-notPin-");
    }
    return pc;
}

#include "file-board.h"

byte *doExt(CELL ir, byte *pc) {
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

// ********************************************
// * HERE is where you load your default code *
// ********************************************
void loadCode() {
    doParse(": T0 dup 32 >= .if dup '~' <= .if emit exit .then .\" (%d)\" ;");
    doParse(": .code cb here over + 1- for i c@ T0 next ;");
}

// NB: tweak this depending on what your terminal window sends for [Backspace]
// E.G. - PuTTY sends a 127 for Backspace
int isBackSpace(char c) { 
  // printStringF("(%d)",c);
  return (c == 127) ? 1 : 0; 
}

void handleInput(char c) {
    static char *tib = NULL, *e = NULL;

    if (tib == NULL) { tib = e = (char *)CA(HERE+32); }
    if (c == 13) {
        *e = 0;
        printString(" ");
        doParse(rtrim(tib));
        tib = NULL;
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
//    fileInit();
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
