# cccForth a minimal and extensible Forth system

cccForth is intended to be a starting point for a programming environment that can grow to fit the user's needs.

The main goals for this minimal Forth are as follows:

- To be easy to modify and add/extend the primitives.
- To be able to run on any system that has a C compiler.
- To have an implementation that is minimal and "intuitively obvious upon casual inspection".
- To be be frugal with its usage of memory.
- To be able to fit on systems with small amounts of memory.
- To be deployable to as many different kinds of development boards as possible, via the Arduino IDE.
- To have a VML (virtual machine language) that is as human-reabable as possible.

To these ends, I have wandered off the beaten path in the following ways:

- This is NOT an ANSI-standard Forth.
- This is a byte-coded implementation.
- Many primitives (core words) are built into the compiler, and are not included in the dictionary.
- These primitves ARE NOT case sensitive (DUP = dup = Dup).
- User-defined words ARE case sensitive.
- The dictionary is intersparsed with the CODE; it is not separated.
- A dictionary entry looks like this: (offset,flags,name,null terminator,implementation).
- It has only 3 additional bytes of overhead (in addition to the name).
- To save space, code addresses are 2 bytes, so code space is limited to 16 bits (64kb).
- All CODE addresses are offsets into the CODE space, not absolute addresses.
- HERE and LAST are also offsets into the CODE space, not absolute addresses.
- The VARIABLE space is separated from the CODE space, and can be larger than 64kb.
- VHERE is a 32-bit offset to the first available byte in he VARIABLE space.
- There are 10 temporary words (T0..T9) that can be re-defined without any dictionary overhead.
- There are 10 temporary variables (r0..r9) that can be allocated/destroyed.

## Temporary words:
- A temporary word is named T0 .. T9. (e.g. - ": T4 ... ;" will define word T4, but T4 is not added to the dictionary)
- I think of temporary words as named transient words that don't incur any dictionary overhead.
- Their purpose is to support code reuse and factoring without the overhead of a dictionary entriy.
- You cannot create an IMMEDIATE temporary word.
- When defined, they refer to the current value of HERE.
- All previous references to T4 still refer to the previous definition. New references to T4 refer to the new definition.
- Here is a simple example:
```
: T1 dup $20 < .if drop '.' .then ;
: dumpC for i c@ T1 emit next ;
: .code cb dup here + 1- for i c@ T1 emit next ;
```

## Temporary variables:
- Temporary variables are allocated in sets of 10.
- They are completely under the control of the programmer.
- They are not built into the call sequence, so they can be accessed across words.
- They are not set or retrieved using @ and !.
- There are 4 special operations for temp variables: read, set, increment and decrement
- The words to manage temps are: +tmps, r0..r9, s0..s9, i0..i9, d0..d9 and -tmps
  - +tmps: allocate 10 new temps
  - rX: push the value of temp #x onto the stack (read)
  - sX: pop the new value for temp #X off the stack (set)
  - iX: increment temp #X
  - dX: decrement temp #x
  - -tmps: destroy the most recently allocated temps
- Here are some simple examples:
```
: betw +tmps s3 s2 s1 r2 r1 <= r1 r3 <= and -tmps ;
: .c (n--) s9 r9 #32 $7e betw if r9 emit else r9 ." (%d)" then ;
: dumpX (a n--) swap s8 1 for r8 c@ .c i8 next ;
```

