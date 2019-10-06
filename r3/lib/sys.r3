| SYSTEM
| PHREDA 2019
|----------------

#.exit 0

::onshow | 'word --
	0 '.exit !
	0 ( drop
		update
		dup ex
		redraw
		.exit 0? )
	2drop
	0 '.exit !
	;

::exit
	1 '.exit ! ;