// Strings

: strtrunc ( a-- ) 0 swap c! ;
: strend ( a1--a2 ) begin dup c@ 0= .if unloop-w exit .then 1+ again ;
: strcat ( src dst-- ) +tmps strend s2 s1
    begin
        r1 c@ dup r2 c! i1 i2
        0= .if -tmps unloop-w exit .then
    again ;
: strcpy ( src dst-- ) dup strtrunc strcat ;
: strcatc ( c dst-- ) strend 0 over 1+ c! c! ;
