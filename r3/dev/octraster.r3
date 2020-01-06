|MEM 32768
| octree rasterization
| PHREDA 2020
|---------------------
^r3/lib/3d.r3
^r3/lib/gr.r3
^r3/lib/gui.r3
^r3/lib/zbuffer.r3

^r3/lib/trace.r3

##w3do
##h3do
#hocc
#wocc

#$base
##$octree
##$pixels
#$paleta

#minx #lenx
#miny #leny
#minz #lenz

#nminz
#zlen

#x1 #y1 #z1	| Nx
#x2 #y2 #z2	| Ny
#x4 #y4 #z4	| Nz

#sx #sy #sz	| suma

|---- hoja en iso,ratio y octree
#veci
#vecr
#veco

#isovec * 960
#isovec> 'isovec

:vec-	-48 'isovec> +! ;

|-------------------------------
#tpopcnt (
 0 1 1 2 1 2 2 3 1 2 2 3 2 3 3 4
 1 2 2 3 2 3 3 4 2 3 3 4 3 4 4 5
 1 2 2 3 2 3 3 4 2 3 3 4 3 4 4 5
 2 3 3 4 3 4 4 5 3 4 4 5 4 5 5 6
 1 2 2 3 2 3 3 4 2 3 3 4 3 4 4 5
 2 3 3 4 3 4 4 5 3 4 4 5 4 5 5 6
 2 3 3 4 3 4 4 5 3 4 4 5 4 5 5 6
 3 4 4 5 4 5 5 6 4 5 5 6 5 6 6 7 )

:popcnt | nro -- cnt
	'tpopcnt + c@ ;

:addchild | bm 0 mask -- bm ch mask
	1 over <<
	pick3 na? ( drop ; ) drop
	swap 4 << over $8 or or swap ;

:fillchild | bitmask -- norden
	0
	nminz addchild
	1 xor addchild	| 1 xor
	3 xor addchild	| 2 xor
	6 xor addchild	| 4 xor
	7 xor addchild	| 3 xor
	6 xor addchild	| 5 xor
	3 xor addchild	| 6 xor
	1 xor addchild	| 7 xor
	drop nip | $ffffffff and
	;

:octcolor | oct -- color
    $octree - $pixels + @ ;

:octcolor3 | oct -- color
	$octree - 2 >> dup 1 << + 1 - $pixels + @ $ffffff and ;

:octcolor8 | oct -- color
	$octree - 2 >> $pixels + c@ $ff and 2 << $paleta + @ ;

:octcolor16 | oct -- color
	$octree - 1 >> $pixels + @ $ffff and ; |16to32 ;

| x y z -- x y z x2 y2 z2
:getnn
	%111 xor
	2 << dup 1 << + 48 - isovec> + >a | 12*
	pick2 a@+ - pick2 a@+ - pick2 a@ - ;

:getn  | x y z n -- x y z x1 y1 z1
	%100 an? ( getnn ; )
	2 << dup 1 << + 48 - isovec> + >a | 12*
	pick2 a@+ + pick2 a@+ + pick2 a@ + ;

|-------------------------------
:calco |  x y z oc -- x y z oc mask
	pick3 x1 * pick3 y1 * + pick2 z1 * + 31 >> 1 and
	pick4 x2 * pick4 y2 * + pick3 z2 * + 31 >> 2 and or
	pick4 x4 * pick4 y4 * + pick3 z4 * + 31 >> 4 and or
	$7 xor
	;

:oct++ | adr -- adr bitmask
	@+ dup 8 >> 2 << rot + swap ; |$ff and ;

|---------------------------
:2/a | a -- b
	dup 31 >> - 1 >> ;

:2/b | a -- c
	dup dup 31 >> - 1 >> - ;

:reduce
	isovec> dup >a 48 -
	@+ 2/a a!+ @+ 2/a a!+ @+ 2/a a!+
	@+ 2/a a!+ @+ 2/a a!+ @+ 2/a a!+
	@+ 2/a a!+ @+ 2/a a!+ @+ 2/a a!+
	@+ 2/a a!+ @+ 2/a a!+ @ 2/a a!+
	a> 'isovec> !
	;

