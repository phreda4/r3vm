| edit-code
| PHREDA 2007
|---------------------------------------
^r3/lib/math.r3
^r3/lib/mem.r3
^r3/lib/print.r3
|^r3/lib/btn.r3
|^r3/lib/input.r3
|^r3/lib/parse.r3
|^r3/lib/system.r3
|^r3/lib/codecolor.r3

|^r4/system/mem-ed.r3



#ed.nombre
#ed.ncar
#ed.ipan

#pantaini>	| comienzo de pantalla
#pantafin>	| fin de pantalla
#prilinea	| primera linea visible
#cntlinea

#inisel		| inicio seleccion
#finsel		| fin seleccion

#fuente  	| fuente editable
#fuente> 	| cursor
#$fuente	| fin de texto

#clipboard	|'clipboard
#clipboard>

#undobuffer |'undobuffer
#undobuffer>

|----- find text
#findpad * 64
#findmode

|---- arreglo de info
|#errorinfo
#errorlin
#errormsg * 256


|----- edicion
:lins  | c --
	fuente> dup 1 - $fuente over - 1 + cmove>
	1 '$fuente +!
:lover | c --
	fuente> c!+ dup 'fuente> !
	$fuente >? ( dup '$fuente ! ) drop
:0lin | --
	0 $fuente c! ;

#modo 'lins

:back
	fuente> fuente <=? ( drop ; )
	dup 1 - c@ undobuffer> c!+ 'undobuffer> !
	dup 1 - swap $fuente over - 1 + cmove
	-1 '$fuente +!
	-1 'fuente> +! ;

:del
	fuente>	$fuente >=? ( drop ; )
    1 + fuente <=? ( drop ; )
	9 over 1 - c@ undobuffer> c!+ c!+ 'undobuffer> !
	dup 1 - swap $fuente over - 1 + cmove
	-1 '$fuente +! ;

:<<13 | a -- a
	( fuente >=?
		 dup c@
		13 =? ( drop ; )
		drop 1 - ) ;

:>>13 | a -- a
	( $fuente <?
		 dup c@
		13 =? ( drop 1 - ; ) | quitar el 1 -
		drop 1 + )
	drop $fuente 2 - ;

#1sel #2sel

#mshift	|******************

:selecc	| agrega a la seleccion
	mshift 0? ( dup 'inisel ! 'finsel ! ; ) drop
	inisel 0? ( fuente> '1sel ! ) drop
	fuente> dup '2sel !
	1sel over <? ( swap )
	'finsel ! 'inisel !
	;

:khome
	selecc
	fuente> 1 - <<13 1 + 'fuente> !
	selecc ;

:kend
	selecc
	fuente> >>13  1 + 'fuente> !
	selecc ;

:scrollup | 'fuente -- 'fuente
	pantaini> 1 - <<13 1 - <<13  1 + 'pantaini> !
	prilinea 1? ( 1 - ) 'prilinea !
	selecc ;

:scrolldw
	pantaini> >>13 2 + 'pantaini> !
	pantafin> >>13 2 + 'pantafin> !
	1 'prilinea +!
	selecc ;

:colcur
	fuente> 1 - <<13 swap - ;

:karriba
	fuente> fuente =? ( drop ; )
	selecc
	dup 1 - <<13		| cur inili
	swap over - swap	| cnt cur
	dup 1 - <<13			| cnt cur cura
	swap over - 		| cnt cura cur-cura
	rot min + fuente max
	'fuente> !
	selecc ;

:kabajo
	fuente> $fuente >=? ( drop ; )
	selecc
	dup 1 - <<13 | cur inilinea
	over swap - swap | cnt cursor
	>>13 1 +    | cnt cura
	dup 1 + >>13 1 + 	| cnt cura curb
	over -
	rot min +
	'fuente> !
	selecc ;

:kder
	selecc
	fuente> $fuente <?
	( 1 + 'fuente> ! selecc ; ) drop
	;

:kizq
	selecc
	fuente> fuente >?
	( 1 - 'fuente> ! selecc ; ) drop
	;

:kpgup
	selecc
	20 ( 1?
		1 - karriba ) drop
	selecc ;

:kpgdn
	selecc
	20 ( 1?
		1 - kabajo ) drop
	selecc ;

|---------------   mover fuente
:e.clear
	fuente dup 'pantaini> ! dup 'fuente> ! '$fuente !
	0lin ;

|------------------------------------------------
:ajpri
	0
	fuente ( pantaini> <?
		c@+ 13 =? ( rot 1 + rot rot )
		drop ) drop
	'prilinea ! ;

:loadtxt | -- cargar texto
	mark
	here 'ed.nombre
	getpath
	load 0 swap c!

	|-- queda solo cr al fin de linea
	fuente dup 'pantaini> !
	here ( c@+ 1?
		13 =? ( over c@ 
				10 =? ( rot 1 + rot rot ) drop )
		10 =? ( drop c@+ 13 <>? ( drop 1 - 13 ) )
		rot c!+ swap ) 2drop '$fuente !
	0lin

	|-- ubicar el cursor
	errorlin
	-? ( drop ed.ncar 'fuente> +! ed.ipan 'pantaini> +! ajpri empty ; )
	fuente ( swap 1?
		1 - swap >>13 2 + )
	drop 'fuente> !
	ed.ncar +? ( dup 'fuente> +! ) drop
	empty ;

:ed.save
	;

:savetxt
	mark	| guarda texto
	fuente ( c@+ 1?
		13 =? ( ,c 10 ) ,c ) 2drop
	'ed.nombre savemem
	empty
	0 dup "debug.err" save
	0 dup "runtime.err" save
	fuente> fuente - 'ed.ncar !
	pantaini> fuente - 'ed.ipan !
	ed.save
	;

|-------------------------------------------
:copysel
	inisel 0? ( drop ; )
	clipboard swap
	finsel over - pick2 over + 'clipboard> !
	cmove
	;

:borrasel
	inisel finsel $fuente finsel - 4 + cmove
	finsel inisel - neg '$fuente +!

|	fuente>
|	inisel >=? ( finsel <=? ( inisel 'fuente> ! )( finsel inisel - over swap - 'fuente> ! ) )  drop

	0 dup 'inisel ! 'finsel ! ;

:kdel
	inisel 0? ( drop del ; )
	drop borrasel ;

:kback
	inisel 0? ( drop back ; )
	drop borrasel ;

|-------------
| Edit CtrE
|-------------
:posfijo? | adr -- desde/0
	( c@+ 1?
		46 =? ( drop ; )
		drop )
	nip ;

:editvalid | adr -- adr 1/0
    "ico" =pre 1? ( ; ) drop
    "bmr" =pre 1? ( ; ) drop
    "vsp" =pre 1? ( ; ) drop
    "spr" =pre
	;

:controle
	ed.save
	savetxt
	fuente> ( dup 1 - c@ $ff and 32 >? drop 1 - ) drop | busca comienzo
	dup c@
	$5E <>? ( 2drop ; ) | no es ^
	drop
	dup fuente - 'ed.ncar !
	dup 2 + posfijo? 0? ( 2drop ; )
	editvalid 0? ( 3drop ; ) drop
	swap 1 + | ext name
	mark
	dup c@ 46 =? ( swap 2 + 'path ,s ) drop
	,w
|	dup "mem/inc-%w.mem" mprint savemem
	empty
|	"r4/system/inc-%w.txt" mprint run
	drop
	;

|-------------
:controlc | copy
	copysel ;

|-------------
:controlx | move
	controlc
	borrasel ;

|-------------
:controlv | paste
	clipboard clipboard> over - 0? ( 3drop ; ) | clip cnt
	fuente> dup pick2  + swap | clip cnt 'f+ 'f
	$fuente over - 1 + cmove>	| clip cnt
	fuente> rot rot | f clip cnt
	dup '$fuente +!
	cmove
	clipboard> clipboard - 'fuente> +!
	;

|-------------
:controlz | undo
	undobuffer>
	undobuffer =? ( drop ; )
	1 - dup c@
	9 =? ( drop 1 - dup c@ [ -1 'fuente> +! ; ] >r )
	lins 'undobuffer> ! ;

|-------------
:=w | w1 w2 -- 1/0
	( c@+ 32 >?
		toupp rot c@+ toupp rot - 1? ( 3drop 0 ; ) drop swap ) 3drop 1 ;

:exactw | adr 1c act cc -- adr 1c act 1/0
	drop dup pick3 =w ;

:setcur | adr 1c act 1
	drop nip nip 'fuente> ! ;

:controla | -- ;find prev
	'findpad
	dup c@ $ff and
	0? ( 2drop ; )
	toupp
	fuente> 1 - | adr 1c act
	( fuente >?
		dup c@ toupp pick2 =? ( exactw 1? ( setcur ; ) )
		drop 1 - ) 3drop ;

:controls | -- ;find next
	'findpad
	dup c@ $ff and
	0? ( drop ; )
	toupp
	fuente> 1 + | adr 1c act
	( $fuente <?
		dup c@ toupp pick2 =? ( exactw 1? ( setcur ; ) )
		drop 1 + ) 3drop ;

:controlf | -- ;find
	findmode 1 xor 'findmode !
|	0 update dro
|	refreshfoco
	;

|-------------
:controld | buscar definicion
	;

:controln  | new
	| si no existe esta definicion
	| ir justo antes de la definicion
	| agregar palabra :new  ;
	;

|------ Dibuja codigo
:drawcur | com -- com
	blink 1? ( drop ; ) drop
|	fuente> >? ( ; )
|	dup	( fuente> <?  c@+ 13 =? ( 2drop ; ) gemit )
|	modo 'lins =? ( $ffffff )( $ffff00 ) 'ink ! drop
|	printcur drop
	;

:drawsel | com -- com
	inisel 0? ( drop ; ) drop
|	finsel >=? ( ; )
|	dup ( inisel <? c@+ 13 =? ( 2drop ; ) gemit )
|	sel>> $88 'ink !
|	( finsel <? c@+ 13 =? ( 2drop sel<< ; ) gemit )
|	drop sel<<
	;

:drawcode
	pantaini>
|	sw 'tx2 !
	0 ( cntlinea <?
		$888888 'ink !
		dup prilinea + "%d" print
       	ccw 2 << 'ccx !
		swap
		drawsel lf drawcur lf
|		>>lineacolor0
		lprint

|		0 'tx1 !
|		0? ( 2drop cntlinea $fuente )( cr )
		swap 1 + ) drop
	$fuente <? ( 1 - ) 'pantafin> !
	fuente>
	( pantafin> >? scrolldw )
	( pantaini> <? scrollup )
	drop
	|----------------- mensaje de error!!
	errorlin -? ( drop ; )
	prilinea <? ( drop ; )
	prilinea - cntlinea >=? ( drop ; )
|	3 swap gotoxy
|	">" $ff0000 'ink ! fillp $ffffff 'ink ! print

|	'errormsg "< %s " $ff0000 'ink ! fillpr $ffffff 'ink ! printr
	;


|-------------- panel control
#panelcontrol

:controlon	1 'panelcontrol ! ;
:controloff 0 'panelcontrol ! ;

:showclip
	cr
 	clipboard> clipboard <>? (
		$ffff 'ink !
		cr
		clipboard ( over <? c@+ emit ) drop
		cr
		) drop
	;

|--- sobre pantalla
:mmemit | adr x xa -- adr x xa
	rot c@+
	13 =? ( 0 nip )
	0? ( drop 1 - rot rot sw + ; )
	9 =? ( drop swap $ffffffe0 and $20 + rot swap ; )
	drop swap ccw + rot swap ;

:cursormouse
	xypen
	pantaini>
	swap cch 2*			| x adr y ya
	( over <?
		cch + rot >>13 2 + rot rot ) 2drop
	swap ccw 2 << ccw +	| adr x xa
	( over <? mmemit ) 2drop
	'fuente> ! ;

:dns
	cursormouse
	fuente> '1sel ! ;

:mos
	cursormouse
	fuente> 1sel over <? ( swap )
	'finsel ! 'inisel ! ;

:ups
	cursormouse
	fuente> 1sel over <? ( swap )
	'finsel ! 'inisel ! ;

|----------------------------
:directrun	
	|savetxt 'ed.nombre run  ;
:profiler	
|	savetxt	"r4/IDE/profiler-code.txt" run ;
:debugrun	
|	savetxt	"r4/IDE/debug-code.txt" run ;
:mkplain	
|	savetxt	"r4/system/r4plain.txt" run 
;

|----------------------------
:nowcompile
| opciones de compilador
| fullscreen,ventana wxh
| win
| optimizacion 0
|	"r4/compiler/r4-com4.txt" run
	;

:testcompile
|	"r4/compiler/r4-com4o.txt" run
	;


|---------- manejo de teclado
:buscapad
	fuente 'findpad findstri
	0? ( drop ; ) 'fuente> ! ;

:findmodekey
	$0 'ink ! 
	|1 linesfill
	" > " print
	
	key
    <ret> =? ( controlf )
	>esc< =? ( controlf )
    drop

	$ffffff 'ink !
|	'buscapad 'findpad 31 inputexec
	;

:controlkey
	$337ab7 'ink !
	|1 linesfill
	$ffffff 'ink !
|	'controle 18 ?key " E-Edit" print | ctrl-E dit
||	'controlh 35 ?key " H-Help" print  | ctrl-H elp
|	'controlz 44 ?key " Z-Undo" print

|	'controlx 45 ?key " X-Cut" print
|	'controlc 46 ?key " C-Copy" print
|	'controlv 47 ?key " V-Paste" print

||	'controld 32 ?key " D-Def" print

||	'controln 49 ?key " N-New" print
||	'controlm 50 ?key " M-Mode" print
|	'controlf 33 ?key " F-Find" print

|	'controla <up>
|	'controls <dn>
|	'controloff >ctrl<

|	'findpad
|	dup c@ 0? ( 2drop ; ) drop
|	" (%s)" print
	;

:teclado
	key
	<char char> bt? ( toasc modo ex ; )
	<back> =? ( kback )
	<del> =? ( kdel )
	<up> =? ( karriba )
	<dn> =? ( kabajo )
	<ri> =? ( kder )
	<le> =? ( kizq )
	<home> =? ( khome )
	<end> =? ( kend )
	<pgup> =? ( kpgup )
	<pgdn> =? ( kpgdn )
	<ins> =? (  modo 
				'lins =? ( drop 'lover 'modo ! ; )
				drop 'lins 'modo ! )
	<ret> =? (  13 modo ex )
	<tab> =? (  9 modo ex )
	>esc< =? ( exit )
	<ctrl> =? ( controlon )
	drop
	;

:barraestado
	panelcontrol 1? ( drop controlkey ; ) drop
	findmode 1? ( drop findmodekey ; ) drop
	$666666 'ink !
	|1 linesfill
	$ffffff 'ink !
	'ed.nombre sp print sp
	teclado
	;

:barv
	$333333 'ink !
	0 0 op 0 sh pline ccw 2 << 0 op ccw 2 << sh pline
	poli ;

:editando
	cls
|	'dns 'mos 'ups guiMap |------ mouse

	barv

	0 rows 1 - gotoxy
	barraestado

	0 0 gotoxy
	$666666 'ink ! 
	|1 linesfill
	$ff00 'ink !
	":R3" print
	$ffffff 'ink !
	"eDIT " print

|------------------------------
|	'directrun dup <f1> "1Run" $fff37b flink sp
|	'debugrun dup <f2> "2Debug" $fff37b flink sp
|	'profiler dup <f3> "3Profile" $fff37b flink sp
|	'mkplain dup <f4> "4Plain" $fff37b flink sp
|	'nowcompile dup <f5> "5Compile" $fff37b flink sp

|	'testcompile <f10>
|------------------------------
	cr
	drawcode

|	cminiflecha
	;

:editor
	$111111 'paper !
	cls
	rows 2 - 'cntlinea !
	'editando onshow
	;

|---- Mantiene estado del editor
:ram
	here	| --- RAM
	dup 'fuente !
	dup 'fuente> !
	dup '$fuente !
	$3ffff +			| 256kb texto
	dup 'clipboard !
	dup 'clipboard> !
	$3fff +				| 16KB
	dup 'undobuffer !
	dup 'undobuffer> !
	$ffff +         	| 64kb
	'here  ! | -- FREE
	mark
	;


:cargaestado
	mark
	-1 'errorlin !
	0 'ed.ncar !
|	0 'errorinfo !

	here dup "debug.err" load
	empty
|	over 4 + <? ( 2drop ed.load ; )
	0 swap !
|	'ed.nombre cpy|
|	?sint 'errorlin !
|	?sint 'ed.ncar !
	'errormsg strcpy

|	here dup "info.err" load
|	over 4 + <? ( 2drop ; ) | ini end
|	dup 'errorinfo !
	;

|----------- principal
:main
	ram
|	cargaestado
|	loadtxt
	editor

|	savetxt
	;

: mark 4 main ;
