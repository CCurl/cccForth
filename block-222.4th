// A genetic algorithm

1 load
223 load

200 constant maxC
 55 constant maxR
maxC 1+ maxR 1+ * constant world-sz
variable world world-sz allot

500 constant #crits
  8 constant #conns

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

: +rand rand abs ;
: rand-cr +rand maxC mod 1+ setC +rand maxR mod 1+ setR ;
: rand-clr rand 7 mod 31 + setCLR ;
: randCrit rand-cr rand-clr r6 4+ 1 #conns for rand-neu over ! 4+ next drop ;
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
: workCrit crit-Unpaint r6 4+ work-conns crit-Paint ;
: workCrits 1 #crits for i setCrit workCrit next ;
: oneDay workCrits ;

: dumpCrit getCLR getCR swap i ." %d: (%d,%d) %d%n" ;
: dumpCrits 1 #crits for i setCrit dumpCrit next ;

: live CLS 1 1000 for oneDay key? if break then next ;
: die ;
: regen randCrits ;
: go randCrits C-OFF begin live die regen key? until key drop 37 FG C-ON 1 maxR ->XY ;
