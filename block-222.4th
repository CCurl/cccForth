// A genetic algorithm

1 load

100 constant maxX
 40 constant maxY

100 constant #crits
  8 constant #conns

// connection
// [from:1][to:1][weight:2]
4 constant conn-sz

// critter:
// [x:1][y:1][color:1][unused:1][connections:?]
#conns conn-sz * 4 + constant critter-sz

variable critters #crits 1+ critter-sz * allot

// r6 and (C): the current critter
// variable (C)
// : C (C) @ ;
: setCrit ( n--r6 ) critter-sz * critters + s6 ;

: setXY ( x y-- ) r6 1+ c! r6 c! ;
: getXY ( --x y ) r6 c@ r6 1+ c@ ;
: setCLR ( c-- ) r6 2+ c! ;
: getCLR ( --c ) r6 2+ c@ ;

: randCrit r6 0 #conns for rand over ! 4 + next drop 
	getXY maxX mod 1+ swap maxY mod 1+ setXY
	getCLR 7 mod 31 + setCLR ;
: randCrits 1 #crits for i setCrit randCrit next ;

: paintCrit getXY ->XY 40 getCLR COLOR '*' emit ;
: paintCrits 1 #crits for i setCrit paintCrit next ;

: workCrit ;
: workCrits 1 #crits for i setCrit workCrit next ;
: oneDay workCrits CLS paintCrits ;

: dumpCrit getXY swap i ." %d: (%d,%d)%n" ;
: dumpCrits 1 #crits for i setCrit dumpCrit next ;

: life 1 1 for oneDay next ;
: die ;
: regen randCrits ;
: go randCrits C-OFF 1 100 for life die regen next 37 FG C-ON ;
