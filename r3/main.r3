#salir 0

:main
	0 'salir !
	( salir 0? drop
		update
		key 27 =? ( dup 'salir ! ) drop
		$ffff vframe  !
		redraw
		) drop ;

: main ;