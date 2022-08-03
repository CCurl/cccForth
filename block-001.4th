reset

: vhere va @ vb + ;
: here ha @ cb + ;

: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: betw ( n min max--f ) +tmps s3 s2 s1  r1 r2 >=  r1 r3 <=  and -tmps ;
: T0 ( n-- ) s0  r0 #32 $7e betw if r0 emit else r0 ." (%d)" then ;
: code ( -- ) cb here  for i c@ T0 next ;
: vars ( -- ) vb vhere for i c@ T0 next ;
: fill ( c f t-- ) for dup i c! next drop ;
: fill-n ( c a n-- ) over + fill ;

// Screen stuff
: ->XY #27 ." %c[%d;%dH" ;
: CLS #27 ." %c[2J" 1 dup ->XY ;
: COLOR ( bg fg -- ) #27 ." %c[%d;%dm" ;
: FG ( fg -- ) 40 swap COLOR ;
: C-ON  #27 ." %c[?25h" ;
: C-OFF #27 ." %c[?25l" ;

: min ( a b--x ) dup dup > .if swap .then drop ;
: max ( a b--x ) dup dup < .if swap .then drop ;
