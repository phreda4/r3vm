| PHREDA - 2019
| Memory words

|--- free memory
##here 0

#memmap * 512
#memmap> 'memmap

::mark | --
	here 0? ( mem dup 'here ! nip )
	memmap> !+ 'memmap> ! ;

::empty | --
	memmap> 'memmap =? ( drop mem 'here ! ; )
	4 - dup 'memmap> ! @ 'here ! ;

::savemem | "" --
	memmap> 4 - @ here over - rot save ;

::cpymem | 'destino --
	memmap> 4 - @ here over -
	cmove ; | de sr cnt --

::appendmem | "" -- ; agregar a diccbase la
	memmap> 4 - @ here over - rot append ;

|---
::, here !+ 'here ! ;
::,c here c!+ 'here ! ;
::,q here q!+ 'here ! ;
::,s here swap ( c@+ 1? rot c!+ swap ) 2drop 'here ! ;
::,w here swap ( c@+ $ff and 32 >? rot c!+ swap ) 2drop 'here ! ;

::,ln ,s
::,cr 13 ,c ;
::,eol 0 ,c ;
::,sp 32 ,c ;
::,nl 13 ,c 10 ,c ;


