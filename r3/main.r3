#salir 0

#memoria 0

:llenado
	vframe 'memoria !
	sw sh * ( 1? 1 -
		dup memoria !
		4 'memoria +!
		) drop ;

:main
	0 'salir !
	( salir 0? drop
		update
		key 27 =? ( dup 'salir ! ) drop

		llenado

		redraw
		) drop ;

: main ;