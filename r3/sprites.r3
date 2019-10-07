| NAVES GAME
| PHREDA 2018

^r3/lib/sprite.r3
^r3/lib/str.r3
^r3/lib/print.r3
^r3/lib/key.r3


#nav32 $008008 | 8x8 32bits
$. $. $. $ff $. $. $. $.
$. $. $. $ff $. $. $. $.
$. $. $ff $ff $ff0000 $. $. $.
$. $. $ff $ff $ff0000 $. $. $.
$. $ff $ff $ffff $ff0000 $ffff $. $.
$. $ff $ff $ffff $ff0000 $ffff $. $.
$. $. $. $. $. $. $. $.
$ff00 $ff00 $ff00 $ff00 $. $. $. $.

#xn 100 #yn 100

:player
	xn yn 'nav32 sprite
	;

#val

:ongame
	cls home
	xypen .d print " " print .d print cr
	val .h print cr

	key
	1? ( dup 'val ! )
	<up> =? ( -1 'yn +! )
	<dn> =? ( 1 'yn +! )
	<lef> =? ( -1 'xn +! )
	<rig> =? ( 1 'xn +! )

	>esc< =? ( exit )
	drop
	player
	;
:
'ongame onshow
;

