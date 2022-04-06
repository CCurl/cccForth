// Tests

reset
1 load

: mil 1000 dup * * ;
: bm timer swap 1 for next timer swap - . ;

: T1 if 'Y' else 'N' then emit ;
: T2 1 2 3 + + . 19 42 for i . next ;
: T3 20 begin dup . 1- while ;
: T4 +tmps 123 s0 445 s1     r0 . r1 . -tmps ;
: T5 +tmps 666 s0 777 s1 T4 r0 . r1 . -tmps ;

cr 1 T1 0 T1 cr T2 cr T3 cr T4 cr T5

: x1 ." %nx1-" 9 s1 begin r1 dup . 1+ s1 r1 34 < while ." -out1" ;
: x2 ." %nx2-" 8 s1 begin r1 dup . 1+ s1 r1 27 = until ." -out2" ;
: x3 ." %nx3-" 7 s1 begin r1 41 = if break then r1 dup . 1+ s1 again ." -out3" ;

x1 x2 x3

cr words
cr .code
