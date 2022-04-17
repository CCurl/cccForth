// A genetic algorithm

1 load

100 constant maxC
 40 constant maxR
maxC 1+ maxR 1+ * constant world-sz
variable world world-sz allot

100 constant #crits
  8 constant #conns

: fill ( c f t-- ) for dup i c! next drop ;
: fill-n ( c f n-- ) over + fill ;
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
4 constant conn-sz

// critter:
// [c:1][r:1][color:1][unused:1][connections:?]
#conns conn-sz * 4+ constant critter-sz

variable critters #crits 1+ critter-sz * allot

// r6: the current critter
: setCrit ( n--r6 ) critter-sz * critters + s6 ;

: setCR ( c r-- ) r6 1+ c! r6 c! ;
: getCR ( --c r ) r6 c@ r6 1+ c@ ;
: setCLR ( n-- ) r6 2+ c! ;
: getCLR ( --n ) r6 2+ c@ ;

: randCrit r6 0 #conns for rand over ! 4+ next drop 
	getCR maxC mod 1+ swap maxR mod 1+ setCR
	getCLR 7 mod 31 + setCLR ;
: randCrits 1 #crits for i setCrit randCrit next ;

: crit->World getCLR getCR worldSet ;
: paintCrits worldClr 1 #crits for i setCrit crit->World next worldPaint ;

: normC ( a--b ) maxC min 1 max ;
: normR ( a--b ) maxR min 1 max ;
: normCR ( c c--c r ) normR swap normC swap ;
: up    getCR 1- normR setCR ;
: down  getCR 1+ normR setCR ;
: left  getCR swap 1- normC swap setCR ;
: right getCR swap 1+ normC swap setCR ;
: randOdd? rand 1 and ;
: up? randOdd? dup if up drop 1 then ;
: down?  if 1 leave then randOdd? dup if down drop 1 then ;
: left?  if 1 leave then randOdd? dup if left drop 1 then ;
: right? if leave then randOdd? if right then ;

: workCrit up? down? left? right? ;
: workCrits 1 #crits for i setCrit workCrit next ;
: oneDay workCrits paintCrits ;

: dumpCrit getCLR getCR swap i ." %d: (%d,%d) %d%n" ;
: dumpCrits 1 #crits for i setCrit dumpCrit next ;

: live 1 10 for oneDay next ;
: die ;
: regen randCrits ;
: go randCrits C-OFF 1 100 for live die regen next 37 FG C-ON ;
