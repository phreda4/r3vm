|--MEM 8192
^r3/lib/sys.r3
^r3/lib/math.r3
^r3/lib/str.r3
^r3/lib/print.r3
^r3/lib/sprite.r3
^r3/lib/loadimg.r3

|----------------------------------------
#ima

:teclado
	key
	>esc< =? ( exit )
	drop
	;

:dumpmem
	0? ( drop ; ) >b
	16 ( 1? 1 -
		8 ( 1? 1 -
    		b@+ $ffffffff and .h print sp
    		) drop
    	cr ) drop ;

:main
	cls home
	0 0 ima sprite
	ima dumpmem
	teclado
	;

:
	mark
	"test.png" loadimg 'ima !
	'main onshow ;
