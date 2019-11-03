^r3/lib/sys.r3
^r3/lib/str.r3
^r3/lib/print.r3

:screen
	home
	0 'ink !
	backline
	$ff00 'ink !
	over .d print sp
	pick2 .d print sp
	pick3 .d print sp
	pick4 .d print sp
	key >esc< =? ( exit ) drop
	;

::trace | --
	'screen onshow
	;

