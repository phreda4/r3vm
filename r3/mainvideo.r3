| Example 5 DRAW in Canvas

^lib/sys.r3
^lib/str.r3
^lib/print.r3

##seed 495090497

::rand | -- r32
  seed 3141592621 * 1 + dup 'seed ! ;

#xv 0

:coso
	cls home
	xv 5 >> dup videoshow
	rand .d print cr
	xv 5 >> .d print cr
	xv 1 + $7ff and 'xv !

	;

:show
  key 27 =? ( exit ) drop
  coso ;

:
"salud.mp4" 600 100 video
0 'paper !
'show onshow
0 dup dup video
;