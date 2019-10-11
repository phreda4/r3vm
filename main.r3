| main filesystem - PHREDA 2019
|
^r3/lib/sys.r3
^r3/lib/print.r3
^r3/lib/str.r3
^r3/lib/mem.r3
^r3/lib/penner.r3

^r3/lib/fontr.r3
^r3/rft/robotoregular.rft

^r3/lib/fontm.r3
^r3/fntm/droidsans13.fnt

#path * 1024

#nfiles
#files * 8192
#files> 'files
#filen * $3fff
#filen> 'filen

#nivel
#pagina
#actual
#linesv 15

|--------------------------------
| win
:FNAME | adr -- adrname
	44 + ;

:files.clear
	0 'nfiles !
	'filen 'filen> !
	'files 'files> !
	;

:reload
	'path ffirst drop | quita .
	fnext drop
	( fnext 1?
		filen> files> !+ 'files> !
		FNAME filen> strcpyl 'filen> !
		) drop
	files> 'files - 2 >> 'nfiles !
	;

:rebuild
	"r3/" 'path strcpy
	files.clear
	0 'pagina !
	0 'nivel !
	reload
	;

:getfilename | nro --
	2 << 'files + @ ;

|--------------------------------
:invert
	$fff0 $0 fontmcolor ;

:normal
	$0 $ff00 fontmcolor ;

:drawl | nro --
	actual =? ( invert getfilename print normal ; )
	getfilename print ;

:drawtree
|    nfiles .d print " " print linesv .d print cr
	normal
	0 ( linesv <?
		dup pagina +
		nfiles  >=? ( 2drop ; )
|		'clicktree onLineClick
    	drawl
		cr 1 + ) drop ;

:runactual | "" --
	drop
	"r3 r3/  pattern-XOR.r3" sys
	;
|--------------------------------
:fenter
	"" runactual drop ;
:a
	actual
	getfilename
	".r3" =pos 1? ( drop runactual ; ) drop
	drop

|	0? ( drop expande ; )
|	8 =? ( drop contrae ; )
|	drop
|	setactual savem
|	runedit
	;

:fdn
	actual nfiles 1 - >=? ( drop ; )
	1 + pagina linesv + 1 - >=? ( dup linesv - 1 + 'pagina ! )
	'actual !
	;

:fup
	actual 0? ( drop ; )
	1 - pagina <? ( dup 'pagina ! )
	'actual !
	;

:fpgdn
	actual nfiles 1 - >=? ( drop ; )
	20 + nfiles >? ( drop nfiles 1 - ) 'actual !
	actual pagina linesv + 1 -
	>=? ( dup linesv - 1 + 'pagina ! ) drop
	;

:fpgup
	actual 0? ( drop ; )
	20 - 0 <? ( drop 0 )
	pagina <? ( dup 'pagina ! )
	'actual !
	;

:fhome
	actual 0? ( drop ; ) drop
	0 'actual ! 0 'pagina !
	;

:fend
	actual nfiles 1- >=? ( drop ; ) drop
	nfiles 1 - 'actual !
	actual 1 + pagina linesv + 1 -
	>=? ( dup linesv - 1 + 'pagina ! ) drop
	;


:teclado
	key
	>esc< =? ( exit )
	<up> =? ( fup )
	<dn> =? ( fdn )
	<pgup> =? ( fpgup )
	<pgdn> =? ( fpgdn )
	<home> =? ( fhome )
	<end> =? ( fend )
	<ret> =? ( fenter )
	drop ;


:inicio
	cls home
|	robotoregular 48 fontr!
	'fontdroidsans13 fontm
	$ff00 'ink !
	drawtree
	teclado
	;


:main
	rebuild
	cls home
	'inicio onshow
	;

: main ;