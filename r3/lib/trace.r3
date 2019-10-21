^r3/lib/sys.r3
^r3/lib/str.r3

:screen
	home
	over .d print cr
	pick2 .d print cr
	pick3 .d print cr
	pick4 .d print cr
	key >esc< =? ( exit ) drop
	;

::trace | --
	'screen onshow
	;

