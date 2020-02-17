^r3/lib/gui.r3

#pp

:main
  cls home
  "sonido" print
  key
  <f1> =? ( pp splay )
  >esc< =? ( exit )
  drop
  ;

:
  mark
  "media/piano.wav" sload 'pp !
  'main onshow
  ;