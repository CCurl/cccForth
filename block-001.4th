reset

: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: vhere va @ ;
: here ha @ ;

( n a b -betw- f )
: betw +tmps s3 s2 s1  r2 r1 <=  r1 r3 <=  and -tmps ;

: T0 ( c-- ) dup BL $7E betw .if emit exit .then ." (%d)" ;
: dumpN  ( a n-- ) 0 for dup c@ .  1+ next drop ;
: dumpNC ( a n-- ) 0 for dup c@ T0 1+ next drop ;
: code cb here dumpNC ;
: vars vb vhere dumpNC ;
: elapsed timer swap - ;
: fill ( c f t-- ) for dup i c! next drop ;
: fill-n ( c a n-- ) over + fill ;

// Screen stuff
: ->XY #27 ." %c[%d;%dH" ;
: CLS #27 ." %c[2J" 1 dup ->XY ;
: COLOR ( bg fg -- ) #27 ." %c[%d;%dm" ;
: FG ( fg -- ) 40 swap COLOR ;
: C-ON  #27 ." %c[?25h" ;
: C-OFF #27 ." %c[?25l" ;
