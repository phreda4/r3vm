|SCR 800 600
|MEM 32768
||FULL
|
| Turnero ABEL

|#fileinfo "C:\WinNMP\WWW\turnero\nn.txt"
|#fileinfof "C:\WinNMP\WWW\turnero\info.txt"
|#filealerta "C:\WinNMP\WWW\turnero\oo.txt"

#fileinfo "D:\WinNMP\WWW\turnero\nn.txt"
#fileinfof "D:\WinNMP\WWW\turnero\info.txt"
#filealerta "D:\WinNMP\WWW\turnero\oo.txt"

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
^r3/rft/droidsansbold.rft
^r3/rft/comfortaa_bold.rft
^r3/rft/archivoblackregular.rft
^r3/rft/dejavusans.rft
^r3/rft/dejavuserifbold.rft
^r3/rft/opensanslight.rft
^r3/rft/opensansregular.rft

#backimg

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
	268 128 videoshow
	0? ( drop ; ) drop
:nextvideo
	filenow> 4 + files> >=? ( drop reload 'files )
:inivideos
	dup 'filenow> !
	@ "videos/%s" mformat
	| test file?????
	512 300 video
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
		) 2drop ;

:fillbb
	20 pick2 66 * 128 +
	2dup 260 over 64 +
	fillbox ;

:colorbox | adr --
	dup 196 + @
	msec swap -

	1800000 >? ( -1 'nbox +! )
	30000 >? ( drop 0 'ink ! fillbb $ffffff 'ink ! ; )

|	10000 >? ( -1 'nbox +! )
|	2000 >? ( drop $333333 'ink ! fillbb $ffffff 'ink ! ; )

	8 >> 1 an? ( drop
		0 'ink !
		fillbb
		$ffffff 'ink ! ; )
	drop
	0 'ink !
	fillbb
	$ffff00 'ink ! ;

:drawboxt |

	colorbox
	swap 8 + swap 3 + atxy

	4 + dup emits
	64 +
	28 pick2 66 * 158 + atxy
	dup emits
	64 + 64 + 4 +
	;

:turnodraw
	robotobold 48 fontr!
	$ffffff 'ink ! 70 76 atxy
	"TURNOS" emits

	$1d242b 'ink ! 330 76 atxy
	"INFORMACION" emits

	robotobold 40 fontr!
	'boxtur
	0 ( nbox <? swap
		drawboxt
		swap 1 + ) 2drop ;

|-------------------------------------
#x1 270 #y1 420 #x2 780 #y2 580

#consola * 4096
#consola> 'consola

#conline

:crc
	x1 'ccx ! cch 'ccy +!
	conline 1? ( drop ; ) drop
	dup 'conline !
	;

:emitc | car --
|	10 =? ( 3 + )
	13 =? ( drop crc ; )
	| color
	| fondo
	emit
	ccx x2 >? ( drop crc ; ) drop
	;

:scrolly
	'consola conline count 1 + cmove ;

:infodraw
|	$ffffff 'ink ! x1 y1 x2 y2 fillbox
	robotoregular 36 fontr!
|	robotobold 36 fontr!

	$0 'ink !
	x1 y1 atxy
	0 'conline !
    'consola ( c@+ 1? emitc ) 2drop
	ccy y2 >? ( scrolly ) drop ;

#buffconsola * 8192

:loadinfo
	'buffconsola
	'consola strcat
	;

#nl ( 13 0 )

:+info
	'nl 'consola strcat
	'consola strcat
	;

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
		q@+ .h emits " " emits
		q@+ .h emits " " emits
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
#sd 0

:nextline
	infonow> infotime> >=? ( drop 'infotime )
	q@+ 'd1 q!
	q@+ 'd2 q!
	@+ 'sd ! |+info
	'infonow> !
	;

:nextinfocon
	nextline
	nowsdate 'd1 q@ 'd2 q@ bt? ( drop sd +info ; ) drop
	nextinfocon
	;

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

|	"img/back-turnero.jpg"
	"img/back-turnero.png"
	loadimg 'backimg !

	loadinfo

	msec 'nextms !
	msec 'nextin !
	;

|-------------------------------------
:teclado

	key
	>esc< =? ( exit )
	<f1> =? ( nextvideo )
|	<f2> =? ( 'textof +info )
|	<f3> =? ( "266649{Reda, Pablo{Alfaro Yamil Adrian{Oftalmologia}" +tt )
|	<f4> =? ( "266642{Quique, Tito{Alfaro Yamil Adrian{Oftalmologia}" +tt )
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
	5000 + 'nextin !
	loadin
	nextinfocon
	;


|--------------------------------
:show
	background
	showvideo
	turnodraw

|	cr nowsdate 'd1 q@ 'd2 q@ "%h %h %h" print

	infodraw

	teclado
	nextmsec
|	nextinfo
	;

:
	mm
	$ffffff 'paper !
	$0 'ink !
	reload
	'files inivideos
	'show onshow
	;