| Server test
| PHREDA 2020
^r3/lib/gui.r3

#mensaje * 1024
#sockserver 0
#ip 0 0
#nn 0

#scliente 0

:acepta
	sockserver TCPACCEPT 0? ( drop ; )
	'scliente ! ;

:recibe
	scliente
	0? ( drop acepta ; )
	'mensaje 1024 tcprecv
	'mensaje + 0 swap c!
	1 'nn +!
	;


:main
	cls home
	"r3 server" print cr
	nn msec "%h %d" print cr
	'ip q@ "IP:%h" print cr cr
	sockserver "SEVER:%h" print cr cr
	'mensaje emits cr cr

	recibe

	key
	>esc< =? ( exit )
	drop 
	15 framelimit ;

:netini
	'ip 0 9999 nethost | 0=server 9999 port
	'ip tcpopen 'sockserver !
	;

:netend
	scliente 1? ( dup tcpclose ) drop
    sockserver 1? ( dup tcpclose ) drop
	;

:
	netini
	'main onshow
	netend ;