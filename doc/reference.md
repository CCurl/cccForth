# MinFORTH reference

## Built-in words

NOTES:
1. Built-in words are not case sensitive
2. They do not show up in WORDS, and are not in the dictionary.

|Word|opcode|stack|notes|
|-|-|-|-|
|DUP|#|(a--a a)|FORTH CORE|
|OVER|%|(a b--a b a)|FORTH CORE|
|SWAP|$|(a b--b a)|FORTH CORE|
|DROP|\\ |(a b--a)|FORTH CORE|
|@ |@|(a--n)|FETCH (long)|
|! |!|(n a----)|STORE (long)|
|W@|w|(a--n)|FETCH (word)|
|W!|W|(n a----)|STORE (word)|
|C@|c|(a--n)|FETCH (byte)|
|C!|C|(n a----)|STORE (byte)|
|IF|j|(f--)|FORTH CORE (0BRANCH)|
|ELSE|(none)|(--)|FORTH CORE|
|THEN|(none)|(--)|FORTH CORE|
|EMIT|,|(n--)|FORTH CORE|
|BL|k|(--n)|FORTH CORE|
|SPACE|b|(--)|FORTH CORE|
|CR|n|(--)|FORTH CORE|
|KEY?|?|(--n)|FORTH CORE|
|KEY|K|(--n)|FORTH CORE|
|+,-,*,/|+,-,*,/|(a b--n)|FORTH CORE|
|/MOD|&|(a b--q r)|FORTH CORE|
|AND,OR,XOR|A,O,X|(a b--n)|FORTH CORE|
|COM|~|(a--b)|FORTH CORE|
|1+,1-|I,D|(a--b)|FORTH CORE|
|<,=,>,0=|<,=,>,N|(a b--f)|FORTH CORE|
|."|Z|(a--)|Type - NOT FORTH standard (1)|
|"|(none)|(--a)|String - NOT FORTH standard(1)|
|FOR|\[|(F T--)|For loop (2)|
|I|i|(--n)|Current index|
|+I|m|(n--)|Add n to index (I)|
|NEXT|\]|(--)|Next: I += 1, if I <= T, jump to FOR|
|BEGIN|{|(--)|Start of BEGIN loop|
|WHILE|v|(n--)|If n != 0, jump to BEGIN|
|UNTIL|u|(n--)|If n == 0, jump to BEGIN|
|AGAIN|}|(--)|Jump to BEGIN|
|BREAK|^|(--)|Break out of FOR or BEGIN loop|
|(.)|.|(--)|Internal: used by .|
|CALL|:|(--)|Internal|
|;,LEAVE|;|(--)|FORTH CORE|
|BLIT|1|(--n)|Internal: byte literal|
|WLIT|2|(--n)|Internal: word literal|
|LIT|4|(--n)|Internal: long literal|
|EXECUTE|G|(a--)|FORTH CORE|
|<<|L|(a n--b)|LSHIFT|
|>>|R|(a n--b)|RSHIFT|
|EDIT|zE|(n--)|Extension: __EDITOR__|
|LOAD|zL|(n--)|Extension: __FILE__|
|BYE|zZ|(--)|Extension: PC only|
|+tmp|p|(--)|TempVars: allocate new|
|-tmp|q|(--)|TempVars: destroy last allocated|
|r0..r9|r|(--n)|TempVar(0..9): read|
|s0..s9|r|(--n)|TempVar(0..9): set|
|i0..i9|r|(--n)|TempVar(0..9): increment|
|d0..d9|s|(n--)|TempVar(0..9): decrement|
|TIMER|t|(--n)|Push current MS|
|RESET|Y|(--)||

```
(1) Notes on " and .":
- These are NOT standard FORTH counted strings
- They are NULL-terminated
- %d: print TOS as integer
- %x: print TOS as hex
- %b: print TOS as binary
- %n: print new-line
- %q: print the quote (") character

example: : ascii $20 '~' for i i i i ." %n%d: (%c) %x %b" next ;
```
```
(2) Notes on FOR:
- F and T can be specified in any order.

example: "1 10 for i . next" prints 1 2 3 4 5 6 7 8 9 10
         "10 1 for i . next" prints 1 2 3 4 5 6 7 8 9 10
```
## Default dictionary entries:
|Word|stack|notes|
|-|-|-|
|cb   |(--a)|Base address for CODE space|
|vb  |(--a)|Base address for VARIABLES|
|vsz |(--a)|Size of VARIABLES space (bytes)|
|csz |(--a)|Size of CODE space (bytes)|
|va  |(--a)|Address of VHERE|
|la  |(--a)|Address of LAST|
|ha  |(--a)|Address of HERE|
|base|(--a)|Address of BASE|
