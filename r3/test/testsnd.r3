^r3/lib/gui.r3

#pp
#p2

:main
  cls home
  "sonido" print
pp "p:%h" print cr
p2 "p2:%h" print cr
  key
  <f1> =? ( pp splay )
  <f2> =? ( p2 mplay )
  >esc< =? ( exit )
  drop
  ;

:
  mark
  "media/piano.wav" sload 'pp !
  "media/test.mp3" mload 'p2 !
  'main onshow
  ;