// "brain" stuff ...

// 1 load

8 constant #conns

8 constant #hidden
variable hidden #hidden cells allot

8 constant #output
variable output #output cells allot

// A crude approximation of tanh(x)
: tanh ( n1--n2 ) dup 82 < if 85 * 100 / else 25 * 100 / 49 + then 99 min -99 max ;
: tanh-graph 1 300 for i tanh 100 swap - i swap ->XY '*' emit next ;

: fire? ( n--f ) tanh rand abs #100 mod >= ;

: normWT ( n1--n2 ) dup $7fff > if $10000 - then ;
: normID ( n1--n2 ) $8f and ;
: id-t ( n--id t ) $80 /mod swap ;
: wt ( a--wt ) 2+ w@ normWT ;

: rand-neu ( --n ) rand normID 8 << rand normID or 16 << rand 400 mod $ffff and or ;
: .tid normID id-t ." (%d,%d) " ;
: .neuron ( n-- ) dup 24 >> .tid dup 16 >> .tid $ffff and normWT . ;

: wipe 0 hidden #hidden cells fill 0 output #output cells fill ;
: ->hidden ( n--a ) cells hidden + ;
: ->output ( n--a ) cells output + ;
: get-input ( id--n ) 
	// id == 1: ??
	// id == 2: ??
	// id == 3: ??
	// id == 4: ??
	drop rand abs 100 mod ;
: get-hidden ( n1--n2 ) ->hidden @ tanh ;
: input ( --n ) r1 c@ id-t if get-hidden else get-input then r1 wt * 100 / ;
: output ( n-- ) r1 1+ c@ id-t if ->hidden else ->output then +! ;
: do-output ( id-- r0 is critter ) 
	// id == 1: move up
	// id == 2: move down
	// id == 3: move left
	// id == 4: move right
	drop ;
: work-conns ( ca-- ) +tmps dup s0 4+ s1 wipe
	1 #conns for input output r1 4+ s1 next
	1 #output for i ->output @ fire? if i do-output then next
	-tmps ;
