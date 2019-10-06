| r3 lib string
| PHREDA 2018

#mbuff * 64

:mbuffi | -- adr
  'mbuff 63 + 0 over c! 1 - ;

:sign | adr sign -- adr'
  -? ( drop $2d over c! ; ) drop 1 + ;

::.d | val -- str
  dup abs mbuffi swap
  ( 10 /mod $30 + pick2 c! swap 1 - swap 1? ) drop
  swap sign ;

::.b | bin -- str
  mbuffi swap
  ( dup $1 and $30 + pick2 c! swap 1 - swap 1 >>> 1? ) drop
  1 + ;

::.h | hex -- str
  mbuffi swap
  ( dup $f and $30 + $39 >? ( 7 + ) pick2 c! swap 1 - swap 4 >>> 1? ) drop
  1 + ;

::.f | fix --
  mbuffi over
  $ffff and 10000 16 *>> 10000 +
  ( 10 /mod $30 + pick2 c! swap 1 - swap 1? ) drop
  1 + $2e over c! 1 -
  over 16 >>
  ( 10 /mod $30 + pick2 c! swap 1 - swap 1? ) drop
  swap sign ;

