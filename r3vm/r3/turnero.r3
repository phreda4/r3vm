|SCR 800 600
|MEM 32768
||FULL
|
| Turnero ABEL

#fileinfo "C:\WinNMP\WWW\turnero\nn.txt"
#fileinfof "C:\WinNMP\WWW\turnero\info.txt"
#filealerta "C:\WinNMP\WWW\turnero\oo.txt"

^r3/lib/sys.r3
^r3/lib/str.r3
^r3/lib/mem.r3
^r3/lib/parse.r3
^r3/lib/print.r3

^r3/lib/sprite.r3

^r3/util/loadimg.r3
^r3/util/arr16.r3
^r3/util/polygr.r3
^r3/util/penner.r3

^r3/lib/fontr.r3

^r3/rft/robotobold.rft
^r3/rft/robotoregular.rft

#backimg 0
#listaimg 0 0
#ding 0

|----------------------
#nfiles
#files * 4096	| 1000 files
#files> 'files
#filenow> 'files
#filen * $3fff
#filen> 'filen

:FNAME | adr -- adrname
	44 + ;

:fileadd
	filen> files> !+ 'files> !
	FNAME filen> strcpyl 'filen> !
	;

|-------------------------------------
:reload
	'files 'files> !
	'filen 'filen> !
	"videos/" ffirst drop | quita .
	fnext drop	| quita ..
	( fnext 1? fileadd ) drop
	files> 'files - 2 >> 'nfiles !
	;

:showvideo
	269 114 videoshow
	0? ( drop ; ) drop
:nextvideo
	filenow> 4 + files> >=? ( drop reload 'files )
:inivideos
	dup 'filenow> !
	@ "videos/%s" mformat
	| test file?????
	528 300 video
	;

|------------- lista de turnos
#ooa * 8192

#boxtur * $3fff
#nbox

#ntur
#pactur * 64
#medtur * 64
#esptur * 64

:next} | adr -- adr'
	( c@+ 1?
		$7b =? ( drop 0 swap 1 - c!+ ; )
		drop ) nip ;

