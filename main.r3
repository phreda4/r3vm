| main filesystem - PHREDA 2019
|
^r3/lib/print.r3
^r3/lib/sprite.r3
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
:FNAME | adr -- adrname
	44 + ;

:FDIR? | adr -- 1/0
	@ 4 >> 1 and ;

:FINFO | adr -- adr info
	dup FDIR? 0? ( 2 + ; ) drop 0 ;


:getname | nro -- ""
	2 << 'files + @ 8 >> 'filen + ;

:getinfo | nro -- info
	2 << 'files + @ $ff and ;

:getlvl | nro -- info
	2 << 'files + @ 4 >> $f and ;

:chginfo | nro --
	2 << 'files + dup @ $1 xor swap ! ;

|--------------------------------
:files.clear
	0 'nfiles !
	'filen 'filen> !
	'files dup 'files> ! 'files< !
	;

:files!+
	files> ( files< >?
		4 - dup @+ swap !
		) drop
	files< !+ 'files< !
	4 'files> +!
	;

:files.free
	0 'files ( files> <?
		@+ pick2 >? ( swap rot )
		drop ) drop
	8 >> 'filen +
	( c@+ 1? drop ) drop
	'filen> !
	;

:fileadd
	FINFO nivel 4 << or filen> 'filen - 8 << or
	files!+
	FNAME filen> strcpyl 'filen> !
	;

:reload
	'path 
	ffirst drop | quita .
	fnext drop	| quita ..
	( fnext 1? fileadd ) drop
	files> 'files - 2 >> 'nfiles !
	;

:rebuild
	"r3" 'path strcpy
	files.clear
	0 'pagina !
	0 'nivel !
	reload
	;

|-----------------------------
:makepath | actual nivel --
	0? ( drop
		"r3/" 'path strcpy
		getname 'path strcat
		; )
	over 1 -
	( dup getlvl pick2 >=?
		drop 1 - ) drop
	over 1 - makepath drop
	"/" 'path strcat
	getname 'path strcat
	;

:expande
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
	'path ( c@+ 1? drop ) drop 1-
	( 'path >?
		dup c@ $2f =? ( drop 0 swap c! remfiles ; )
		drop 1 - ) drop
	remfiles ;

|--------------------------------
:printfn | n
	dup getlvl 1 << nsp
	dup getinfo
	0? (  "+" print )
	1 =? ( "-" print )
	drop
	sp getname print sp
	;

#filecolor $af00 $ff00 $afaf00 $3f00  $7f7f  $7f007f $7f7f00 $7f0000 $7f00  $ff $ff

:colorfile
    dup getinfo $f and 2 << 'filecolor + @
	0 swap fontmcolor ;

:drawl | nro --
	colorfile
	actual =? ( fontminv printfn fontminv ; )
	printfn ;

:drawtree
	0 ( linesv <?
		dup pagina +
		nfiles  >=? ( 2drop ; )
|		'clicktree onLineClick
    	drawl
		cr 1 + ) drop ;

|--------------------------
:remlastpath
	'path ( c@+ 1? drop ) drop 1 -
	( dup c@ $2f <>? drop 1- ) drop
	0 swap c! ;

|--------------------------------
:runfile
	actual -? ( drop ; )
	getinfo $7 and
	2 <? ( drop ; )
	drop

	actual dup getlvl makepath
	actual getinfo $7 and 1? ( remlastpath ) drop

	mark
	"r3 " ,s 'path ,s "/" ,s ,s ,eol
|	"r3v " ,s 'path ,s "/" ,s ,s ,eol
	empty here
	sys drop
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
	drop
	runedit
	;

:enter
	actual getinfo $f and
	0? ( drop expande ; )
	8 =? ( drop contrae ; )
	drop
	runedit
	;

:enter2
|	'padin exer:
|	0 'padin !
|	refreshfoco
	;

#nfile

:newfile
	1 'nfile !
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
	getinfo $f and
	0? ( drop expande ; )
	1 =? ( drop contrae ; )
	drop

	actual
	getname
	".r3" =pos 1? ( drop runfile ; ) drop
	drop
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