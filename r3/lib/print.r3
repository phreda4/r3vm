|---------------
| PRINT LIB
| PHREDA 2018
|---------------
^r3/lib/sys.r3
^r3/lib/fontpc.r3

##ccx 0 ##ccy 0
##cch 16 ##ccw 8
##charrom
##charsiz
##charlin

#_charemit 'char8pc
#_charsize 'size8pc

::font! | 'vemit 'vsize --
  '_charsize ! '_charemit ! ;

::emit | c --
  $ff and dup
  _charemit ex
  _charsize ex 'ccx +! ;

::home
  0 'ccx ! 0 'ccy ! ;

::print | "" --
  ccx ccy xy>v >a
  ( c@+ 1? emit ) 2drop ;

::lprint
  ccx ccy xy>v >a
  ( c@+ 1? $ff and 10 =? ( 2drop ; ) 13 =? ( 2drop ; ) emit ) 2drop ;

::cr	cch 'ccy +! 0 'ccx ! ;
::lf	0 'ccx ! ;
