| Server test
^r3/lib/gui.r3

#mensaje * 1024
#sockserver 0
#sockclient 0
#ip 0 0
#nn 0

:recive
	sockserver TCPACCEPT
	0? ( drop ; )
	'sockclient !
	sockclient 'mensaje 1024 tcprecv
	'mensaje + 0 swap c!

	sockclient tcpclose
	1 'nn +!
	;

:main
	cls home
	"r3 server" print cr
	nn msec "%h %d" print cr
	'ip q@ "IP:%h" print cr cr
	sockclient sockserver "SEVER:%h CLIENT:%h" print cr cr
	'mensaje emits cr cr
	recive
	key
	>esc< =? ( exit )
	drop ;

:inicio
	'ip 0 9999 NETHOST | server
	'ip tcpOpen 'sockserver !
	;

:
	inicio
	'main onshow ;