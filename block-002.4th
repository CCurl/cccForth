// Strings

: strtrunc ( a--a ) dup 0 swap c! ;
: strend ( a1--a2 ) count + ;
: strcat ( src dst-- ) +tmps strend s2 s1
    begin
        r1 c@ dup r2 c! i1 i2
        0= .if -tmps unloop-w exit .then
    again ;
: strcpy ( src dst-- ) dup strtrunc strcat ;
: strcatc ( c dst-- ) strend 0 over 1+ c! c! ;
: strcatd ( d dst-- ) swap '0' + dup '9' > .if 7 + .then swap strcatc ;
: strcatnw ( n w dst-- ) +tmps s2 s3 0 s1 
    begin i1 base @ /mod swap dup while drop
	r1 r3 < if r1 r3 for 0 r2 strcatd next then
    0 r1 for r2 strcatd next 
    -tmps ;
: strcatn ( n dst-- ) 0 swap strcatnw ;
