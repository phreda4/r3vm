
^r3/lib/sys.r3
^r3/lib/math.r3
^r3/lib/str.r3
^r3/lib/print.r3

^r3/lib/fontr.r3
^r3/rft/robotoregular.rft

#car 0
#tablelist 0

:v>rfw ccw 14 *>> ;
:rf>xy | value -- x y
	dup 18 >> ccw 14 *>> ccx +
	swap 46 << 50 >> cch 14 *>> ccy +
	;

|--------- formato fuente
#yp #xp

:a0 drop ; 									| el valor no puede ser 0
:a1 xp yp pline rf>xy 2dup 'yp !+ ! op ;  | punto
:a2 rf>xy pline ; | linea
:a3 swap >b rf>xy b@+ rf>xy pcurve b> ;  | curva
:a4 swap >b rf>xy b@+ rf>xy b@+ rf>xy pcurve3 b> ; | curva3
|---- accediendo a x e y
:a5 rf>xy opx swap pline opy pline ;
:a6 rf>xy opy pline opx swap pline ;

#gfont a0 a1 a2 a3 a4 a5 a6 0 0 0 0 0 0 0 0 0

:drawrf | 'rf --
	@+ rf>xy 2dup 'yp !+ ! op
	( @+ 1?
		dup $f and 2 << 'gfont + @ ex
		) 2drop
	xp yp pline
	poli
	;

:ptok
	$ffffffff and .h print sp ;
:a
	dup $f and .h print sp 
	dup 18 >> .d print sp 
	46 << 50 >> .d print sp ;  

:g>xy
	dup 18 >>
	swap 46 << 50 >>
	;


#auxf * 1024

:optyy | adr+1 val -- adr+1
	
	;
:optxx | adr+1 val -- adr+1
	;

:optpp | adr+1 val -- adr+1 ; es punto punto
	over @ g>xy 'y2 ! 'x2 !
	dup g>xy
	y2 =? ( 2drop optyy ; ) drop
	x2 =? ( drop optxx ; ) drop
	a!+  
	;

:optp | adr+1 val -- adr+1| es punto
	over @ $f and
	2 =? ( drop optpp ; )
	drop
	a!+  
	;

:punto | adr+1 val -- adr+1
	dup $f and 
	2 =? ( drop optp ; )
	drop	
	a!+ ;

:convert 
	'auxf >a
	( @+ 1? punto ) a!+ drop ;
	 	

:coso
	cls home
	robotoregular 28 fontr!
	$ff0000 'ink !
	"R3d4 " print 
	car .d print cr
	$ff00 'ink !

	car 2 << tablelist + @
	( @+ 1? ptok ) 2drop cr 

	car 2 << tablelist + @ convert
	'auxf
	( @+ 1? ptok ) 2drop cr
	

	sw 2/ 'ccx ! sh 2/ 'ccy !
	200 'ccw ! 200 'cch !
	$ff 'ink !
	car 2 << tablelist + @
	drawrf

	;

:teclado
	key
	>esc< =? ( exit )
	<up> =? ( 1 'car +! )
	<dn> =? ( -1 'car +! )
	drop ;

:show
	teclado
	coso ;

:
	robotoregular 'tablelist !
	4drop
	33 'car !
	'show onshow
	;