## cccForth Primitives
```
NOTE: (1) These are NOT case-sensitive.
      (2) They do NOT show up in WORDS.

*** MATH ***
+        (a b--c)          Addition
-        (a b--c)          Subtraction
/        (a b--c)          Division
*        (a b--c)          Multiplication
<<       (a b--c)          c: a left-shifted b bits
>>       (a b--c)          c: a right-shifted b bits
ABS      (a--b)            Absolute value
MAX      (a b--c)          c: max(a,b)
MIN      (a b--c)          c: min(a,b)
/MOD     (a b--q r)        q: quotient(a,b), r: modulus(a,b)
MOD      (a b--c)          modulus(a,b)
NEGATE   (a--b)            b: -a

*** STACK ***
1+       (a--b)            Increment TOS
1-       (a--b)            Decrement TOS
2*       (a--b)            b: a*2
2/       (a--b)            b: a/2
DROP     (a b--a)          Drop TOS
DUP      (a--a a)          Duplicate TOS
2DROP    (a b--)           Drop NOS and TOS
2DUP     (a b--a b a b)    Duplicate NOS and TOS
NIP      (a b--b)          Drop NOS
OVER     (a b--a b a)      Copy NOS
SWAP     (a b--b a)        Swap TOS and NOS
TUCK     (a b--b a b)      Push TOS under NOS
>R       (n--)             Move n to return stack
R>       (--n)             Move n from return stack
R@       (--n)             Copy n from return stack
ROT      (a b c--b c a)    Rotate a to TOS
-ROT     (a b c--c a b)    Rotate c before a

*** BITWISE ***
AND      (a b--c)          Bitwise AND
OR       (a b--c)          Bitwise OR
XOR      (a b--c)          Bitwise XOR
COM      (a--b)            Bitwise COMPLEMENT

*** FLOAT ***
Fi       (n--)             Float In
Fo       (--n)             Float Out
F+       (a b--c)          Float Add
F-       (a b--c)          Float Sub
F*       (a b--c)          Float Mult
F/       (a b--c)          Float Div
F<       (a b--c)          Float LT
F>       (a b--c)          Float GT
F.       (--)              Float PRINT
FDUP     (a--a a)          Float DUP
FOVER    (a b--a b a)      Float OVER
FSWAP    (a b--b a)        Float SWAP
FDROP    (a--)             Float DROP

*** INPUT/OUTPUT ***
(.)      (n--)             Output n in the current BASE (no SPACE)
.        (n--)             Output n in the current BASE (trailing SPACE)
."       (?--?)            Output a (possibly formatted) string. See (1).
CR       (--)              Output a newline (#10,#13)
EMIT     (c--)             Output c as a character
COUNT    (a--a n)          n: length of string at a (a must be NULL-terminated)
TYPE     (a n--)           Output string at (standard Forth TYPE)
KEY      (--c)             c: Next keyboard char, wait if no press yet
KEY?     (--f)             f: FALSE if no keyboard press, else TRUE
QTYPE    (a--)             Quick string output, no formatting.
SPACE    (--)              Output a single SPACE
ZTYPE    (a--)             Output string at a. See (1).

(1) Notes on ." and ZTYPE:
- ." is NOT ansi-standard
- %d: output TOS as integer
- %x: output TOS as hex
- %b: output TOS as binary
- %c: output TOS as character
- %n: output a new-line (13,10)
- %q: output the quote (") character

example: : ascii $20 '~' for i i i i ." %n%d: (%c) %x %b" next ;

*** FILE ***
FOPEN    (a n--fh)         a: file name, fh: file handle, n: 0 => READ, else WRITE
FREAD    (fh--c n)         c: next char from file fh, n: number chars read (0 => EOF)
FWRITE   (c fh--)          c: char to write to file fh.
FCLOSE   (fh--)            fh: file handle to close.
FDELETE  (fn--)            fn: The name of the file to be deleted.
FLIST    (--)              Print the list of files created on the dev board.
FSAVE    (--)              Saves the system to file "/system.ccc"
FLOAD    (--)              Loads the last saved system file, if any.

*** LOGICAL ***
=        (a b--f)          Equality
<        (a b--f)          Less-than
>        (a b--f)          Greater-than
<=       (a b--f)          Less-than or equal-to
>=       (a b--f)          Greater-than or equal-to
<>       (a b--f)          Not equal-to
0=       (a b--f)          Logical NOT
NOT      (a--b)            Logical NOT

*** MEMORY ***
@        (a--n)            n: CELL at a
C@       (a--b)            b: BYTE at a
W@       (a--w)            w: WORD at a
!        (n a--)           Store CELL n at a
C!       (b a--)           Store BYTE b at a
W!       (w a--)           Store WORD w at a
+!       (n a--)           Add n to CELL at a
"        (--a)             a: 32-bit address of a string. See (2).

(2) Notes on " (--a):
- This does not generate a standard FORTH counted string.
- It is NULL-terminated, no count byte.

*** FLOW CONTROL ***
IF       (f--)             Standard IF
ELSE     (--)              Standard ELSE
THEN     (--)              Standard THEN
.IF      (f--)             Simple IF, no ELSE allowed (shorter)
.THEN    (--)              Simple THEN
FOR      (f t--)           Begin FOR/NEXT loop
I        (--n)             n: Current index
+I       (n--)             n: value to add to I
EXIT-F   (--)              Drop top 3 entries from loop stack (unwind FOR loop)
NEXT     (--)              Increment I, jump to start of loop if I < T
BEGIN    (f--f)            Start WHILE/UNTIL/AGAIN loop.
WHILE    (f--f?)           If f==0, jump to BEGIN, else DROP f and continue.
UNTIL    (f--f?)           If f<>0, jump to BEGIN, else DROP f and continue.
AGAIN    (--)              Jump to BEGIN. Use IF EXIT-W EXIT THEN to break out.
EXIT-W   (--)              Drop top 1 entry from loop stack (unwind WHILE loop)
EXIT     (--)              Exit the word immediately (don't forget to EXIT-F/W first if in a LOOP)

*** TEMPORARY VARIABLES ***
+TMPS    (--)              Allocate 10 temp variables, r0 .. r9
rX       (--n)             n: read value of temp var X (X:[0..9])
sX       (n--)             Set value of temp var X to n
iX       (--n)             Increment temp var X
dX       (--n)             Decrement temp var X
-TMPS    (--)              Destroy current temp variables

**** SYSTEM/OTHER ***
.S       (--)              Output the stack
BL       (--c)             c: 32
BYE      (--)              Exit cccForth (PC)
CONSTANT (--)              Define a constant
CELL     (--n)             n: The size of a CELL
EDIT     (n--)             Edit block n
EXECUTE  (a--)             Jump to CODE address a
LOAD     (n--)             Load block n from disk
NOP      (--)              Do nothing
RAND     (--n)             n: a RANDOM 32-bit number
RESET    (--)              Re-initialize cccForth
SYSTEM   (a--)             a: string to send to system() ... eg: " dir" system (PC)
TIMER    (--n)             n: clock()
WAIT     (n--)             n: MS to sleep
VARIABLE (--)              Define a variable
WORDS    (--)              Output the dictionary
```

