| NAVES GAME
| PHREDA 2018

^lib/sprite.r3

#nav4 $2008008 | pal-0 type-2 8x8
$........
$...1....
$...1....
$..111...
$..121...
$.11111..
$...5....
$....5...

#nav32 $008008 | 8x8 32bits
$. $. $. $ffff $. $. $. $.
$. $. $. $ffff $. $. $. $.
$. $. $ffff $ffff $ffff $. $. $.
$. $. $ffff $ffff $ffff $. $. $.
$. $ffff $ffff $ffff $ffff $ffff $. $.
$. $ffff $ff00 $ff00 $ffff $ffff $. $.
$. $. $. $. $. $. $. $.
$. $. $. $. $. $. $. $.

#lives
#pnts
#xn 100 #yn 100
#xv #yv

:player
	xn yn 'nav32 sprite
	xypen 'nav4 sprite
	;

:ongame
	cls
	player
	key 27 =? ( exit ) drop
	;

:
'ongame onshow
;
