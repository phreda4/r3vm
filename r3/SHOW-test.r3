| Example 4

^r3/lib/gr.r3

|--------------------
#x 10 #vx 1
#y 10 #vy 1

:hitx vx neg 'vx ! ;
:hity vy neg 'vy ! ;

:box | x y --
 xy>v >a $ffffff a! ;

:show
 cls
 x sw 4 - >=? ( hitx ) 4 <=? ( hitx )
 y sh 4 - >=? ( hity ) 4 <=? ( hity )
 box
 vx 'x +!
 vy 'y +!

 key
 >esc< =? ( exit )
 drop
 ;

|--------------------
:
'show onshow
;
