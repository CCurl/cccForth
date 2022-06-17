# MinFORTH reference

## Built-in words

NOTES:
1. Built-in words are not case sensitive
2. They do not show up in WORDS, and are not in the dictionary.

|Word|macro|stack|notes|
|-|-|-|-|
|+|+|(a b--c)|FORTH CORE|
|-|-|(a b--c)|FORTH CORE|
|/|/|(a b--c)|FORTH CORE|
|*|*|(a b--c)|FORTH CORE|
|/MOD|&|(a b--c)|FORTH CORE|
|MOD|M|(a b--c)|FORTH CORE|
|SWAP|$|(a b--c)|FORTH CORE|
|DROP|`\`|(a b--c)|FORTH CORE|
|OVER|%|(a b--c)|FORTH CORE|
|DUP|#|(a b--c)|FORTH CORE|
|NIP|`$`|(a b--c)|FORTH CORE|
|TUCK|$%|(a b--c)|FORTH CORE|
|2DUP|%%|(a b--c)|FORTH CORE|
|2DROP|`\\`|(a b--c)|FORTH CORE|
|EMIT|,|(a b--c)|FORTH CORE|
|(.)|.|(a b--c)|FORTH CORE|
|SPACE|32,|(a b--c)|FORTH CORE|
|CR|13,10,|(a b--c)|FORTH CORE|
|BL|32|(a b--c)|FORTH CORE|
|=|=|(a b--c)|FORTH CORE|
|<|<|(a b--c)|FORTH CORE|
|>|>|(a b--c)|FORTH CORE|
|<=|>N|(a b--c)|FORTH CORE|
|>=|<N|(a b--c)|FORTH CORE|
|<>|=N|(a b--c)|FORTH CORE|
|!=|=N|(a b--c)|FORTH CORE|
|0=|N|(a b--c)|FORTH CORE|
|ABS|#0<(\_)|(a b--c)|FORTH CORE|
|NEGATE|\_|(a b--c)|FORTH CORE|
|<<|L|(a b--c)|FORTH CORE|
|>>|R|(a b--c)|FORTH CORE|
|ZLEN|T|(a b--c)|FORTH CORE|
|.|.32,|(a b--c)|FORTH CORE|
|@|@|(a b--c)|FORTH CORE|
|C@|c|(a b--c)|FORTH CORE|
|W@|w|(a b--c)|FORTH CORE|
|!|!|(a b--c)|FORTH CORE|
|C!|C|(a b--c)|FORTH CORE|
|W!|W|(a b--c)|FORTH CORE|
|AND|a|(a b--c)|FORTH CORE|
|OR|o|(a b--c)|FORTH CORE|
|XOR|x|(a b--c)|FORTH CORE|
|COM|~|(a b--c)|FORTH CORE|
|NOT|N|(a b--c)|FORTH CORE|
|1+|P|(a b--c)|FORTH CORE|
|2+|PP|(a b--c)|FORTH CORE|
|4+|PPPP|(a b--c)|FORTH CORE|
|+!|$%@+$!|(a b--c)|FORTH CORE|
|1-|D|(a b--c)|FORTH CORE|
|I|I|(a b--c)|FORTH CORE|
|+I|m|(a b--c)|FORTH CORE|
|EXECUTE|G|(a b--c)|FORTH CORE|
|MIN|`%%>($)\`|(a b--c)|FORTH CORE|
|MAX|`%%<($)\`|(a b--c)|FORTH CORE|
|RAND|zR|(a b--c)|FORTH CORE|
|EXIT|;|(a b--c)|FORTH CORE|
|TIMER|zT|(a b--c)|FORTH CORE|
|WAIT|zW|(a b--c)|FORTH CORE|
|RESET|Y|(a b--c)|FORTH CORE|
|UNLOOP|k|(a b--c)|FORTH CORE|
|LEAVE|l|(a b--c)|FORTH CORE|
|KEY|K|(a b--c)|FORTH CORE|
|KEY?|?|(a b--c)|FORTH CORE|
|+TMPS|p|(a b--c)|FORTH CORE|
|-TMPS|q|(a b--c)|FORTH CORE|
|ZTYPE|Z|(a b--c)|FORTH CORE|
|QTYPE|t|(a b--c)|FORTH CORE|
|>R|Q<|(a b--c)|FORTH CORE|
|R>|Q>|(a b--c)|FORTH CORE|
|R@|Q@|(a b--c)|FORTH CORE|
|ROT|Q<$Q>$|(a b--c)|FORTH CORE|
|-ROT|$Q<$Q>|(a b--c)|FORTH CORE|
|.IF|(|(a b--c)|FORTH CORE|
|.THEN|)|(a b--c)|FORTH CORE|
|.S|zS|(a b--c)|FORTH CORE|
|NOP|(space)|(a b--c)|FORTH CORE|

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
