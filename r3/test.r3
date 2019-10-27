^r3/lib/sys.r3
^r3/lib/math.r3
^r3/lib/str.r3
^r3/lib/mem.r3
^r3/lib/print.r3
^r3/lib/input.r3
^r3/lib/btn.r3

#k
#c


:teclado
	key
	>esc< =? ( exit )
	1? ( 'k ! ; )
	drop
	char
	1? ( 'c ! ; )
	drop
	;

#pad "hola" * 128

:main
	cls home gui
	cr
	"key: " print k .h print cr
	"char: " print c .h print cr

	'pad 127
	trace
	input

	teclado
	;

: 33 'main onshow ;
