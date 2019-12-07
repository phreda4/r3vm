# r3

r3 es un lenguaje concatenativo de la familia de forth, mas precisamente toma elementos del ColorForth, los colores que tienen las palabras internamentes estan codificadas por un prefijo, en r3 este prefijo es explicito.

## El funcionamento del lenguaje:

Se define PALABRA como una secuencia de letras separada por espacios, hay tres exepciones para mejorar la expresividad del lenguaje que se ven mas adelante

Cada palabra puede ser un numero o es buscada en el DICCIONARIO.

Si es un numero valido, en decimal, binario (%), hexa ($) o punto fijo (0.1) este valor es apilado en la PILA DE DATOS.

Como todo FORTH, la PILA DE DATOS es la estructura de memoria utilizada para realizar calculos intermedios y pasar parametros entre palabras.

Si la palabra NO es un numero valido, entonces se busca en el DICCIONARIO, si no se encuentra, es un ERROR, la regla es "toda palabra usada debe estar definida antes".

El lenguaje posee un DICCIONARIO BASE que ya esta definido, a partir del cual se van definiendo nuevas PALABRAS que se iran usando para construir el programa.

## lista de palabras BASE:

usamos "|" para indicar comentario hasta el fin de la linea (la primera excepcion a la separacion de las palabras)

```
;			| Fin de Palabra
( )			| Bloque de palabras para construir IF y WHILE
[ ]			| Bloque de palabras para construir definiciones sin nombre
EX			| Ejecutar una palabra a travez de su direccion
0? 1?		| Condicionales cero y no cero
+? -?		| Condicionales positivo y negativo
<? >? =?	| Condicionales de comparacion
>=? <=? <>?	| Condicionales de comparacion
AN? NA?		| Condicionales logicos AND y NOT AND
BT?			| Condicional de entre
```

## Palabras para modificar la PILA DE DATOS
```
DUP DROP OVER PICK2
PICK3 PICK4 SWAP NIP
ROT 2DUP 2DROP 3DROP
4DROP 2OVER 2SWAP
```

## Palabras para modificar la PILA DE EJECUCION
```
>R R> R@
```

## Operadores Logicos
```
AND OR XOR
```

## Operadores Aritmeticos
```
+ - * /
<< >> >>>
MOD /MOD */ *>> <</
NOT NEG ABS SQRT CLZ
```

## Acceso a Memoria
```
@ C@ Q@ @+ C@+ Q@+
! C! Q! !+ C!+ Q!+
+! C+! Q+!
```

## Registros de ayuda
```
>A A> A@ A! A+ A@+ A!+
>B B> B@ B! B+ B@+ B!+
```

## Copia y Llenado de Memoria
```
MOVE MOVE> FILL
CMOVE CMOVE> CFILL
QMOVE QMOVE> QFILL
```

## Uso e Interaccion con el Sistema Operativo
```
UPDATE REDRAW
MEM SW SH VFRAME
XYPEN BPEN KEY CHAR
MSEC TIME DATE
LOAD SAVE APPEND
FFIRST FNEXT
SYS
```

Cada palabra puede tomar y dejar valores en la PILA DE DATOS, se expresa esto con un diagrama de estado de la pila antes -- y despues de la palabra.

por ejemplo

```
+ | a b -- c
```
la palabra + toma los dos elementos del tope de la PILA DE DATOS, los consume y deja el resultado.
Ademas de una modificacion en la pila, tambien puede haber un accion lateral, por ejemplo:

```
REDRAW | --
```
No consume ni produce valores en la pila pero actualiza la pantalla grafica con el buffer en memoria.

## Prefijos en las palabras

La programacion ocurre cuando definimos nuestras propias palabras.
Podemos definir palabras como acciones con el prefijo :

```
:addmul  + * ;
```

o datos con el prefijo #

```
#lives 3
```

## La lista completa de prefijos es la siguiente:

* `|` se ignora hasta el fin de linea, esto es un comentario
* `^` se toma hasta el fin de linea el nombre del archivo a incluir, esto permite nombres * con espacio.
* `"` se busca el fin de comillas para delimitar el contenido, si hay una doble comilla "* " se toma como una comilla incluida en el string.
* `:` define accion
* `::` define accion y prevalece esta definicion cuando una archivo es incluido (* exportada)
* `#` define dato
* `##` define dato exportado
* `$` define numero hexadecimal
* `%` define numero binario, permite el . como 0
* `'` significa direccion de una palabra y lo que hace es apilar en la PILA DE DATOS la DIRECCION, de la palabra, tanto si es un DATO o una ACCION, hay que aclarar que las palabras del DICCIONARIO BASE NO tienen direccion, pero las definidas por el programador, si.

`: ` solo es el commienzo del programa, un programa completo en r3 puede ser el siguiente

```
:sum3 dup dup  + + ;

: 3 sum3 ;
```


