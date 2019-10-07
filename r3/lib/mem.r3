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

