|MEM 32768
| test zbuffer
| PHREDA 2020

^r3/lib/gui.r3
^r3/lib/zbuffer.r3

:ztest
	0 0 xy>v >a
	zb >b
	sh ( 1? 1 -
		sw ( 1? 1 -
			b@+
|			$7fffffff <>? ( drop b> zbo + @ )
			$7fffffff =? ( 0 nip )
			a!+
			) drop
		) drop ;

:tz
	zb.clear
	
	30 ( 1? 1 -
		30 ( 1? 1 -
			$ff00 pick2 70 + pick2 70 + 2dup 1 >> + rot rot zbo!
			) drop
		) drop
	
	30 ( 1? 1 -
		30 ( 1? 1 -
			$ff pick2 90 + pick2 90 + 2dup 1 << + rot rot zbo!
			) drop
		) drop


	ztest ;


:screen
   	tz
    home
    $ffffff 'ink !
	over "%d" print cr
	key
	>esc< =? ( exit )
	drop
	;

|----------------------
:main
	33
	mark
	sw sh zb.ini
	'screen onshow ;

: main ;