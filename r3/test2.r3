|SCR 512 512
| Example 1

^r3/lib/gui.r3
^r3/lib/mem.r3
^r3/lib/print.r3

^r3/sys/r3parse.r3

#src
#cc

:test
	cls
	trace
	cc "%l" mprint
	trace
	print
	trace

	key
	>esc< =? ( exit )
	<up> =? ( cc >>cr 'cc ! )
	<dn> =? ( exit )
	drop
	;

:
	mark

	here
	dup "r3/test.r3" load 'here !
	'src !
	src 'cc !

	'test onshow
;

