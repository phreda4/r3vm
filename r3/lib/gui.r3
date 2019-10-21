|------------------------------
| gui.e3 - PHREDA
| Immediate mode gui for r3
|------------------------------
^r3/lib/sys.r3

|--- state
##hot	| activo actual
#hotnow	| activo anterior
#foco	| activo teclado

|--- id
#id		| id gui actual
#idf	| id gui foco actual (teclado)
#idl	| id foco ultimo

|---------
::gui.debug
	idf id "ID:%d IDF:%d " print cr
	foconow foco "foco:%d now:%d" print cr
	hotnow hot "hot:%d now:%d" print cr
	;

::gui
	idf 'idl ! hot 'hotnow !
	0 dup dup 'id ! 'idf ! 'hot !
	;

|-- boton
::guiBtn | 'click --
	1 'id +!
	xypen whin 0? ( 2drop ; ) drop
	bpen 0? ( id hotnow =? ( 2drop ex ; ) 3drop ; ) 2drop
	id 'hot ! ;

|-- move
::guiMove | 'move --
	1 'id +!
	bpen 0? ( 2drop ; ) drop
	xypen whin 0? ( 2drop ; ) drop
	id 'hot !
	ex ;

|-- dnmove
::guiDnMove | 'dn 'move --
	1 'id +!
	bpen 0? ( 3drop ; ) drop
	xypen whin 0? ( 3drop ; ) drop
	id dup 'hot !
	hotnow <>? ( 2drop ex ; )
	drop nip ex ;

::guiDnMoveA | 'dn 'move -- | si apreto adentro.. mueve siempre
	1 'id +!
	bpen 0? ( 3drop ; ) drop
	hotnow 1? ( id <>? ( 3drop ; ) ) drop | solo 1
	xypen whin 0? ( id hotnow =? ( 'hot ! drop nip ex ; ) 4drop ; ) drop
	id dup 'hot !
	hotnow <>? ( 2drop ex ; )
	drop nip ex ;


|-- mapa
::guiMap | 'dn 'move 'up --
	1 'id +!
	xypen whin 0? ( 4drop ; ) drop
	bpen 0? ( id hotnow =? ( 2drop nip nip ex ; ) 4drop drop ; ) drop
	id dup 'hot !
	hotnow <>? ( 3drop ex ; )
	2drop nip ex ;

::guiDraw | 'move 'up --
	1 'id +!
	xypen whin 0? ( 3drop ; ) drop
	bpen 0? ( id hotnow =? ( 2drop nip ex ; ) 4drop ; ) drop
	id dup 'hot !
	hotnow <>? ( 3drop ; )
	2drop ex ;

::guiEmpty | --		; si toca esta zona no hay interaccion
	1 'id +!
	xypen whin 1? ( id 'hot ! )
	drop ;

|----- test adentro/afuera
::guiI | 'vector --
	xypen whin 0? ( 2drop ; ) drop ex ;

::guiO | 'vector --
	xypen whin 1? ( 2drop ; ) drop ex ;

::guiIO | 'vi 'vo --
	xypen whin 1? ( 2drop ex ; ) drop nip ex ;

|---------------------------
::onLineMove | 'vec --
	bpen 0? ( 2drop ; ) drop
	xypen
	swap tx1 <? ( 3drop ; ) tx2 >? ( 3drop ; ) drop
	ccy <? ( 2drop ; ) ccy cch + >? ( 2drop ; ) drop
	ex ;

::onLineClick | 'vec --
	1 'id +!
	xypen
	swap tx1 <? ( 3drop ; ) tx2 >? ( 3drop ; ) drop
	ccy <? ( 2drop ; ) ccy cch + >? ( 2drop ; ) drop
	bpen 1? ( 2drop id 'hot ! ; ) drop
	id hotnow <>? ( 2drop ; ) drop
	ex ;


|---------------------------------------------------
| manejo de foco (teclado)

::nextfoco
	foco 1+ idl >? ( 0 nip ) 'foco !
	0 key!
	;

::prevfoco
	foco 1- 0 <=? ( idl nip ) 'foco !
	0 key!
	;

::setfoco | nro --
	'foco ! -1 'foconow ! ;

::ktab
	mshift 0? ( drop nextfoco ; ) drop
	prevfoco ;

::clickfoco
	idf foco =? ( drop ; ) 'foco ! ;

::clickfoco1
	idf 1+ 'foco ! -1 'foconow ! ;

::exit
	-1 '.exit !
::refreshfoco
	-1 'foconow ! 0 'foco ! ;

::w/foco | 'in 'start --
	idf 1+
	foco 0? ( drop dup dup 'foco ! ) | quitar?
	<>? ( 'idf ! 2drop ; )
	foconow <>? ( dup 'foconow ! swap ex )( nip )
	'idf !
	ex ;

::focovoid | --
	idf 1+
	foco 0? ( drop dup dup 'foco ! ) | quitar?
	<>? ( 'idf ! ; )
	foconow <>? ( dup 'foconow ! )
	'idf ! ;

::esfoco? | -- 0/1
	idf 1+ foconow - not ;

::in/foco | 'in --
	idf 1+
	foco 0? ( drop dup dup 'foco ! )
	<>? ( 'idf ! drop ; )
	'idf !
	ex ;

::bordefoco
	ink
	blanco 1 dup 'w +! 'h +! gc.rod
	negro
	1 dup 'w +! 'h +! gc.rod
	'ink !
	-3 dup 'w +! 'h +!
	;

 | no puedo retroceder!
::lostfoco | 'acc --
	idf 1+ foco <>? ( 'idf ! drop ; ) 'idf !
	ex
	nextfoco
	;
