| Example 1

^lib/gr.r3

:patternxor
 vframe >a
 sh ( 1? 1 -
  sw ( 1? 1 -
    2dup xor msec + 8 <<
	a!+
    ) drop
  ) drop
  key 27 =? ( exit ) drop
  ;

:
 'patternxor onshow
;
