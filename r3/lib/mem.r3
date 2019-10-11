| PHREDA

|--- Memoria
::zcopy | destino fuente -- destino' con 0
	( @+ 1? rot !+ swap ) rot !+ nip ;
::strcat | src des --
	( c@+ 1? drop ) drop 1 -
::strcpy | src des --
	( swap c@+ 1? rot c!+ ) nip swap c! ;
::strcpyl | src des -- ndes
	( swap c@+ 1? rot c!+ ) rot c!+ nip ;
::strcpyln | src des --
	( swap c@+ 1? 13 =? ( 2drop 0 swap c! ; )
		rot c!+ ) rot c!+ drop ;

::copynom | sc s1 --
	( c@+ 32 >?
		rot c!+ swap ) 2drop
	0 swap c! ;

::copystr | sc s1 --
	( c@+ 34 <>?
		rot c!+ swap ) 2drop
	0 swap c! ;

::toupp | c -- C
	$df and ;

::tolow | C -- c
	$20 or ;


::count | s1 -- s1 cnt	v3
	dup >a
	0 ( a@+ dup $01010101 - swap not and
		$80808080 na? drop 4 + )
	$80 an? ( drop ; )
	$8000 an? ( drop 1 + ; )
	$800000 an? ( drop 2 + ; )
	drop 3 + ;

::= | s1 s2 -- 1/0
	( swap c@+ 1?
		toupp rot c@+ toupp rot -
		1? ( 3drop 0 ; ) drop
		) 2drop
	c@ $ff and 33 <? ( drop 1 ; )
	drop 0 ;

::=pre | s1 s2 -- 1/0
	( c@+ 1?
		toupp rot c@+ toupp rot -
		1? ( 3drop 0 ; )
		drop swap )
	3drop 1 ;

::=w | s1 s2 -- 1/0
	( c@+ 32 >?
		toupp rot c@+ toupp rot -
		1? ( 3drop 0 ; )
		drop swap ) 2drop
	c@ $ff and 33 <? ( drop 1 ; )
	drop 0 ;

::=pos | s1 ".pos" -- s1 1/0
	over count
	rot count | s1 s1 cs1 "" c"
	rot swap - | s1 s1 "" dc
	rot + | s1 "" s1.
	= ;
