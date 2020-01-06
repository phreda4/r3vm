| zbuffer - convex polyfill
| PHREDA 2020
|-------------------------
##zb
##zbw ##zbh
#zcnt ##zbo

|--- maskbuffer | zbw*zbh
::zb.adr | x y -- d
	zbw * + 2 << zb + ;

::zb@ | x y -- b
	zb.adr @ ;

::zbo! | o z x y --
	zb.adr !+ zbo + 4 - ! ;

::zb.clear
	zb >a zcnt ( 1? 1 - $7fffffff a!+ ) drop ;
|	zb $7fffffff zcnt fill ;

::zb.ini | w h --
	2dup * dup 'zcnt !	| w h cnt
	dup 2 << 'zbo !
	here dup 'zb !			| w h cnt here
	swap 3 << +	'here !	| w h
	'zbh ! 'zbw !		|
	zb.clear
	;

