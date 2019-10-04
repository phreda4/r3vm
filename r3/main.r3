^lib/str.r3
^lib/print.r3

#salir 0
#val

:llenado
	0 'paper !
	cls home
	$ff0000 'color !
	"Hello Word!" print cr
	$00ff 'color !
	"Hola Mundo!" print cr
	$ffff 'color !
	"r3" print cr
	$ffffff 'color !
	val .d print cr
	1 'val +!
	;


:main
	0 'salir !
	( salir 0? drop
		update
		key 27 =? ( dup 'salir ! ) drop
		llenado
		redraw
		) drop ;

:m2
	0 'paper !
	'llenado onshow ;

: main ;