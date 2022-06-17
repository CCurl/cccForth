// Tests

reset
1 load

: mil 1000 dup * * ;
: bm timer swap 1 for next elapsed ." %d ms" ;

here s7

: T1 if 'Y' else 'N' then emit ;
: T2 cr 1 2 3 + + . 19 42 for i . next ;
: T3 cr 20 s1 begin r1 . d1 r1 while ;
: T4 cr +tmps 123 445 s2 s1 r1 . r2 . -tmps ;
: T5 cr +tmps 666 777 s2 s1 r1 . r2 . T4 r1 . r2 . -tmps ;
: T6 cr 211 245 for i dup . 223 = if leave then next ." -out0" ;

cr 1 T1 0 T1 
T2 T3 T4 T5 T6

: T1 ." %nx1-"  9 s1 begin r1 . i1 r1 34 < while ." -out1" ;
: T2 ." %nx2-"  8 s1 begin r1 . i1 r1 27 = until ." -out2" ;
: T3 ." %nx3-"  7 s1 begin r1 41 = if leave then r1 . i1 again ." -out3" ;
: T4 ." %nx4-"  7 s1 begin r1 41 = if unloop exit then r1 . i1 again ." -out4" ;
: T5 T4 ." -out5" ;
T1 T2 T3 T4 T5

cr words
