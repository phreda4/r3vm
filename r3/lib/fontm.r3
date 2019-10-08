| font monospace
| PHREDA 2014
|  uso:
|	^r3/lib/fontm.txt
|   ^r3/fntm/...fuente.rtf
|
|	::fontm | 'fontm --
|-------------------------------
^r3/lib/gr.r3
^r3/lib/print.r3

:a00 4 a+ ;
:a01 a@ color col33% a!+ ;
:a10 color a@ col33% a!+ ;
:a11 color a!+ ;

#acc a00 a01 a10 a11
#acn a11 a10 a01 a00

:charsizem | byte -- size
	ccw nip ;

:charline | sx n bit --
	0? ( drop ccw 2 << a+ ; )
	ccw ( 1? 1 -
		swap dup $3 and 2 << 'acc + @ ex
		2 >> swap ) 2drop ;

:charm | c --
    charlin * charrom +
	ccx ccy xy>v >a
	sw ccw - 2 <<
	cch ( 1? 1 -
		rot @+ charline rot rot
		over a+
		) 3drop ;

::charmt | c --
	$ff and charlin * charrom +
	sw ccw - 2 <<
	cch ( 1? 1 -
        rot @+ charline rot rot
		over a+
		) 3drop ;

:charlinen | sx n bit --
	0? ( drop ccw ( 1? 1 - color a!+ ) drop ; )
	ccw ( 1? 1 -
		swap dup $3 and 2 << 'acn + @ ex
		2 >> swap ) 2drop ;

::charmtn | c --
	$ff and charlin * charrom +
	sw ccw - 2 <<
	cch ( 1? 1 -
        rot @+ charlinen rot rot
        over a+
		) 3drop ;


::fontm | 'fontm --
	>a a@+ dup 2 << 'charlin !
	a@+ swap 'cch ! 'ccw !
	a> 'charrom !
	'charm 'charsizem font! ;