:precalcvec
	isovec> dup >a 48 -
	8 2 <<
	( 1? 1 - swap
		@+ 2/a a!+ @+ 2/a a!+ @+ 2/a a!+
		swap ) 2drop ;

|---------------------------
:restac
	%111 xor
	2 << dup 1 << + isovec> + >a | 12*
	pick4 a@+ - pick4 a@+ - pick4 a@ -
	48 'isovec> +!
	;

:sumac | x y z node bitm nro -- x y z node bitm xn yn zn
	%100 an? ( restac ; )
	2 << dup 1 << + isovec> + >a | 12*
	pick4 a@+ + pick4 a@+ + pick4 a@ +
	48 'isovec> +!
	;

:child-oct | x y z node bitm nro place -- x y z node bitm xn yn zn noct
	1 - pick2 and popcnt 2 << pick3 +	| x y z node bitm nro nnode
	>r sumac r>			| x y z node bitm xn yn zn nnode
	;

|--------------------------------
#xx0 #yy0 #zz0
#xx1 #yy1 #zz1
#xx2 #yy2 #zz2
#xx4 #yy4 #zz4

#xy #zz
#len

#xmask * 256
#ymask * 256

#vecpos * 64	| child vectors | 8*8
#stacko * 256	| stack octree+nchildrens
#stacko> 'stacko

:stack4! | a b c d --
	stacko> !+ !+ !+ !+ 'stacko> ! ;

:getyxmaskl | len -- len bm
	xy dup 16 >> 'ymask + c@
	swap $ffff and 'xmask + c@
	and $ff and ;

:getyxmask0 | y x -- y x mask
	over 'ymask + c@
	over 'xmask + c@
	and $ff and	;

|--------------------------------------
:stack4@2 | -- a b
	stacko> 16 - dup 'stacko> !
	@+ 'zz ! @+ 'xy !
	@+ swap @ ;

:prevchild | len -- ordenn len
	1 >> 0? ( dup ; )
	stack4@2
	4 >>> 0? ( 2drop prevchild ; )
	swap >b swap ;

:nextchild | norden len -- norden len
	1 << swap		| len norden
	dup b> xy zz stack4!
	$7 and 1 over << 1 - >r
	3 << 'vecpos +
	@+ xy swap - 1 << 'xy !
	@ neg 'zz +!
    b@+ dup r> and popcnt swap 8 >> + 2 << b+
	getyxmaskl	| len bm
	b@ and 0? ( drop prevchild ; )
	fillchild	| len norden
	swap ;

:rayoctree | octree s y x -- octree s y x
	getyxmask0 0? ( drop 4 a+ ; )
	pick4 >b b@ and 0? ( drop 4 a+ ; )
	pick2 16 << pick2 or 'xy !
	minz 'zz !
	'stacko 'stacko> !
	fillchild	| norden
	1 ( len <?		| norden len
		b> $pixels >=? ( octcolor a!+ 2drop ; ) drop
		nextchild	| norden len
		0? ( 2drop 4 a+ ; )
		) 2drop
	b> octcolor a!+ ;

:drawiso | octree --
	minx miny xy>v >a
	sw lenx - 2 <<
	0 ( leny <?
		0 ( lenx <?
			rayoctree
			1 + ) drop
		over a+
		1 + ) 3drop ;

|---------------------------------------------
:sminmax3 | a b c -- sn sx
	pick2 dup 63 >> not and
	pick2 dup 63 >> not and +
	over dup 63 >> not and + >r
	dup 63 >> and
	swap dup 63 >> and +
	swap dup 63 >> and +
	r> ;

:packxyza!+ | x y z -- xyz0
	rot xx0 + 1 >>
	rot yy0 + 1 >> 16 << or a!+
	zz0 + a!+ ;

