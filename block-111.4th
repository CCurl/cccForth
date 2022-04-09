// Conway's game of life

1 load

: rows 40 ; : cols 100 ;
: pop-sz rows 2 + cols * 2 + ;

variable pop pop-sz allot
variable bak pop-sz allot

: p ( r c -- n ) swap cols * + pop + ;
: p@ ( r c -- v ) p c@ ;

: ->b ( r c -- n ) swap cols * + bak + ;
: b+ ( a -- ) dup dup c@ 1+ swap c! 1+ ;
: b++ ( r c -- ) ->b s0
    r0 1- cols - b+ b+ b+ drop
    r0 1-        b+ 1+ b+ drop
    r0 1- cols + b+ b+ b+ drop ;

variable s

: rnd s @ dup 0= if drop timer then
    dup 13 << xor
    dup 17 >> xor
    dup  5 << xor dup s ! ;

: alive? ( a b -- c ) s1 s0
    r1 2 < if 0 leave then
    r1 3 > if 0 leave then
    r1 3 = if 1 leave then
    r0 ;

: rnd-pop 0 pop-sz for rnd $FF and #200 > i pop + c! next ;
: clr-bak 0 pop-sz for 0 i bak + c! next ;
: bak->pop 0 pop-sz for 
        i pop + c@ 
        i bak + c@ alive? 
        i pop + c! 
    next ;

: .pop C-OFF 1 1 ->XY
    1 rows for i s1
        1 cols for r1 i p@ if '*' else bl then emit next cr
    next C-ON ;

: gen clr-bak
    1 rows for i s8
        1 cols for i s9
            r8 r9 p@ if r8 r9 b++ then
        next
    next 
    bak->pop .pop ;

: gens 1 for gen next ;
: go rnd-pop gens ;

: rrr 111 load ;
