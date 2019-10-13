^r3/lib/sys.r3
^r3/lib/math.r3
^r3/lib/str.r3
^r3/lib/print.r3

:teclado
	key
	>esc< =? ( exit )
	drop ;

:main
	cls home
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