:fillx | child x --
	xx0 + 1 >> 'xmask +
	lenx 1 >> ( 1? 1 - | child xmin len
		pick2 pick2 c+!
		swap 1 + swap ) 3drop ;

:filly | child x --
	yy0 + 1 >> 'ymask +
	leny 1 >> ( 1? 1 - | child xmin len
		pick2 pick2 c+!
		swap 1 + swap ) 3drop ;

:isodraw | x y z node --
	>r
	0 getn p3di 'xx0 ! 'yy0 ! 'zz0 !
	1 getn p3di xx0 - 'xx1 ! yy0 - 'yy1 ! zz0 - 'zz1 !
	2 getn p3di xx0 - 'xx2 ! yy0 - 'yy2 ! zz0 - 'zz2 !
	4 getn p3di xx0 - 'xx4 ! yy0 - 'yy4 ! zz0 - 'zz4 !
	3drop

    xx1 xx2 xx4 sminmax3 over - 1 + 'lenx ! xx0 + 'minx !
    yy1 yy2 yy4 sminmax3 over - 1 + 'leny ! yy0 + 'miny !
    zz1 zz2 zz4 sminmax3 over - 1 + 'lenz ! zz0 + 'minz !

    minx neg 'xx0 +!
    miny neg 'yy0 +!
	minz neg 'zz0 +!

	lenx leny max 4 >> 'len !
	'vecpos >a
	0 0 0 packxyza!+
	xx1 yy1 zz1 packxyza!+
	xx2 yy2 zz2 packxyza!+
	xx1 xx2 + yy1 yy2 + zz1 zz2 + packxyza!+
	xx4 yy4 zz4 packxyza!+
	xx4 xx1 + yy4 yy1 + zz4 zz1 + packxyza!+
	xx4 xx2 + yy4 yy2 + zz4 zz2 + packxyza!+
	xx4 xx1 + xx2 + yy4 yy1 + yy2 + zz4 zz1 + zz2 + packxyza!+

	'xmask 0 64 fill
	$1 0 fillx
	$2 xx1 fillx
	$4 xx2 fillx
	$8 xx1 xx2 + fillx
	$10 xx4 fillx
	$20 xx4 xx1 + fillx
	$40 xx4 xx2 + fillx
	$80 xx4 xx2 + xx1 + fillx

	'ymask 0 64 fill
	$1 0 filly
	$2 yy1 filly
	$4 yy2 filly
	$8 yy1 yy2 + filly
	$10 yy4 filly
	$20 yy4 yy1 + filly
	$40 yy4 yy2 + filly
	$80 yy4 yy2 + yy1 + filly

	r> drawiso
	vec-
	;

:testiso | x y z node --
	>r
	0 getn p3di 'xx0 ! 'yy0 ! 'zz0 !
	1 getn p3di xx0 - 'xx1 ! yy0 - 'yy1 ! zz0 - 'zz1 !
	2 getn p3di xx0 - 'xx2 ! yy0 - 'yy2 ! zz0 - 'zz2 !
	4 getn p3di xx0 - 'xx4 ! yy0 - 'yy4 ! zz0 - 'zz4 !
	3drop

    xx1 xx2 xx4 sminmax3 over - 1 + 'lenx ! xx0 + 'minx !
    yy1 yy2 yy4 sminmax3 over - 1 + 'leny ! yy0 + 'miny !
    zz1 zz2 zz4 sminmax3 over - 1 + 'lenz ! zz0 + 'minz !

	r>
	octcolor 'ink !
	lenx leny minx miny fillrect
	vec- ;

:nchild | x y z node ordenn -- x y z node ordenn xn yn zn noct
	1 over $7 and << 1 -
	1 - pick2 and popcnt 2 << pick3 +	| x y z node bitm nro nnode
	>r sumac r>			| x y z node bitm xn yn zn nnode
	;

|---------------- search iso ratio
:viewrentry | x y z node bm norden -- x y z node
	1 over << pick2 na? ( 2drop ; )
	child-oct
