^r3/lib/sys.r3
^r3/lib/math.r3
^r3/lib/str.r3
^r3/lib/mem.r3
^r3/lib/print.r3

#k
#c

#test 1 2 3 4 5 6 7 8 9 0

:teclado
	key
	>esc< =? ( exit )
	1? ( 'k ! ; )
	drop
	char
	1? ( 'c ! ; )
	drop
	;

#sx -1.5 | Desplazamiento de la imagen
#sy -1.0
#tw 2.0 | Escala
#th 2.0

:main
	cls home
	cr
	k .h print cr
	c .h print cr
	mark
	sx tw *. sy " %f %f " ,format
	empty here print cr

	mark
	tw th " %f %f " ,format
	empty here print cr

	15.0 2.0 16 <</	.f print cr
	'test 10 ( 1? 1 - swap @+ .d print sp swap ) 2drop

	teclado
	;

:
	'test 33 5 fill
	33 'main onshow ;
