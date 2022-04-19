// A genetic algorithm

1 load

200 constant maxC
 55 constant maxR
maxC 1+ maxR 1+ * constant world-sz
variable world world-sz allot

500 constant #crits
  8 constant #conns

: fill ( c f t-- ) for dup i c! next drop ;
: fill-n ( c a n-- ) over + 1- fill ;
: cells ( n1--n2 ) cell * ;
: +! ( n a--) tuck @ + swap ! ;

: worldClr 0 world world-sz fill-n ;
: T0 ( c r--a ) maxC * + world + ;
: worldSet ( n c r--) T0 c! ;
: worldGet ( c r--n ) T0 c@ ;
: worldPaintR ( r-- ) 1 swap T0
	1 maxC for dup c@ dup if FG '*' else drop bl then emit 1+ next
	drop cr ;
: worldPaint ( -- ) 1 1 ->XY 1 maxR for i worldPaintR next ;

// connection
// [from:1][to:1][weight:2]
// from: [type:1][id:7] - type: 0=>input, 1=>hidden
// to:   [type:1][id:7] - type: 0=>output, 1=>hidden
// weight: normalized to -400 to 400
4 constant conn-sz

// A crude approximation of tanh(x)
: tanh dup 82 < if 85 * 100 / else 25 * 100 / 51 + then 99 min -99 max ;
: fire? ( n--f ) tanh rand abs #100 mod >= ;

8 constant #hidden
variable hidden #hidden cells allot

8 constant #output
variable output #output cells allot

: normWT ( n1--n2 ) ;
: normID ( n1--n2 ) $8f and ;
: id-t ( n--id t ) $80 /mod swap ;
: wt ( a--wt ) 2+ w@ dup $ff > if $ff and negate then ;
: wipe 0 hidden #hidden cells fill 0 output #output cells fill ;
: ->hidden ( n--a ) cells hidden + ;
: ->output ( n--a ) cells output + ;
: get-input ( n1--n2 ) rand 100 mod ;
: get-hidden ( n1--n2 ) ->hidden @ tanh ;
: input ( --n ) r1 c@ id-t if get-hidden else get-input then r1 wt * ;
: output ( n-- ) r1 1+ c@ id-t if ->hidden else ->output then +! ;
: do-output ( n-- ) drop ;
: work-conns ( -- ) wipe r6 4+ +tmps s1
	1 #conns for input output r1 4+ s1 next
	1 #output for i ->output @ fire? if i do-output next
	-tmps ;

// critter:
// [c:1][r:1][color:1][unused:1][connections:?]
#conns conn-sz * 4+ constant critter-sz

variable critters #crits 1+ critter-sz * allot

// r6: the current critter
: setCrit ( n--r6 ) critter-sz * critters + s6 ;

: getC  ( --c ) r6 c@ ;
: setC  ( c-- ) r6 c! ;
: getR  ( --r ) r6 1+ c@ ;
: setR  ( r-- ) r6 1+ c! ;
: getCR ( --c r ) getC getR ;
: setCR ( c r-- ) setR setC ;

: setCLR ( n-- ) r6 2+ c! ;
: getCLR ( --n ) r6 2+ c@ ;

: randCrit r6 0 #conns for rand over ! 4+ next drop 
	getC maxC mod 1+ setC getR maxR mod 1+ setR
	getCLR 7 mod 31 + setCLR ;
: randCrits 1 #crits for i setCrit randCrit next ;

: normC ( c--c1 ) maxC min 1 max ;
: normR ( r--r1 ) maxR min 1 max ;

: up    getR 1- normR setR ;
: down  getR 1+ normR setR ;
: left  getC 1- normC setC ;
: right getC 1+ normC setC ;

: up?    dup 0 = if up    then ;
: down?  dup 1 = if down  then ;
: left?  dup 2 = if left  then ;
: right?     3 = if right then ;

: crit-Unpaint getCR ->XY space ;
: crit-Paint getCLR FG getCR ->XY '*' emit ;

: move rand 3 and up? down? left? right? ;
: workCrit crit-Unpaint work-conns crit-Paint ;
: workCrits 1 #crits for i setCrit workCrit next ;
: oneDay workCrits ;

: dumpCrit getCLR getCR swap i ." %d: (%d,%d) %d%n" ;
: dumpCrits 1 #crits for i setCrit dumpCrit next ;

: live CLS 1 1000 for oneDay key? if break then next ;
: die ;
: regen randCrits ;
: go randCrits C-OFF begin live die regen key? until key drop 37 FG C-ON 1 maxR ->XY ;
