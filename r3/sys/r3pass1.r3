| r3 compiler
| pass 1 - load all includes and define order of compiler
| PHREDA 2018
|----------------
^r3/lib/str.r3
^r3/lib/trace.r3

^./r3base.r3

|----------- comments / configuration
:setgraf | adr -- adr
	3 + trim
|	"F" =pre 1? ( fullscreen ) drop
|	"W" =pre 1? ( size ) drop
	>>cr ;

:escom
	"|WIN|" =pre 1? ( drop 5 + ; ) drop | Compila para WINDOWS
|	"||G" =pre 1? ( setgraf ; ) drop | GRAPHIC (F/Ww,h)(x2/x3/x4)
	| MOUSE/PEN
	| SOCKET
	| JOYSTICK
	| CAMERA
	| STREAM
	| CONSOLE
    >>cr ;


:includepal | str car -- str'
	$7c =? ( drop escom ; )		| $7c |	 Comentario
	$3A =? ( 1 'cntdef +! )		| $3a :  Definicion
	$23 =? ( 1 'cntdef +! )		| $23 #  Variable
	1 'cnttokens +!
	$22 =? ( drop >>" ; )		| $22 "	 Cadena
	drop >>sp ;

|----------- includes
:ininc? | str -- str adr/0
	'inc ( inc> <?
		@+ pick2 =s 1? ( drop ; ) drop
		4 + ) drop 0 ;

:load.inc | str -- str new ; incluye codigo
    here over		| str here str

|	"." =pre 1? ( drop 2 + 'path "%s%w" mprint )( drop "%w" mprint )
|	'r3path "%s/%l" mprint

	"%l" mprint
|	dup slog

	load here =? ( drop 0 "File not found" 'error ! "File not found" slog ; ) | no existe
	here
	0 rot c!+ 'here !
	;

:add.inc | src here -- src
	over inc> !+ !+ 'inc> ! ;

:includes | src --
	dup ( trimcar 1?
		( $5e =? drop | $5e ^  Include
			ininc? 0? ( drop
				load.inc 0? ( drop ; ) |no existe
				includes
				error 1? ( drop ; ) drop
				dup ) drop
			>>cr trimcar )
		includepal
		) 2drop
	add.inc ;

::r3-stage-1 | filename str -- err/0
	includes
	;
