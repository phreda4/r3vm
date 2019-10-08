| Example 5 DRAW in Canvas

^r3/lib/sys.r3
^r3/lib/key.r3
^r3/lib/str.r3
^r3/lib/print.r3

#xv 0

:coso
	cls home
	xv 5 >> dup videoshow
	xv 5 >> .d print cr
	xv 1 + $7ff and 'xv !
	;

:show
  key 
  >esc< =? ( exit )
  <f1> =? ( "video.mp4" 700 100 video )
  drop
  coso ;

:
|	"salud.mp4" 600 100 video
|	"video.mp4" 600 100 video
	"bigbuckbunny_480x272.h265" 600 100 video
	0 'paper !
	'show onshow
	|0 dup dup video
	;