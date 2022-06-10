reset

: hex $10 base! ;
: decimal #10 base! ;
: binary %10 base! ;

( : vhere va @ ; )
( : here ha @ ; )

: cells 4 * ;
: code cb here 1- for i c@ dup ':' = if cr then emit next ;

( n a b -btw- f )
: btw r+ s3 s2 s1 r2 r1 <= r1 r3 <= and r- ;

: .c dup BL $7E btw if emit exit then ." [%d]" ;
: dumpN  1 for dup  @ .  1+ next drop ;
: dumpNC 1 for dup c@ .c 1+ next drop ;
: elapsed timer swap - ;
( : fill for dup i c! next drop ;				    )
( : fill-n over + 1- fill ;					        )

// Screen stuff
: ->XY #27 ." %c[%d;%dH" ;
: CLS #27 ." %c[2J" 1 dup ->XY ;
: COLOR ( bg fg -- ) #27 ." %c[%d;%dm" ;
: FG ( fg -- ) 40 swap COLOR ;
: C-ON  #27 ." %c[?25h" ;
: C-OFF #27 ." %c[?25l" ;
