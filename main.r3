^r3/lib/print.r3
^r3/lib/str.r3
^r3/lib/key.r3
^r3/lib/mem.r3

#path * 1024

#nfiles
#files * 8192
#files> 'files
#filen * $3fff
#filen> 'filen

#nivel
#pagina
#linesv 10

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

:drawl | nro --
	getfilename print
	;

:drawtree
    nfiles .d print " " print linesv .d print cr
	0 ( linesv <?
		dup pagina +
		dup .d print " " print
|		nfiles 	>=? ( 2drop ; )
|		'clicktree onLineClick
    	drawl
		cr 1 + ) drop ;

:teclado
	key
	>esc< =? ( exit )
	drop ;

:inicio
	cls home
	drawtree
	teclado
	;

:main
	rebuild
	'inicio onshow
	;
: main ;