( reset )

( : hex $10 base ! ; )
( : decimal #10 base ! ; )
( : binary %10 base ! ; )

( : vhere va @ ; )
( : here ha @ ; )

: cells 4 * ;

( n a b -betw- f )
( : betw +tmps s3 s2 s1  r2 r1 <=  r1 r3 <=  and -tmps ; )

( : T0 dup BL $7E betw .if emit exit .then ." [%d]" ; )
( : dumpN   1 for dup c@ .  1+ next drop ;		    )
( : dumpNC  1 for dup c@ T0 1+ next drop ;		    )
( : elapsed timer swap - ;							)
( : fill for dup i c! next drop ;				    )
( : fill-n over + 1- fill ;					        )

// Screen stuff
: ->XY #27 ." %c[%d;%dH" ;
: CLS #27 ." %c[2J" 1 dup ->XY ;
: COLOR ( bg fg -- ) #27 ." %c[%d;%dm" ;
: FG ( fg -- ) 40 swap COLOR ;
: C-ON  #27 ." %c[?25h" ;
: C-OFF #27 ." %c[?25l" ;
