|SCR 800 600
||FULL
| TV
| PHREDA 2019

^r3/lib/sys.r3
^r3/lib/rand.r3
^r3/lib/print.r3

^r3/util/arr16.r3
^r3/util/polygr.r3
^r3/util/penner.r3

^r3/lib/fontr.r3

^r3/rft/robotobold.rft
^r3/rft/robotoregular.rft
^r3/rft/droidsansbold.rft
^r3/rft/comfortaa_bold.rft
^r3/rft/archivoblackregular.rft
^r3/rft/dejavusans.rft
^r3/rft/dejavuserifbold.rft
^r3/rft/opensanslight.rft
^r3/rft/opensansregular.rft

#screen 0 0
#scrbarra 0 0

#ltime 0 0	| tiempo

:reset
	'screen p.clear
	'scrbarra p.clear
	'ltime p.clear
	;

|---------------------------
| caja
| c x1 y1 x2 y2
:caja
	>b
	b@+ 'ink !
	b@+ 16 >> b@+ 16 >>
	b@+ 16 >> b@+ 16 >>
	fillbox ;

:+caja | x1 y1 x2 y2 color --
	'caja 'screen p!+ >b
	b!+
	swap 16 << b!+ 16 << b!+
	swap 16 << b!+ 16 << b!+
	;

|---------------------------
| letra
| x y w h c r adr
|4 8 12162024
#lastx

:letrap | adr --
	>b
	b@+
	dup 1.0 - b> 4 - !
	16 >> 'ccx ! b@+ 16 >> 'ccy !
	b@+ 16 >> 'ccw ! b@+ 16 >> 'cch !

	ccx ccw 1 >> +
	lastx >? ( dup 'lastx ! )
	-? ( drop 0 ; ) drop

	b@+ 'ink !
	b@+ fontremit
	;

:+letrap | adr --
	'letrap 'scrbarra p!+ >b
	ccx 16 << b!+ ccy 16 << b!+
	ccw 16 << b!+ cch 16 << b!+
	ink b!+
	b!+
	;

:+stringp | "" --
    '+letrap swap fontrprint
	;

:+barra | "" --
	droidsansbold 40 fontr!
	$444444 'ink !
	lastx 560 atxy
	+stringp
	;

:barradraw
	'scrbarra p.cnt 1? (
		$f0f0f0 'ink !
		0 550 800 600
		fillbox
		) drop
	sw 'lastx !
	'scrbarra p.draw
	;

|---------------------------
| letra
| x y w h c r adr
|4 8 12162024
:letra | adr --
	>b
	b@+ 16 >> 'ccx ! b@+ 16 >> 'ccy !
	b@+ 16 >> 'ccw ! b@+ 16 >> 'cch !
	b@+ 'ink !
	b@+ b@+ swap fontremitr
	;

:+letra | adr --
	'letra 'screen p!+ >b
	ccx 16 << b!+ ccy 16 << b!+
	ccw 16 << b!+ cch 16 << b!+
	ink b!+
	0 b!+
	b!+
	;

:+string | "" --
    '+letra swap fontrprint ;

:+bstring | "" color --
	>r swprint ccx ccy rot pick2 + over cch + r> +caja
    '+letra swap fontrprint ;

|---------- tiempo
#deltat
#prevt

:time.ini
	msec 'prevt ! 0 'deltat ! ;

:time.next
	msec dup prevt - 'deltat ! 'prevt ! ;

:t1 | adr --
	>a
	a@ deltat + dup a!+
	a@+ *. 1.0 min	| t
	a@+ over Ela_InOut
	*. a@+ + a@ !
	1.0 - 1? ( drop ; )
	;

:t2 | adr --
	>a
	a@ deltat + dup a!+
	a@+ *. 1.0 min	| t
	dup Ela_InOut
	a@+ over *. a@+ +	| t tc x1
	a@+ rot *. a@+ +	| t x1 y1
	swap a@ !+ !
	1.0 - 1? ( drop ; )
	;


|--- agrega a timeline
:+rot2d  | fn obj time --
	't1 'ltime p!+ >a
	0 a!+ 1000 *. 1.0 swap /. a!+
	20 +	| fn obj
	dup @			| fn obj obja
	rot over - a!+	| dB
	a!+				| A
	a! ;

:+scale  | fn obj time --
	't1 'ltime p!+ >a
	0 a!+ 1000 *. 1.0 swap /. a!+
	24 +	| escala
	dup @			| fn obj obja
	rot over - a!+	| dB
	a!+				| A
	a! ;

:+move2d | x y obj time --
	't2 'ltime p!+ >a
	0 a!+ 1000 *. 1.0 swap /. a!+ | ti tiempo
	| x y obj
	4 + rot over @ swap over - a!+ a!+		| y obj
	4 + swap over @ swap over - a!+ a!+	| obj
	a!
	;

:randobj
	rand 'screen p.cnt mod abs 'screen p.nro ;

:randtime
	rand 4.0 mod 8.0 + ;

:movt
	rand 1.0 mod rand 1.0 mod
    randobj
	randtime
	+move2d ;

:scat
	rand 0.8 mod 1.0 +
	randobj
	randtime
	+scale ;

:rot2
	rand 2.0 mod
	randobj
	randtime
	+rot2d ;

|---------------------------
:lrmove | adr --
	>a
	a@+ a@+
	a@+ a@+ 2swap
	( over <?
		pick2 over 4 + +!
		pick3 over 8 + +!
		64 + ) 4drop ;

:lrmove2d | dx dy o1 o2 --
	'lrmove 'ltime p!+ >a
 	a!+ | o2
	a!+ | o1
	a!+	| dy
	a!+	| dx
	;

| 1.0 1000

|---------------------------
:xypos
	rand sw mod abs 16 << rand sh mod abs 16 << ;
:vxypos
	rand 5.0 mod rand 5.0 mod ;

:add
	opensansregular 88 fontr!
	home
	cr cr cr
	$ff 'ink !
	+string

	cr sp

	droidsansbold 112 fontr!
	$ff00ff 'ink !
	" Que Tul " $ff +bstring
	;

:inicio
	;

|------- APP
:teclado
	key
	>esc< =? ( exit )
	<f1> =? ( "Esto es una prueba" add )
	<f2> =? ( "coso de prueba" +barra )
	<f3> =? ( scat )
	<f4> =? ( rot2 )

	<f10> =? ( reset )
	drop
	;

|------- SHOW
:show
	cls home
|	$ff00 'ink ! " r" print over .d print cr

	time.next
	'ltime p.draw
	'screen p.draw

	barradraw

	teclado
	;

|------- RAM
:ram
	mark
	1024 'scrbarra p.ini
	8192 'screen p.ini
	8192 'ltime p.ini
	time.ini
	;

|------- BOOT
:
	ram
	inicio
	'show onshow
;