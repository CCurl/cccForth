# minForth

The main goals for this minimal Forth are as follows:

- To be very frugal with its usage of memory.
- To be easy to modify and add/extend the primitives.
- To have an implementation that is minimal and "intuitively obvious upon casual inspection".
- To be able to run on any system that has a C compiler.
- To be deployable to as many different kinds of development boards as possible via the Arduino IDE.
- To have a VML (virtual machine language) that is as human-reabable as possible, but not at the expense of performance.

To these ends, I have wandered off the beaten path in the following ways:

- This is NOT an ANSI-standard Forth.
- This is a byte-coded implementation to save code space.
- Many primitives (core words) are built into the compiler, and are not included in the dictionary.
- The primitves ARE NOT case sensitive (DUP = dup = Dup).
- User-defined words ARE case sensitive.
- The dictionary is intersparsed with the CODE; it is not separated.
- A dictionary entry looks like this: (offset,flags,name,null terminator,implementation).
- It has only 3 additional bytes of overhead (in addition to the name).
- To save space, code addresses are 2 bytes, so code space is limited to 16 bits (64kb).
- All CODE addresses are offsets into the CODE space, not absolute addresses.
- HERE and LAST are also offsets into the CODE space, not absolute addresses.
- The VARIABLE space is separated from the CODE space, and can be larger than 64kb.
- VHERE is a 32-bit absolute address to the first available byte in VARIABLE.
- There are 10 temporary words (T0..T9) that can be re-defined without any dictionary overhead.
- There are 10 temporary variables (r0..r9) that can be allocated/destroyed.
- "WORDS" is built in, and cannot be called by another word at runtime (it's IMMEDIATE).

## Temporary words:
- A temporary word is named T0 .. T9. (e.g. - ": T4 ... ;" will define word T4, but T4 is not added to the dictionary)
- I think of temporary words as named transient words that don't incur any dictionary overhead.
- Their purpose is to support code reuse and factoring without the overhead of a dictionary entriy.
- You cannot create an IMMEDIATE temporary word.
- When defined, they refer to the current value of HERE.
- All previous references to T4 still refer to the previous definition. New references to T4 refer to the new definition.
- Here is a simple example:
```
: T1 dup $20 < if drop '.' then ;
: dumpC for i c@ T1 emit next ;
: .code cb dup here + 1- for i c@ T1 emit next ;
```

## Temporary variables:
- Temporary variables are allocated in sets of 10.
- They are completely under the control of the programmer.
- They are not built into the call sequence, so they can be accessed across words.
- They are not set or retrieved using @ and !.
- There are 2 special operations for temp variables: read and set (eg - r3 1+ s3).
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

## VM primitives (these are NOT case-sensitive, and do NOT show up in WORDS)
```
+        (a b--c)          Addition
-        (a b--c)          Subtraction
/        (a b--c)          Division
*        (a b--c)          Multiplication
/MOD     (a b--q r)        q: quotient(a,b), r: modulus(a,b)
MOD      (a b--c)          modulus(a,b)
SWAP     (a b--b a)        Swap top 2 stack entries (TOS and NOS)
DROP     (a b--a)          Drop TOS
OVER     (a b--a b a)      Copy NOS
DUP      (a--a a)          Duplicate TOS
NIP      (a b--b)          Drop NOS
TUCK     (a b--b a b)      Push TOS under NOS
2DUP     (a b--a b a b)    Duplicate NOS And TOS
2DROP    (a b--)           Drop NOS and TOS
EMIT     (c--)             Output c as a character
(.)      (n--)             Output n in the current BASE (no SPACE)
.        (n--)             Output n in the current BASE (trailing SPACE)
SPACE    (--)              Output a single SPACE
CR       (--)              Output a newline (#10,#13)
BL       (--c)             c: 32
CELL     (--n)             n: The size of a CELL
=        (a b--f)          Equality
<        (a b--f)          Less-than
>        (a b--f)          Greater-than
<=       (a b--f)          Less-than or equal-to
>=       (a b--f)          Greater-than or equal-to
<>       (a b--f)          Not equal-to
0=       (a b--f)          Logical NOT
NOT      (a--b)            Logical NOT
ABS      (a--b)            Absolute value
NEGATE   (a--b)            b: -a
<<       (a b--c)          c: a left-shifted b bits
>>       (a b--c)          c: a right-shifted b bits
ZLEN     (a--n)            n: length of string at a
@        (a--n)            n: CELL at a
C@       (a--b)            b: BYTE at a
W@       (a--w)            w: WORD at a
!        (n a--)           Store CELL n at a
C!       (b a--)           Store BYTE b at a
W!       (w a--)           Store WORD w at a
FOR      (f t--)           Begin FOR/NEXT loop
I        (--n)             n: Current index
+I       (n--)             n: value to add to I
NEXT     (--)              Increment I, jump to start of loop if I < T
BEGIN    (f--f)            Start WHILE loop: if f=0, skip to WHILE
WHILE    (f--f?)           If f==0, jump to BEGIN, else DROP f and continue
UNTIL    (f--f?)           If f<>0, jump to BEGIN, else DROP f and continue
UNLOOP   (--)              Drop top of loop stack (FOR or WHILE)
AND      (a b--c)          Bitwise AND
OR       (a b--c)          Bitwise OR
XOR      (a b--c)          Bitwise XOR
COM      (a--b)            Bitwise COMPLEMENT
1+       (a--b)            Increment TOS
1-       (a--b)            Decrement TOS
+!       (n a--)           Add n to CELL at a
EXECUTE  (a--)             Jump to CODE address a
MIN      (a b--c)          c: min(a,b)
MAX      (a b--c)          c: max(a,b)
RAND     (--n)             n: a RANDOM 32-bit number
EXIT     (--)              Leave the word immediately (make sure to UNLOOP first if in a LOOP)
TIMER    (--n)             n: clock()
WAIT     (n--)             n: MS to sleep
RESET    (--)              Initialize minForth
KEY      (--c)             c: Next keyboard char, wait if no press yet
KEY?     (--f)             f: FALSE if no keyboard press, else TRUE
+TMPS    (--)              Allocate 10 temp variables, r0 .. r9
rX       (--n)             n: read value of temp var X
sX       (n--)             Set value of temp var X to n
iX       (--n)             Increment temp var X
dX       (--n)             Decrement temp var X
-TMPS    (--)              Destroy current temp variables
."       (?--?)            Output a (possibly formatted) string. See (1).
"        (--a)             a: address of a string. See (2).
ZTYPE    (a--)             Output string at a. See (1).
QTYPE    (a--)             Quick string output, no formatting.
>R       (n--)             Move n to return stack
R>       (--n)             Move n from return stack
R@       (--n)             Copy n from return stack
ROT      (a b c--b c a)    Rotate a to TOS
-ROT     (a b c--c a b)    Rotate c before a
IF       (f--)             Standard IF
ELSE     (--)              Standard ELSE
THEN     (--)              Standard THEN
.IF      (f--)             Simple IF, no ELSE allowed (shorter)
.THEN    (--)              Simple THEN
.S       (--)              Output the stack
VARIABLE (--)              Define a variable
CONSTANT (--)              Define a constant
WORDS    (--)              Output the dictionary
SYSTEM   (a--)             a: string to send to system() ... eg: " dir" system
LOAD     (n--)             Load block n from disk
EDIT     (n--)             Edit block n
BYE      (--)              Exit minForth
NOP      (--)              Do nothing

(1) Notes on .":
- This is NOT ansi-standard
- %d: print TOS as integer
- %x: print TOS as hex
- %b: print TOS as binary
- %n: print new-line
- %q: print the quote (") character

example: : ascii $20 '~' for i i i i ." %n%d: (%c) %x %b" next ;

(2) Notes on ":
- This does not generate a standard FORTH counted string.
- It is NULL-terminated, no count byte.
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
