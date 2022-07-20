// Tests

12345 constant xxx
xxx .

: elapsed timer swap - ;
: mil 1000 dup * * ;
: bm timer swap 1 for next elapsed ." %d ms" ;

: here ha @ cb + ;
here s7

: T1 if 'Y' else 'N' then emit ;
: T2 cr 1 2 3 + + . 19 42 for i . next ;
: T3 cr 20 s1 begin r1 . d1 r1 while ;
: T4 cr +tmps 123 445 s2 s1 r1 . r2 . -tmps ;
: T5 cr +tmps 666 777 s2 s1 r1 . r2 . T4 r1 . r2 . -tmps ;
: T6 s6 for i . i r6 = .if unloop-f exit .then next ." out6" ;

: xx ;

cr 1 T1 0 T1 
T2 T3 T4 T5
50 80 72 T6

: T1 ." %nx1-"  9 begin dup . 1+ dup 34 < while ." -out1" drop ;
: T2 ." %nx2-"  8 begin dup . 1+ dup 27 = until ." -out2" drop ;
: T3 ." %nx3-"  8 s1 begin r1 . i1 r1 30 > .if unloop-w exit .then again ." -out2" drop ;
T1 T2 T3

cr words
