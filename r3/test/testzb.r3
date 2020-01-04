|MEM 65536 | 64MB
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
			|$7fffffff <>? ( drop b> zbo + @ )
			$7fffffff =? ( 0 nip )
			a!+
			) drop
		) drop ;

:ztestc
	0 0 xy>v >a
	zb >b
	sh ( 1? 1 -
		sw ( 1? 1 -
			b@+
			$7fffffff <>? ( drop b> zbo + @ )
|			$7fffffff =? ( 0 nip )
			a!+
			) drop
		) drop ;


:tz
	zb.clear

	50 60 $f00 zop
	100 380 $1ff zline
	xypen $007f zline
	50 60 $f00 zline
	$ff00 zo! zpoly

	50 60 $f00 zop
	xypen $007f zline
	300 200 $3ff zline
	50 60 $f00 zline
	$ff0000 zo! zpoly

	100 70 $fe zop
	30 200 $fff zline
	90 300 $fff zline
	100 70 $fe zline
	$ff zo! zpoly

	100 70 $fe zop 90 300 $fff zline 300 80 $fff zline 100 70 $fe zline $ff0000 zo! zpoly

	$ff0000 zo!
	400 100 250 200 zbh!
	401 100 250 200 zbh!

	$ff zo!
	300 200 150 200 zbw!
	300 201 150 200 zbw!

	$ff00ff zo!
	$fff zz!
	40 40 501 201 zb.fill

	$ffff zo!
	$1fff zz!
	40 40 521 221 zb.fill

	ztest ;

:tzunion
	zb.clear

	100 140 $fe zop
	30 200 $fff zline
	xypen $fff zline
	100 140 $fe zline
	$ff zo! zpoly

	400 380 $fe zop
	30 200 $fff zline
	xypen $fff zline
	400 380 $fe zline
	$ff00 zo! zpoly

	100 140 $fe zop
	500 50 $fff zline
	xypen $fff zline
	100 140 $fe zline
	$ff0000 zo! zpoly

	400 380 $fe zop
	500 50 $fff zline
	xypen $fff zline
	400 380 $fe zline
	$ff00ff zo! zpoly

	ztestc ;

|----------------------
#x 500 #y 200

:testcover
	30 30 x y $ffff zb.occ  "%d " print cr
	$ff 'ink !
	x y op x 30 + y line x 30 + y 30 + line x y 30 + line x y line
	key
	<ri> =? ( 1 'x +! )
	<le> =? ( -1 'x +! )
	<up> =? ( -1 'y +! )
	<dn> =? ( 1 'y +! )
	drop
	;

:screen
	cls home
|    	tz
|        testcover
|    tzunion
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