reset

: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: vhere va @ ;
: here ha @ ;

( n a b -betw- f )
: betw +tmps s3 s2 s1  r2 r1 <=  r1 r3 <=  and -tmps ;

: T0 ( c-- ) s9 r9 BL $7E betw if r9 emit else r9 ." (%d)" then ;
: dumpN  ( a n-- ) 1 for dup c@ .  1+ next drop ;
: dumpNC ( a n-- ) 1 for dup c@ T0 1+ next drop ;
: .code cb here dumpNC ;
: .vars vb vhere vb - dumpNC ;
: dump-code 0 s9 ." [" 
    cb dup here + 1- for i c@ ." %d, " 
        i9 r9 10 = if 0 s9 cr then
    next ." 0]" ;
: elapsed timer swap - ;

variable s
: rand s @ dup 0= if drop timer then
    dup 13 << xor
    dup 17 >> xor
    dup  5 << xor dup s ! ;

// Screen stuff
: ->XY #27 ." %c[%d;%dH" ;
: CLS #27 ." %c[2J" 1 dup ->XY ;
: C-ON  #27 ." %c[?25h" ;
: C-OFF #27 ." %c[?25l" ;
