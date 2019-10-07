| Fractal como prueba de rendimiento
| Programa original de Markus Hoffman (2007) para la maquina virtual X11-basic
| Adaptacion de Galileo (2016)
| Adaptacion a r3 PHREDA (2019)

^r3/lib/sys.r3
^r3/lib/math.r3
^r3/lib/str.r3
^r3/lib/print.r3

#bx 0 | Ubicacion
#by 0
#bw 512 | Dimensiones de la ventana
#bh 480
#sx -1.5 | Desplazamiento de la imagen
#sy -1.0
#tw 2.0 | Escala
#th 2.0

:colorc | c --
  dup dup 3 << $ff and
  rot 2 << $ff and
  rot 1 << $ff and
  8 << or 8 << or 'color ! ;

:calcula | x y gx gy -- zx zy c
  2dup 0  ( 256 <? 1 +
  	>r
    over dup *. over dup *. - pick4 + | zx
    rot 1 << rot *. pick2 +            | zy
    over dup *. over dup *. +
	4.0 >? ( drop r> ; )
  	drop r> ) ;

:mandel
	0 0 ( bh <? dup
    	by - bh /. th *. sy + pick2 | gx
		bx - bw /. tw *. sx + swap  | gy
		calcula >r 4drop r>
		colorc 2dup pset swap 1 + bw =? ( drop 1 + 0 ) swap
		) 2drop ;

:waitkey
	key 27 =? ( exit ) drop ;

: 	msec
	cls
	"Se esta dibujando un fractal. Paciencia ..." print
	redraw
	mandel
	msec swap -
	$ffffff 'color !
	home
	cr
	"Se ha tardado " print .d print " ms" print cr

	'waitkey onshow ;
