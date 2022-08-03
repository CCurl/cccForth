// Conway's game of life

1 load

variable (r) 100 (r) !
variable (c) 300 (c) !
: rows (r) @ ; : cols (c) @ ;
: pop-sz rows 2 + cols 2 + * ;

variable pop pop-sz allot
variable bak pop-sz allot
variable ln cols allot

: T5 ( a -- b ) dup c@ 1+ over c! 1+ ;
: b++ ( -- )
    r4 1- cols - T5 T5 T5 drop
    r4 1-        T5 1+ T5 drop
    r4 1- cols + T5 T5 T5 drop ;

: alive? ( a b -- c )
    dup 2 = .if drop exit .then
    3 = .if drop 1 exit .then
    drop 0 ;

: rand-pop 0 pop-sz for rand $FF and #200 > i pop + c! next ;
: clr-bak bak s1 0 pop-sz for 0 r1 c! i1 next ;
: bak->pop 0 pop-sz for 
        i bak + s1 i pop + s2
        r2 c@ r1 c@ alive? 
        r2 c! 
      0 r1 c!
    next ;

: ->p ( c r -- v ) cols * + pop + ;
: ->b ( c r -- v ) cols * + bak + ;

: .pop 1 dup ->XY
    1 rows 1+ for i s1 ln s6 
        1 cols 1+ for i r1 ->p c@ if '*' else bl then r6 c! i6 next 
        0 r6 c! ln qtype cr
    next ;

: gen 1 1 ->b s4
    1 1 ->p cols rows ->p
    for i c@ .if b++ .then i4 next
    bak->pop .pop r7 . i7 ;

// 60 (r) ! 200 (c) !
40 (r) ! 150 (c) !
: T1 begin gen key? until key drop ;
: go 1 s7 clr-bak rand-pop C-OFF CLS T1 C-ON ;