:viewr | x y z node --
	calco 'nminz !
	over clz zlen <=? ( drop

		isodraw
|		testiso

		; ) drop
	$pixels >=? ( testiso ; ) |vecr exec ; )
	1 'zlen +!
	oct++		| x y z node+ bm
	nminz >r
	r@ viewrentry
	r@ 1 xor viewrentry
	r@ 2 xor viewrentry
	r@ 4 xor viewrentry
	r@ 3 xor viewrentry
	r@ 5 xor viewrentry
	r@ 6 xor viewrentry
	r> 7 xor viewrentry
	nip 4drop
	-1 'zlen +!
	vec- ;

|----------- search cube in screen
:cullz | z x
	-? ( neg <? ( $1 ; ) 0 ; )
	<? ( $2 ; ) 0 ;

:culling | x y z -- cull
	dup 2 - 59 >> $10 and >a | 1 <? ( $10 )( 0 ) >a
	swap hocc *. cullz a+
	swap wocc *. cullz 2 << a+
	drop a> ;

:cull1 | x y z -- cull
	culling dup 8 << or ;

:culln | xyz -- cullo culla
	culling dup 8 << $ff or ;

:cullingcalc | x y z node -- x y z node aocull
	>r
	0 getn cull1 >r
	1 getn culln r> and or >r
	2 getn culln r> and or >r
	3 getn culln r> and or >r
	4 getn culln r> and or >r
	5 getn culln r> and or >r
	6 getn culln r> and or >r
	7 getn culln r> and or
	r> swap ;


|----------------------
:vieworetry | x y z node bm norden -- x y z node
	1 over << pick2 na? ( 2drop ; )
	child-oct
:viewo | x y z node --
	cullingcalc
	0? ( drop viewr ; )
	$ff00 na? ( nip 4drop vec- ; )
	drop
	$pixels >=? ( veco ex ; )
	calco swap >r
	1 'zlen +!
	oct++
	r@ vieworetry
	r@ 1 xor vieworetry
	r@ 2 xor vieworetry
	r@ 4 xor vieworetry
	r@ 3 xor vieworetry
	r@ 5 xor vieworetry
	r@ 6 xor vieworetry
	r> 7 xor vieworetry
	nip 4drop
	-1 'zlen +!
	vec- ;

|-------- octree in octree
:vecis	
	|drawcube veci- ; |drawbox veci- ; |drawboxi veci- ;
	; | no mas?
:vecrs
	|drawrealcube
	vec- ; | no iso
:vecos
	4drop |drawborde
	vec- ;

#vecsim	'vecis 'vecrs 'vecos 0 0 0 0 0

:setvec | m --
	$100 and 6 >>
	2 << 'vecsim +
	>a a@+ 'veci ! a@+ 'vecr ! a@ 'veco !
	;

:adjustmem | octree --
	dup '$base !
	dup 28 + '$octree !
	@+ setvec
	@ $octree + '$pixels !
	;

#opila * 32
#opila> 'opila

:getoct | octree -- octree
	$base opila> !+ 'opila> !
	$octree - $pixels + @
	2 << $base +
	adjustmem
	$octree ;

:getocti | --
	-4 'opila> +! opila> @
	adjustmem ;

:vecio 	|getoct viewi getocti | no usado mas
		;
:vecro	getoct viewr getocti ;
:vecoo	getoct viewo getocti ;

|--------- exportadas
::drawsoctree | size moctree --
	adjustmem
	dup 1 << clz 6 - 'zlen !

	0 0 0 transform 'sz ! 'sy ! 'sx !
	'isovec >b
	neg
	dup dup dup transform rot sx - b!+ swap sy - b!+ sz - b!+
	dup dup dup neg transform rot sx - b!+ swap sy - b!+ sz - b!+
	dup dup neg over transform rot sx - b!+ swap sy - b!+ sz - b!+
	dup neg dup transform rot sx - b!+ swap sy - b!+ sz - b!+
	b> 'isovec> !

	sx sy sz
	0 0 -255 transform pick3 - 'z1 ! pick3 - 'y1 ! pick3 - 'x1 !
	0 -255 0 transform pick3 - 'z2 ! pick3 - 'y2 ! pick3 - 'x2 !
	-255 0 0 transform pick3 - 'z4 ! pick3 - 'y4 ! pick3 - 'x4 !

	precalcvec
	$octree viewo ;

