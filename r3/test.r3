|SCR 256 512
| Example 1

^r3/lib/sys.r3
^r3/lib/gui.r3
^r3/lib/print.r3

:patternxor
 vframe >a
 sh ( 1? 1 -
  sw ( 1? 1 -
    2dup xor msec + 8 <<
	a!+
    ) drop
  ) drop
  key 27 =? ( exit ) drop
  ;

:
 'patternxor onshow
;


:scroll
	cch neg 'ccy +!
	vframe sw cch * 2 << over + sw sh cch - * move
	vframe sw ccy * 2 << + 0 sw cch * fill
	;

::slog | ... --
	mprint print cr
	ccy sh >=? ( scroll ) drop
	redraw	;

#b1 * 8192
#b2 * 8192
#b3 * 8192

:showb
	cls
	vframe >a
	'b1 >b
	2048 ( 1? 1 - b@+ a!+ ) drop
	8192 a+
	'b2 >b
	2048 ( 1? 1 - b@+ a!+ ) drop
	8192 a+
	'b3 >b
	2048 ( 1? 1 - b@+ a!+ ) drop
	;

:t1
	'b1 $ff 2048 fill
	'b2 $ff00 2048 fill
	'b3 $ff0000 2048 fill


	;

:
	mark
	t1
	showb
	home
	40 ( 1? 1 - dup "hola %d" slog waitesc ) drop
	;
