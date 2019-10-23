| Sistema de Particulas
| PHREDA 2019

^r3/lib/sys.r3
^r3/lib/rand.r3
^r3/lib/print.r3

^r3/util/arr16.r3
^r3/util/polygr.r3

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

:reset
	'screen p.clear
	;

|---------------------------
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


:
|------- SHOW
:show
	cls home
	$ff00 'ink !
|	" r" print over .d print cr

	'screen p.draw

	key
	>esc< =? ( exit )
	<f1> =? ( "Esto es una prueba" add )
	<f2> =? ( reset )
	drop
	;

|------- RAM
:ram
	mark
	5000 'screen p.ini
	;

|------- BOOT
: ram
3
'show onshow
;