::drawoctree | moctree --
	0.5 swap drawsoctree ;

::load3do | "" -- moctree
	here dup rot load 'here ! ;

::ini3do | w h --
	2dup o3dmode
	2dup 'h3do ! 'w3do !
	zb.ini

    2.1 'hocc ! | calcular los valores!!****** 1024*600
    1.2 'wocc !

    'vecsim 16 + >a 'vecio a!+ 'vecro a!+ 'vecoo a!
    ;

|-------------
| TEST LIB...
|-------------
:pix
	b@+
	$7fffffff <>? ( drop b> zbo + @ a!+ ; )
	drop 4 a+ ;

|	$7fffffff =? ( drop 1 px+! ; )
|	$ff and px!+ ;

::zdraw | x y --
	xy>v >a
	zb >b
	sw w3do - 2 <<
	h3do ( 1? 1 -
		w3do ( 1? 1 - pix ) drop
		over a+
		) 2drop ;

|-------------
#xcam 0 #ycam 0 #zcam 2.0

|-------------
:3dop transform p3d op ;
:3dline transform p3d line ;

|-------------
:freelook
	xypen
	sh 1 >> - 7 << swap
	sw 1 >> - neg 7 << swap
	neg mrotx mroty
	;

#sizx #sizy #sizz

:drawz | z --
	sizx neg sizy neg pick2 3dop
	sizx sizy neg pick2 3dline
	sizx sizy pick2 3dline
	sizx neg sizy pick2 3dline
	sizx neg sizy neg rot 3dline ;

:drawl | x1 x2 --
	2dup sizz neg 3dop sizz 3dline ;

:box3d | sx sy sz --
	'sizz ! 'sizy ! 'sizx !
	sizz neg drawz sizz drawz
	sizx neg sizy neg drawl sizx sizy neg drawl
	sizx sizy drawl sizx neg sizy drawl
	;


:tecla
	key
	<f1> =? ( 0.01 'wocc +! )
	<f2> =? ( -0.01 'wocc +! )
	<f3> =? ( 0.01 'hocc +! )
	<f4> =? ( -0.01 'hocc +! )
	<up> =? ( -0.01 'zcam +! )
	<dn> =? ( 0.01 'zcam +! )
	<le> =? ( -0.01 'xcam +! )
	<ri> =? ( 0.01 'xcam +! )
	<pgup> =? ( -0.01 'ycam +! )
	<pgdn> =? ( 0.01 'ycam +! )
	>esc< =? ( exit )
	drop ;

#fps
#fpsc
#mseca

:everysec
	1 'fpsc +!
	msec mseca <? ( drop ; )
	1000 + 'mseca !
	fpsc 'fps ! 0 'fpsc !
	;

#Omario

:main
	cls home
	matini
	freelook
	xcam ycam zcam mtrans

|	zb.clear
	Omario drawoctree
|	0 0 zdraw

	$ff00 'ink !
	over "%d " print cr
	fps "fps:%d" print cr
	zcam ycam xcam "%f %f %f" print cr
	hocc wocc "%f %f" print cr
	h3do w3do 16 <</
	1 << "%f" print cr
	minz "%d " print cr
	lenz leny lenx "%d %d %d" print cr

	tecla
	everysec
	;

:
	msec 'mseca !
	mark
	sw sh ini3do

|	"3do/horse.3do"
|	"3do/sibenika.3do"
|	"3do/tree1.3do"
	"3do/mario.3do"
	load3do 'Omario !
	33
	'main onshow
	;

