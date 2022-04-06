# minForth

The main goals for this minimal Forth are as follows:

- For the implementation of it to be minimal and "intuitively obvious upon casual inspection".
- To be able to run under any system that has a C compiler.
- To be deployable to as many different kinds of development boards as possible via the Arduino IDE.
- To be easy to modify and extend the primitives.

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
