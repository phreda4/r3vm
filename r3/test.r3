^r3/lib/sys.r3
^r3/lib/math.r3
^r3/lib/str.r3
^r3/lib/print.r3

#k

:teclado
	key
	>esc< =? ( exit )
	1? ( 'k ! ; )
	drop ;

:main
	cls home
	k .h print cr
	over .d print cr
	10
	8 11 bt? ( "ok " print )
	11 23 bt? ( "no" print )
	-4 9 bt? ( "no" print )
	drop
	teclado
	;

:
	33 'main onshow ;
