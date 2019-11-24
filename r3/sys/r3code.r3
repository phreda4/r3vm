|MEM 8192  | 8MB

| r3 code analyzer
| PHREDA 2019
|------------------
^./r3base.r3
^./r3pass1.r3
^./r3pass2.r3
^./r3pass3.r3
^./r3pass4.r3
^./r3gencod.r3
^./r3gendat.r3

^r3/lib/fontm.r3
^r3/fntm/droidsans13.fnt


:r3name | "" --
	dup
	'r3filename strcpy
	'r3path strcpyl
	( 'r3path >? 1 -
		dup c@ $2f | /
			=? ( drop 0 swap c! ; )
		drop ) drop
	0 'r3path !
	;

::r3c | str --
	r3name
	here dup 'src !
	'r3filename

|	dup "load %s" slog

	2dup load | "fn" mem
	here =? ( "no src" slog ; )
	0 swap c!+ 'here !
	0 'error !
	0 'cnttokens !
	0 'cntdef !
	'inc 'inc> !

|	" pass1" slog
	swap r3-stage-1

	error 1? ( "ERROR %s" slog ; ) drop

|	cntdef cnttokens "toks:%d def:%d" slog
|debuginc

|	" pass2" slog
	r3-stage-2

	1? ( "ERROR %s" slog ; ) drop
|	code> code - 2 >> "..code:%d" slog

	" pass3" slog
	r3-stage-3

|debugdicc

|	" pass4" slog
	r3-stage-4
|trace
|	" gencode" slog
|	r3-gencode
|	r3-gendata
	;

|------------------------------------------------
#filenow
#defnow
#defpnow
#wordnow
#wordpnow

#iniword
#cntword

#cntlines

:header
    $888888 $ff00 fontmcolor
	"r3Code " print
    $444444 $ffffff fontmcolor

	filenow 3 << 'inc + @
	"%l" mprint print
	cr
	;

:wordline | nro --
    $0 $ffffff fontmcolor
	defnow =? ( $ffffff 0 fontmcolor )
	4 << dicc +
	@+ "%w " mprint print
	drop
	cr
	;

:pagdef
	0 max
	cntdef 1 - min
	defpnow <? ( dup 'defpnow ! )
	defpnow cntlines 1 - + >? ( dup cntlines 1 - - 'defpnow ! )
	dup 4 << dicc + 4 + @ 'iniword !
	'defnow !
	0 'wordpnow !
	;

:insline
	2 << iniword + @
	"%h" mprint print
	;

:paglin
	0 max
	cntword 1 - min

	'wordnow !
	;

:keyboard
	key
	<le> =? ( defnow 1 - pagdef )
	<ri> =? ( defnow 1 + pagdef )

	<up> =? ( wordnow 1 - paglin )
	<dn> =? ( wordnow 1 + paglin )

	>esc< =? ( exit )
	drop
	;

:browser
	cls home
	header
	0 ( cntlines <?
		dup defpnow + wordline
		1 + ) drop
	0 ( cntlines <?
		14 over 1 + gotoxy
		dup wordpnow + insline
		1 + ) drop
	keyboard
	;

:r3code
	inc> 'inc - 3 >> 1 - 'filenow !
	0 pagdef
	'browser onshow
	;

: mark
	'fontdroidsans13 fontm
	rows 2 - 'cntlines !

	"r3/test.r3"
|	"r3/testgui.r3"
	r3c

    r3code
	;

