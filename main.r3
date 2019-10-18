| main filesystem - PHREDA 2019
|
^r3/lib/sys.r3
^r3/lib/print.r3
^r3/lib/sprite.r3
^r3/lib/str.r3
^r3/lib/mem.r3

^r3/lib/fontm.r3
^r3/fntm/droidsans13.fnt

#path * 1024

#nfiles
#files * 8192
#files> 'files
#files< 'files
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

:FDIR? | adr -- 1/0
	@ $10 ;

:files.clear
	0 'nfiles !
	'filen 'filen> !
	'files 'files> !
	;

:fileadd

	FNAME
|   	".r3" =pos 0? ( 2drop ; ) drop
	filen>

	files> !+ 'files> !
	filen> strcpyl 'filen> !
	;

:reload
	'path ffirst drop | quita .
	fnext drop
	( fnext 1? fileadd ) drop
	files> 'files - 2 >> 'nfiles !
	;

:files.free
	0 'files ( files> <?
		@+ pick2 >? ( swap rot )
		drop ) drop
	8 >> 'filen +
	( c@+ 1? drop ) drop
	'filen> !
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

:getinfo | nro --
	2 << 'files + @ ;

:getname | nro -- ""
	2 << 'files + @ 8 >> 'filen + ;

:getinfo | nro -- info
	2 << 'files + @ $ff and ;

:getlvl | nro -- info
	2 << 'files + @ 4 >> $f and ;

:chginfo | nro --
	2 << 'files + dup @ $8 xor swap ! ;

|-----------------------------
:makepath | actual nivel --
|	0? ( drop getname "r4/%s" mprint 'path strcpy ; )
	over 1 -
|	( dup getlvl pick2 >=? )( drop 1- ) drop
	over 1 - makepath drop
	"/" 'path strcat
	getname 'path strcat
	;

:expande
|	0 'name !
	actual
	dup getlvl makepath

   	actual chginfo
	actual getlvl 1 + 'nivel !
    actual 1 + 2 << 'files + 'files< !
	reload
	;

:remfiles
	actual chginfo
	actual getlvl 1 +
	actual 1 +
	( dup getlvl pick2 >=?
		drop 1 + ) drop
	nip
	actual 1+
	( swap nfiles <?
		dup 2 << 'files + @
		pick2 2 << 'files + !
		1 + swap 1 + ) drop
	2 << 'files + 'files> !
	files> 'files - 2 >> 'nfiles !
	files.free
	;

:contrae
|	0 'name !
	'path ( c@+ 1? drop ) drop 1-
	( 'path >?
		dup c@ $2f =? ( drop 0 swap c! remfiles ; )
		drop 1 - ) drop
	remfiles ;

|--------------------------------
:invert
	$fff0 $0 fontmcolor ;

:normal
	$0 $ff00 fontmcolor ;

:printfn
	sp getfilename print sp ;

:drawl | nro --
	actual =? ( invert printfn normal ; )
	printfn ;

:drawtree
|    nfiles .d print " " print linesv .d print cr
	normal
	0 ( linesv <?
		dup pagina +
		nfiles  >=? ( 2drop ; )
|		'clicktree onLineClick
    	drawl
		cr 1 + ) drop ;

|--------------------------------

:runactual | "" --
	mark
	"r3 " ,s 'path ,s ,s ,eol
	empty here
	sys drop
	;

:runfile
	actual -? ( drop ; )
	getinfo $7 and
	0? ( drop ; )
|	setactual savem
|	1 >? ( nrun ; )
	drop
|	'name 'path "%s/%s" mprint run
	;

:runedit
|	ed.load
|	'name 'path "%s/%s" mprint 'ed.nombre =
|	0? ( 'name 'path "%s/%s" mprint 'ed.nombre strcpy 0 'ed.ncar ! 0 'ed.ipan ! ed.save )
|	drop
|	"r4/IDE/edit-code.txt" run
	;

:editfile
	actual -? ( drop ; )
	getinfo $7 and
	0? ( drop ; )
|	setactual savem
|	1 >? ( nrun ; )
	drop
	runedit
	;

:enter
	actual getinfo $f and
	0? ( drop expande ; )
	8 =? ( drop contrae ; )
	drop
|	setactual savem
	runedit
	;

:enter2
|	'padin exer:
|	0 'padin !
|	refreshfoco
	;

#nfile

:newfile
|	setactual
	1 'nfile !
|	0 'name !
	;

:remaddtxt | --
|	'name ".r3" =pos 1? ( drop ; ) drop
|	".r3" swap strcat
	;

:createfile
	0 'nfile !
	remaddtxt
|	savem
	mark
	"^r3/lib/gui.r3" ,ln ,cr
	":main" ,ln
	"	show clrscr" ,ln
	"	""Hello Human!"" print " ,ln
	"	'exit >esc<" ,ln
	"	cminiflecha ;" ,ln ,cr
	": main ;" ,ln
|	'name 'path "%s/%s" mprint savemem
	empty
	runedit
	;

|--------------------------------
:fenter
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

	<f1> =? ( runfile )
	<f2> =? ( editfile )
	<f3> =? ( newfile )

	drop ;

:header
	$888888 'ink !
	0 rows 1 - gotoxy backline
	$888888 $ffffff fontmcolor
	" /" print 'path print

	0 0 gotoxy backline
	$0 $ff00 fontmcolor
	" r3" print
	$0 $ff0000 fontmcolor
	"d4 " print

	0 1 gotoxy
	;

:inicio
	cls home
	header
	drawtree
	teclado
	acursor
	;


:main
	rebuild
	'fontdroidsans13 fontm
	rows 2 - 'linesv !
	'inicio onshow
	;

: main ;