reset

: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: vhere va @ vb + ;
: here ha @ cb + ;

( n a b -betw- f )
: betw +tmps s3 s2 s1  r2 r1 <=  r1 r3 <=  and -tmps ;

: T0 ( c-- ) dup BL $7E betw .if emit exit .then ." (%d)" ;
: code cb here for i c@ T0 next ;
: vars vb vhere for i c@ T0 next ;
: fill ( c f t-- ) for dup i c! next drop ;
: fill-n ( c a n-- ) over + fill ;

// Screen stuff
: ->XY #27 ." %c[%d;%dH" ;
: CLS #27 ." %c[2J" 1 dup ->XY ;
: COLOR ( bg fg -- ) #27 ." %c[%d;%dm" ;
: FG ( fg -- ) 40 swap COLOR ;
: C-ON  #27 ." %c[?25h" ;
: C-OFF #27 ." %c[?25l" ;
