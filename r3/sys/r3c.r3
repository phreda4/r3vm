| r3 compiler
| PHREDA 2019
|------------------
^./r3base.r3
^./r3pass1.r3
^./r3pass2.r3
^./r3pass3.r3
^./r3pass4.r3
^./r3gencod.r3
^./r3gendat.r3

::r3name | "" --
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

	"load" slog
	here dup 'src !
	'r3filename
	2dup load | "fn" mem
	here =? ( "no src" slog ; )
	0 swap c!+ 'here !

	0 'error !
	0 'cnttokens !
	0 'cntdef !
	'inc 'inc> !

	"pass1" slog
	swap r3-stage-1

	cnttokens "..toks:%d" slog
	cntdef "..def:%d" slog

	"pass2" slog
	r3-stage-2

	1? ( "error ** %d" slog ; ) drop
	code> code - 2 >> "..code:%d" slog

|	debugdicc

	"pass3" slog
	r3-stage-3

	"pass4" slog
	r3-stage-4

	"gencode" slog
	r3-gencode
	r3-gendata

	;

: mark
	"r3/test.r3"
	r3c
	waitesc
	;
