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

Usamos "|" para indicar comentario hasta el fin de la linea (la primera excepcion a la separacion de las palabras)

```
;			| Fin de Palabra
(  )		| Bloque de palabras para construir IF y WHILE
[  ]		| Bloque de palabras para construir definiciones sin nombre
EX			| Ejecutar una palabra a travez de su direccion
```

## Condicionales, junto con bloques hacen las estructuras de control
```
0? 1?		| Condicionales cero y no cero
+? -?		| Condicionales positivo y negativo
<? >? =?	| Condicionales de comparacion
>=? <=? <>?	| Condicionales de comparacion
AN? NA?		| Condicionales logicos AND y NOT AND
BT?			| Condicional de entre
```

## Palabras para modificar la PILA DE DATOS
```
DUP		| a -- a a
DROP	| a --
OVER	| a b -- a b a
PICK2	| a b c -- a b c a
PICK3	| a b c d -- a b c d a
PICK4	| a b c d e -- a b c d e a
SWAP	| a b -- b a
NIP		| a b -- b
ROT		| a b c -- b c a
2DUP	| a b -- a b a b
2DROP	| a b --
3DROP	| a b c --
4DROP	| a b c d --
2OVER	| a b c d -- a b c d a b
2SWAP	| a b c d -- c d a b
```

## Palabras para modificar la PILA DE EJECUCION
```
>R		| a --		; r: -- a
R>		| -- a 		; r: a --
R@		| -- a 		; r: a -- a
```

## Operadores Logicos
```
AND		| a b -- c
OR		| a b -- c
XOR		| a b -- c
NOT		| a -- b
```

## Operadores Aritmeticos
```
+		| a b -- c
-		| a b -- c
*		| a b -- c
/		| a b -- c
<<		| a b -- c
>>		| a b -- c
>>>		| a b -- c
MOD		| a b -- c
/MOD	| a b -- c d
*/		| a b c -- d
*>>		| a b c -- d
<</		| a b c -- d
NEG		| a -- b
ABS		| a -- b
SQRT	| a -- b
CLZ		| a -- b
```

## Acceso a Memoria

`@` fetch, lee un valor de memoria
`!` store, graba un valor en memoria

```
@		| a -- [a]
C@		| a -- b[a]
Q@		| a -- q[a]
@+		| a -- b [a]
C@+		| a -- b b[a]
Q@+		| a -- b q[a]
!		| a b --
C!		| a b --
Q!		| a b --
!+		| a b -- c
C!+		| a b -- c
Q!+		| a b -- c
+!		| a b --
C+!		| a b --
Q+!		| a b --
```

## Registros de ayuda

Registro para recorrer la memoria para leer,copiar o llenar.

```
>A		| a --
A>		| -- a
A@		| -- a
A!		| a --
A+		| a --
A@+		| -- a
A!+		| a --
>B		| a --
B>      | -- a
B@		| -- a
B!		| a --
B+      | a --
B@+     | -- a
B!+     | a --
```

## Copia y Llenado de Memoria
```
MOVE	| a b c --
MOVE>	| a b c --
FILL	| a b c --
CMOVE	| a b c --
CMOVE>	| a b c --
CFILL	| a b c --
QMOVE	| a b c --
QMOVE>	| a b c --
QFILL	| a b c --
```

## Uso e Interaccion con el Sistema Operativo
```
UPDATE	| --
REDRAW	| --
MEM		| -- a
SW		| -- a
SH		| -- a
VFRAME	| -- a
XYPEN	| -- a b
BPEN	| -- a
KEY		| -- a
CHAR	| -- a
MSEC	| -- a
TIME	| -- a
DATE	| -- a
LOAD	| a b c --
SAVE	| a b c --
APPEND	| a b c --
FFIRST	| a -- b
FNEXT	| a -- b
SYS		| a --
```

## Prefijos en las palabras

* `|` se ignora hasta el fin de linea, esto es un comentario
* `^` se toma hasta el fin de linea el nombre del archivo a incluir, esto permite nombres * con espacio.
* `"` se busca el fin de comillas para delimitar el contenido, si hay una doble comilla "* " se toma como una comilla incluida en el string.
* `:` define accion
* `::` define accion y prevalece esta definicion cuando una archivo es incluido (* exportada)
* `#` define dato
* `##` define dato exportado
* `$` define numero hexadecimal
* `%` define numero binario, permite el . como 0
* `'` significa direccion de una palabra, esta se apila en la PILA DE DATOS, hay que aclarar que las palabras del DICCIONARIO BASE NO tienen direccion, pero las definidas por el programador, si.

La programacion ocurre cuando definimos nuestras propias palabras.
Podemos definir palabras como acciones con el prefijo :

```
:addmul  + * ;
```

o datos con el prefijo #

```
#lives 3
```


`: ` solo es el commienzo del programa, un programa completo en r3 puede ser el siguiente

```
:sum3 dup dup  + + ;

: 2 sum3 ;
```

## Condicionales y Repeticion

La forma de construir condicionales y repeticiones es a travez de las palabra ( y )

por ejemplo:
```
5 >? ( drop 5 )
```

El significado de estas 6 palabras es: comprobar el tope de la pila con 5, si es mayor, quitar este valor y apilar un 5.

La condicion produce un salto al final del bloque de codigo si no se cumple. Se combierte en un bloque IF.

r3 identifica esta construccion cuando hay una palabra condicional antes de la palabra `(`. Si esto no pasa el bloque representa una repeticion y, un condicional adentro que esta repeticion que no sea un IF se utiliza con condicion de mientras WHILE.

por ejemplo:
```
1 ( 10 <?
	1 + ) drop
```

cuenta de 1 a 9, mientras se cumple el condicional



