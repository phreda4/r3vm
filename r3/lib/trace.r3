^r3/lib/sys.r3
^r3/lib/str.r3

:screen
	home
	pick4 .d print sp
	pick3 .d print sp
	pick2 .d print sp
	over .d print sp
	key
	>esc< =? ( exit )
	drop
	;

::trace | --
	'screen onshow
	;

