// String words

// str-catc   ( c dst-- )    Concatenate char c to dst
// str-catd   ( d dst-- )    Concatenate digit d to dst
// str-catnw  ( n w dst-- )  Stringify n to width w and append to dst
// str-catn   ( n dst-- )    Stringify n and append to dst

: str-catc ( c dst-- ) str-end 0 over 1+ c! c! ;
: str-catd ( d dst-- ) swap dup 9 > .if 7 + .then '0' + swap str-catc ;
: str-catnw ( n w dst-- ) +tmps s2 s3 0 s1 
    begin i1 base @ /mod swap dup while drop
	r1 r3 < if r1 r3 for '0' r2 str-catc next then
    0 r1 for r2 str-catd next 
    -tmps ;
: str-catn ( n dst-- ) 0 swap str-catnw ;
