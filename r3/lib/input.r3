^r3/lib/gui.r3
^r3/lib/btn.r3

|--- Edita linea
#cmax
#padi>	| inicio
#pad>	| cursor
#padf>	| fin

:lins  | c --
	padf> padi> - cmax >=? ( 2drop ; ) drop
	pad> dup 1 - padf> over - 1 + cmove> 1 'padf> +!
:lover | c --
	pad> c!+ dup 'pad> !
	padf> >? (
		dup padi> - cmax >=? ( swap 1 - swap -1 'pad> +! ) drop
		dup 'padf> ! ) drop
:0lin | --
	0 padf> c! ;
:kdel
	pad> padf> >=? ( drop ; ) drop
	1 'pad> +!
:kback
	pad> padi> <=? ( drop ; )
	dup 1 - swap padf> over - 1 + cmove -1 'padf> +! -1 'pad> +! ;
:kder
	pad> padf> <? ( 1 + ) 'pad> ! ;
:kizq
	pad> padi> >? ( 1 - ) 'pad> ! ;
:kup
	pad> ( padi> >?
		1 - dup c@ $ff and 32 <? ( drop 'pad> ! ; )
		drop ) 'pad> ! ;
:kdn
	pad> ( c@+ 1?
		$ff and 32 <? ( drop 'pad> ! ; )
		drop ) drop 1 - 'pad> ! ;

#modo 'lins

|----- tamaño de entrada
:makesizew | w --
	ccw * |dup 2/ +
	2 + 'w ! ccx w 2/ + 'xc !
	ccy cch dup 'h ! 2/ + 'yc !
|	gc2win
	;

:makesizehw | h w --
	ccw * |dup 2/ +
	2 + 'w ! ccx w 2/ + 'xc !
	cch * 'h ! ccy h 2/ + 'yc !
|	gc2win
	;

:makesizel | 'list -- 'list
	dup count makesizew ;

::scrsizew | w -- w h
	ccw * 2 + cch ;

::scrsizehw | h w -- w h
	ccw * 2 + swap cch * ;

|----- cursor
:drcursor
	ink >r
	modo 'lins =? ( verde )( rojo ) drop
	printcur drop
	r> 'ink ! ;

:cursori
	blink 1? ( drop ; ) drop
	padi> ( pad> =? ( drcursor ; ) c@+ 1?
		gemit ) 2drop ;

:cursorn
	blink 1? ( drop ; ) drop
	verde " " printcur drop ;

:cursorm
	blink 1? ( drop ; ) drop
	padi> ( pad> =? ( drcursor ; ) c@+ 1?
		allowcrx gemit ) 2drop ;

::pcursor | adr -- adr
	pad> <>? ( ; )
	blink 1? ( drop ; ) drop
	ccx 1- ccy 2dup op cch + pline
	dup c@ emitsize 1+
	ccx + ccy 2dup cch + op pline
	;

|----- ALFANUMERICO
:iniinput | 'var max 'dr IDF -- 'var max 'dr IDF
	pick2 'cmax !
	pick3 dup 'padi> !
	( c@+ 1? drop ) drop 1 - dup
	'pad> ! 'padf> !
	'lins 'modo ! ;

:proinputa | --
	ccx cursori 'ccx !
	[ key toasc modo ex ; ] <visible>
	[ modo 'lins =? ( 'lover )( 'lins ) 'modo ! drop  ; ] <ins>
	'kback <back>	'kdel <del>
	'kder <ri>		'kizq <le>
	[ padi> 'pad> ! ; ] <home>
	[ padf> 'pad> ! ; ] <end>
	'ktab dup <tab> <enter>
	'nextfoco <dn> 'prevfoco <up>
	;

|************************************
::inputa | 'var max w --
	gc.push makesizew
	'proinputa 'iniinput w/foco
	'clickfoco guiBtn
	drop ccx w + >r
	printx
	gc.pop r> 'ccx !
	;

|************************************
::input | 'var max  --
	dup inputa ;


|************************************
:proinputc | --
	ccx cursori 'ccx !
	[ key toasc modo ex ; ] <visible>
	[ modo 'lins =? ( 'lover )( 'lins ) 'modo ! drop  ; ] <ins>
	'kback <back>	'kdel <del>
	'kder <ri>		'kizq <le>
	[ padi> 'pad> ! ; ] <home>
	[ padf> 'pad> ! ; ] <end>
	'ktab <tab>
	;

::inputcell | 'var max --
	dup gc.push makesizew
	'proinputc 'iniinput w/foco
	'clickfoco guiBtn
	drop ccx w + >r
	printx
	gc.pop r> 'ccx !
	;

::inputcr | 'var max --
	'proinputc 'iniinput w/foco
	allowcr prin
	;

:proinputexe | --
	ccx cursori 'ccx !
	[ key toasc modo ex pick2 ex ; ] <visible>
	[ modo 'lins =? ( 'lover )( 'lins ) 'modo ! drop  ; ] <ins>
	[ kback pick2 ex ; ] <back>
	[ kdel pick2 ex ; ] <del>
	'kder <ri>		'kizq <le>
	[ padi> 'pad> ! ; ] <home>
	[ padf> 'pad> ! ; ] <end>
	'ktab dup <tab> <enter>
	'nextfoco <dn> 'prevfoco <up>
	;

|************************************
::inputexec | 'vector 'var max  --
	gc.push dup makesizew
	'proinputexe 'iniinput w/foco
	'clickfoco guiBtn
	drop ccx w + >r
	printx
	gc.pop r> 'ccx !
	drop
	;

