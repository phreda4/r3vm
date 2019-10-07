^r3/lib/str.r3
^r3/lib/print.r3

#val 0 0 | 64 bits var

:wait
	cls home
	'val q@ dup 31.1 + 'val q!
	.f print
	key 27 =? ( exit ) drop ;
:
	'wait onshow
	;