## Built-in words
```
cb     (--a)   a: Start address for CODE area
vb     (--a)   a: Start address for VARS area
csz    (--n)   n: Size of CODE area
vsz    (--n)   n: Size of VARS area
ha     (--a)   a: Address of HERE
la     (--a)   a: Address of LAST
va     (--a)   a: Address of VHERE
base   (--a)   a: Address of BASE
```
## Extending cccForth
In the C code, TOS the "top-of-stack", and NOS is "next-on-stack". There is also push(x) and pop(), which manage the stack.

In the cccForth.cpp file, in the beginning, there is a section where prims[] is defined. This enumerates all the primitives that cccForth knows about. The first member of an entry is the Forth name, the second part is the VML code.

You will see there are sections that add additional primitives if a symbol is #defined, for example:
```
#ifdef __PIN__
    // Extension: PIN operations ... for dev boards
    , { "pin-input","zPI" }       // open input
    , { "pin-output","zPO" }      // open output
    , { "pin-pullup","zPU" }      // open input-pullup
    , { "analog-read","zPRA" }    // Pin read: analog
    , { "digital-read","zPRD" }   // Pin read: digital
    , { "analog-write","zPWA" }   // Pin write: analog
    , { "digital-write","zPWD" }  // Pin write: digital
#endif
```
You will notice that the VML code for these operations all begin with 'z'. The byte 'z' is the trigger to the VM that the command is implemented in doExt(ir, pc). Here is the definition of that function for development boards (cccForth.ino):
```
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
            if (ir == 'A') { push(analogRead(pin));  }                     // zPRA (p--n)
            if (ir == 'D') { push(digitalRead(pin)); }             break;  // zPRD (p--n)
        case 'W': ir = *(pc++);
            if (ir == 'A') { analogWrite(pin,  (int)pop()); }              // zPWA (n p--)
            if (ir == 'D') { digitalWrite(pin, (int)pop()); }      break;  // zPWD (n p--)
        default:
            isError = 1;
            printString("-notPin-");
        }                                       break;
    case 'W': delay(pop());                     break;  // zW (n--)
    default:
        isError = 1;
        printString("-notExt-");
    }
    return pc;
}
```
In this context, 'pc' points to the next byte in the command stream, so "byte x = *(pc++);" sets x to the next byte and points pc to the next byte after that. A VML command "zN" will execute the 'N' case in doExt(), which in this case, executes "push(micros())", which pushes the return value from the Arduino lilbrary function "micros()" onto the stack. 

So, to add a primitive, add the Forth word to the prims[] array (either behind a #define or not), make sure it starts with 'z', then add a handler for that new case in doExt() that does whatever you want.