|--MEM 8192
^r3/lib/sys.r3
^r3/lib/mem.r3
^r3/lib/print.r3
^r3/lib/sprite.r3
^r3/lib/loadimg.r3

#ima

:teclado
	key >esc< =? ( exit ) drop 	;

:main
	cls home
	xypen ima sprite
	teclado
	;

:
	mark
	"test.png" loadimg 'ima !
	'main onshow ;
