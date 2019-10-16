^r3/lib/sys.r3
^r3/lib/math.r3
^r3/lib/str.r3
^r3/lib/print.r3

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

:main
	cls home
	cr
	k .h print cr
	c .h print cr
	teclado
	;

:
	33 'main onshow ;
