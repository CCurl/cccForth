// Conway's game of life

1 load

: rows 50 ; : cols 250 ;
: pop-sz rows 2 + cols * 2 + ;

variable pop pop-sz allot
variable bak pop-sz allot
variable ln cols allot
variable WT 0 WT !

: T5 ( a -- ) dup c@ 1+ over c! 1+ ;
: b++ ( c r -- ) cols * + bak + s0
    r0 1- cols - T5 T5 T5 drop
    r0 1-        T5 1+ T5 drop
    r0 1- cols + T5 T5 T5 drop ;

: alive? ( a b -- c ) s1 s0
    r1 2 < if 0 leave then
    r1 3 > if 0 leave then
    r1 3 = if 1 leave then
    r0 ;

: rand-pop 0 pop-sz for rand $FF and #200 > i pop + c! next ;
: clr-bak 0 pop-sz for 0 i bak + c! next ;
: bak->pop 0 pop-sz for 
        i pop + c@ 
        i bak + c@ alive? 
        i pop + c! 
    next ;

: p@ ( c r -- v ) cols * + pop + c@ ;

: .pop C-OFF 1 dup ->XY
    1 rows for i s1
		ln s6 
        1 cols for i r1 p@ if '*' else bl then r6 c! i6 next 
		0 r6 c! ln qtype cr
    next C-ON ;

: gen clr-bak
    1 rows for i s8
        1 cols for
            i r8 p@ if i r8 b++ then
        next
    next 
    bak->pop .pop i7 r7 . WT @ wait ;

: gens 1 for gen next ;
: go 0 s7 rand-pop gens ;
