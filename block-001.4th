reset

: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: vhere va @ ;
: here ha @ ;

( n a b -betw- f )
: betw +tmps s3 s2 s1  r2 r1 <=  r1 r3 <=  and -tmps ;

: .ch s9 r9 #32 #126 betw if r9 emit else r9 ." (%d)" then ;
: dump for i c@ . next ;
: dumpc for i c@ .ch next ;
: .code u dup here + 1- dumpc ;
: .vars vb vhere dumpc ;
: dump-code 0 s9 ." [" 
    u dup here + 1- for i c@ ." %d, " 
        i9 r9 10 = if 0 s9 cr then
    next ." 0]" ;

// Screen stuff
: ->XY #27 ." %c[%d;%dH" ;
: CLS #27 ." %c[2J" 1 dup ->XY ;
