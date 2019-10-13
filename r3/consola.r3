| simple console in graphics code
| PHREDA 2013
|-------------------------
^r3/lib/mem.r3
^r3/lib/rand.r3
^r3/lib/fontpc.r3

#c.x #c.y
#c.w #c.h
#c.scrn 0
#c.cursor
#c.atrib $ff00

#wsum
#wcar
#hcar

#palc 0 0

:charline | byte --
	8 ( 1? 1 - swap
		dup $80 and 6 >> 'palc + @ a!+
		1 << swap ) 2drop ;

:2color | c1 -- c1 c2
	dup $f0f0f0 and dup 4 >> or
	swap $f0f0f and dup 4 << or ;

:dchar | dchar -- ;
	dup 8 >> 2color 'palc !+ !
	$ff and 4 << 'font8x16 +
	16 ( 1? 1 - swap
		c@+ charline
		wcar a+ swap ) 2drop ;

:c.draw
	c.scrn >b
	vframe >a
	c.h ( 1? 1 -
    	c.w ( 1? 1 -
    		b@+ dchar
			hcar a+
			) drop
    	wsum a+
    	) drop ;

:c.ink | color --
	8 << $fff00 and c.atrib $fff000ff and or 'c.atrib ! ;

:c.rand
	c.scrn >b c.w c.h * ( 1? 1 - random b!+ ) drop ;

:c.full | w h --
	4 >> 'c.h !
	3 >> 'c.w !
	here dup 'c.scrn ! 'c.cursor !
	0 'c.x ! 0 'c.y !
	c.w c.h * 2 << 'here +!

	sw 8 - 2 << 'wcar !
	sw 4 << neg 8 + 2 << 'hcar !
	sw 4 << c.w 3 << - 2 << 'wsum !
	;

:c.in
	c.x c.y
:c.at | x y --
	c.w * + 2 <<
	c.scrn + 'c.cursor ! ;

:c.uscroll
	c.scrn
	c.w 2 << over +
	c.w c.h *
	move
	;

:c.cr
	0 'c.x !
	c.y 1 + c.h <? ( 'c.y ! c.in ; ) drop
	c.uscroll c.in ;

:c.emit | char --
	$ff and c.atrib or c.cursor !
	c.x 1 + c.w <? ( 'c.x ! c.in ; ) drop
	0 'c.x !
	c.y 1 + c.h <? ( 'c.y ! c.in ; ) drop
	c.uscroll c.in ;

:c.print | "" --
	( c@+ 1? c.emit ) 2drop ;

:c.cls
	c.scrn dup 'c.cursor !
	0 'c.x ! 0 'c.y !
	c.w c.h * ( 1? 1 - 0 rot !+ swap ) 2drop ;

:c.le
	c.x 0 >? ( 1 - 'c.x ! c.in ; ) drop
	c.w 1 - 'c.x !
	c.y 0 >? ( 1 - 'c.y ! c.in ; ) drop
	c.in ;
:c.ri
	c.x c.w <? ( 1 + 'c.x ! c.in ; ) drop
	0 'c.x !
	c.y c.h <? ( 1 + 'c.y ! c.in ; ) drop
	c.in ;
:c.up
	c.y 0 >? ( 1 - 'c.y ! c.in ; ) drop
	c.h 1 - 'c.y !
	c.in ;
:c.dn
	c.y c.h <? ( 1 + 'c.y ! c.in ; ) drop
	0 'c.y !
	c.in ;

|--------------------------
:teclado
	key
	<char char> bt? ( c.emit ; )
	>esc< =? ( exit )
	<f2> =? ( c.cls )
	<f3> =? ( c.uscroll )
	<ret> =? ( c.cr )
	<le> =? ( c.le )
	<ri> =? ( c.ri )
	<up> =? ( c.up )
	<dn> =? ( c.dn )
	drop
	;

:test
	cls
	c.draw
	teclado
	;

:
mark
sw sh c.full
$00f00000 'c.atrib !
'test onshow
;
