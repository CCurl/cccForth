// Tests

12345 constant xxx
xxx .

: here ha @ cb + ;
here s7

: T1 IF 'Y' EMIT EXIT THEN 'N' EMIT ;
: T2 cr 1 2 3 + + . cr 19 42 for i . next cr 19 42 do i . -1 +loop ;
: T3 cr 20 s1 begin r1 . d1 r1 while ;
: T4 cr +tmps 123 445 s2 s1 r1 . r2 . -tmps ;
: T5 cr +tmps 666 777 s2 s1 r1 . r2 . T4 r1 . r2 . -tmps ;
: T6 s6 do i . i r6 = .if unloop exit .then loop ." out6" ;

cr 1 T1 0 T1 
T2 T3 T4 T5
80 52 72 T6
: prime? ( n--f ) 3 s2 s1 begin 
		r1 r2 /mod 0= if 0= unloop exit then 
		drop r2 dup * r1 > if unloop 1 exit then i2 i2 
	again ;
: primes ( n1--n2 ) +tmps 4 s5 11 do i prime? if i5 then 1 +loop r5 -tmps ;
: .primes ( n1-- )  1 . 2 . 3 for i prime? if i . then 1 +i next ;
: elapsed timer swap - ;
: bm timer swap primes . elapsed ." primes. (%d ms)" ;
: mil 1000 dup * * ;
." (got-here-1)"
cr see prime? cr
." (got-here-2)"
71 prime? ."  71 prime? (%d)%n"
cr cr 1 mil 2/ bm cr
cr 100 .primes ." (primes in 100)%n"

: T1 ." %nx1-"  9 begin dup . 1+ dup 34 < while ." -out1" drop ;
: T2 ." %nx2-"  8 begin dup . 1+ dup 27 = until ." -out2" drop ;
: T3 ." %nx3-"  8 s1 begin r1 . i1 r1 30 > .if unloop exit .then again ." -out2" drop ;
T1 T2 T3

cr words