:next{ | adr -- adr'
	( c@+ 1?
		$7d =? ( drop 0 swap 1 - c!+ ; )
		drop ) nip ;

|----------
:+stackt
	nbox 6 >? ( -1 'nbox +! ) drop
	'boxtur dup 200 + swap nbox 200 * 2 >> move>
	'boxtur >a
	ntur a!+
	a> 'pactur 16 move 64 a+
	a> 'medtur 16 move 64 a+
	a> 'esptur 16 move 64 a+
	msec a!+
	1 'nbox +!
	;

:+turno | adr --
	( trim dup 1? c@ 1? drop
		str>nro 'ntur !
		1 +
		dup next} swap 'pactur strcpy
		dup next} swap 'medtur strcpy
		dup next{ swap 'esptur strcpy
		+stackt
		) 2drop
	ding splay ;

#xb1 #yb1

:fillbb
	$495560 'ink !
	xb1 yb1 268 over 67 +
	fillbox ;

:colorbox | adr --
	dup 196 + @
	msec swap -
	1800000 >? ( -1 'nbox +! )
	120000 >? ( drop ; )
	drop
	xb1 220 + yb1 20 +
	msec 8 >> 1 and 2 << 'listaimg + @
	sprite
	;

:drawboxt |
	fillbb
	colorbox

	$ffffff 'ink !
	xb1 10 + yb1 6 + atxy
	4 + dup emits
	xb1 10 + yb1 34 + atxy
	64 + dup emits
	64 + 64 + 4 +
	;

:turnodraw
	robotobold 48 fontr!
	$1d242b 'ink ! 330 70 atxy
	"INFORMACION" emits

	robotobold 36 fontr!
	$ffffff 'ink ! 330 423 atxy
	"NOTICIAS" emits

	robotobold 34 fontr!
	4 'xb1 ! 115 'yb1 !
	'boxtur
	0 ( nbox <? swap
		drawboxt
		69 'yb1 +!
		swap 1 + ) 2drop ;

|-------------------------------------
#x1 282 #y1 465 #x2 780 #y2 580

#cline * $800
#lastcl $500

:bprint | "" --
	c@+ 0? ( 2drop ; )
	1 =? ( robotoregular 32 fontr! )
	2 =? ( robotobold 40 fontr! )
	drop
	dup c@ 0? ( 2drop ; ) drop
	$ffffff 'ink !
	swprint ccx 8 - ccy rot pick2 + 16 + over cch + fillbox
	$0 'ink !
	emits ;

:infodraw
	x1 y1 atxy
	'cline 5 ( 1?
		over bprint
		1 - swap $100 + swap
		x1 'ccx ! cch 'ccy +!
		) 2drop ;

:addline | "" t --
    'cline dup $100 + $800 cmove
	'cline lastcl + c!+
	strcpy ;

|-------------------------------------
:loadoo
	'ooa 'filealerta load
	'ooa =? ( 0 swap ! ; )
	0 swap !
	0 0 'filealerta save | delete
	'ooa +turno	;

|--------------------------------
:nowsdate | -- s64
	date 24 << time or ;

:strsdate2 | str -- s64 ; d/m/y
	0 >a
	trim str>nro 24 << a+ | day
	1 +  str>nro 32 << a+ | moth
	1 +  str>nro 40 << a+ | year
	trim str>nro 16 << a+ | hora
 	1 +  str>nro 8 << a+  | min
	1 +  str>nro a+       | seg
	drop a> ;

:strsdate | str -- s64 ; y-m-d
	0 >a
	trim str>nro 40 << a+ | year
	1 +  str>nro 32 << a+ | moth
	1 +  str>nro 24 << a+ | day
	trim str>nro 16 << a+ | hora
 	1 +  str>nro 8 << a+  | min
	1 +  str>nro a+       | seg
	drop a> ;

:>>sep | adr -- adr'/0
	0? ( ; )
	( c@+ 1? $ff and
		$ac =? ( drop ; ) |$ac - separador "¬"
		drop ) nip ;

#infotime * 4096
#infotime> 'infotime
#infolines * $ffff
#infolines> 'infolines

#infonow>

:,itq
	infotime> q!+ 'infotime> ! ;

:,it
	infotime> !+ 'infotime> ! ;

:debuglline
	'infotime
	( infotime> <?
		"* " emits
		q@+ "%h " print
		q@+ "%h " print
		@+ emits cr
		@+ emits cr
		) drop ;

:parseline | adr -- adr'
	dup strsdate ,itq
	>>sep 0? ( ; )
	dup strsdate ,itq
	>>sep 0? ( ; )
	dup >>sep 0? ( drop ; )
	0 over 1 - c!
	infolines> ,it
    swap infolines> strcpyl 'infolines> !
	dup >>sep 0? ( drop ; )
	0 over 1 - c!
	infolines> ,it
    swap infolines> strcpyl 'infolines> !
	;

:loadinfo
	mark
	here 'fileinfof load 0 swap !
	'infotime 'infotime> !
	'infotime 'infonow> !
	'infolines 'infolines> !
	here
	( >>sep 1? parseline ) drop
	empty
	;

#d1 0 0
#d2 0 0
#ti 0
#sd 0

#ttnex 0
#tnext 0

:nextline
	infonow>
	q@+ 'd1 q!
	q@+ 'd2 q!
	@+ 'ti ! |+info
	@+ 'sd ! |+info
	infotime> >=? ( drop 'infotime )
	'infonow> !
	;

:buscavalido | solamente lo que este en fecha
	infonow>
	( nextline
	  nowsdate 'd1 q@ 'd2 q@ bt? ( 2drop ; ) drop
	  infonow> <>? ) drop
	0 'ti ! ;

:addcr
	"" 0 addline ;

#auxtt * 1024
#auxt 0

:toprint | s "" -- str'
	0 swap ( c@+ 1?
|		10 =? ( 3 + )
		13 =? ( drop nip nip 1 - ; )
		emitsize rot + | s "" cnt
		pick2 >? ( drop nip 1 - ; )
		swap ) drop nip nip 1 - ;

:ttcut
	ttnex
	1 =? ( robotoregular 32 fontr! )
	2 =? ( robotobold 40 fontr! )
	drop
	x2 x1 - tnext toprint
    ( dup c@ $ff and 32 >? drop 1 - ) drop
	0 swap c!
	;

:nuevainfo
	buscavalido
    ti 0? ( drop addcr ; )
	'auxtt dup 'tnext ! strcpyl 0 swap !
	2 'ttnex !
	ttcut
	addcr ;

:nextcut
	tnext
	( c@+ 1? drop ) drop
	trim
	dup c@
	0? ( 'tnext ! drop ; ) drop
	'tnext !
	ttcut ;

:nextcut2
	tnext
	( c@+ 1? drop ) drop
	trim
	dup c@
	0? ( 2drop 1 'ttnex ! 1 'tnext ! ; ) drop
	'tnext !
	ttcut ;

| proxima linea
:nextinfocon
	tnext
	0? ( drop nuevainfo ; )
	1 =? ( drop sd 'auxtt dup 'tnext ! strcpyl 0 swap ! ttcut ; )
	ttnex addline
	ttnex 1 =? ( drop nextcut ; ) drop
	nextcut2 ;

:loadin
	'ooa 'fileinfo load
	'ooa =? ( 0 swap ! ; )
	0 swap !
	0 0 'fileinfo save | delete
	loadinfo ;

|-------------------------------------
#nextms
#nextin

:mm
	mark
	cls home
	" cargando..." emits
	redraw
	"img/a1.png" loadimg 'listaimg !
	"img/a2.png" loadimg 'listaimg 4 + !
	"img/back-turnero.png" loadimg 'backimg !
	"media/dingdong.wav" sload 'ding !
	loadinfo

	msec 'nextms !
	msec 'nextin !
	;

|-------------------------------------
#aux * 1024

:+tt
	'aux strcpy
	'aux +turno
	;
|-------------------------------------

:teclado
	key
	>esc< =? ( exit )
	<f1> =? ( nextvideo )
	<f3> =? ( "266649{Del Prete, Juan Carlos{Oftalmologia - Dr. Gutierrez{Oftalmologia}" +tt )
	<f4> =? ( "266642{Quique, Tito{Alfaro Yamil Adrian{Oftalmologia}" +tt )
	drop ;

:background
    0 0 backimg 0? ( 3drop cls ; )
	sprite ;

:nextmsec
	msec nextms <? ( drop ; )
	1000 + 'nextms !
    loadoo
	;

:nextinfo
	msec nextin <? ( drop ; )
	3000 + 'nextin !	| 3 seg por linea
	nextinfocon
	loadin
	nfiles 1? ( drop ; ) drop | hay videos
	reload
	'files inivideos
	;


|--------------------------------
:show
	background
	showvideo
	turnodraw
	infodraw
	teclado
	nextmsec
	nextinfo
	;

:
	mm
	$ffffff 'paper !
	$0 'ink !
	reload
	'files inivideos
	33
	'show onshow
	;