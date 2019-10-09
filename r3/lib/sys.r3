| SYSTEM
| PHREDA 2019
|----------------
^r3/lib/key.r3

#.exit 0

::onshow | 'word --
	0 '.exit !
	( .exit 0? drop
		update
		dup ex
		redraw ) 2drop
	0 '.exit ! ;

::exit
	1 '.exit ! ;

:wk
	key >esc< =? ( exit ) drop ;

::waitesc
	'wk onshow ;