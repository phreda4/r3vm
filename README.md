
# r3

r3 is a concatenative language of the forth family, more precisely it takes elements of the ColorForth, the colors that have the words internally are encoded by a prefix, in r3 this prefix is explicit.

## How language work

WORD is defined as a sequence of letters separated by spaces, there are three exceptions to improve the expressiveness of the language that are seen later

Each word can be a number or is searched in the DICTIONARY.

If it is a valid number, in decimal, binary (%), hexa ($) or fixed point (0.1) this value is pushed to DATA STACK.

Like all FORTH, the DATA STACK is the memory structure used to perform intermediate calculations and pass parameters between words.

If the word is NOT a valid number, then it is searched in the DICTIONARY, if it is not found, it is an ERROR, the rule is "every word used must be defined before".

The language has a BASIC DICTIONARY that is already defined, from which new WORDS are defined that will be used to build the program.

## BASE word list:

Each word can take and/or leave values of the DATA STACK, this is expressed with a state diagram of the stack before -- and after the word.

for example

```
+ | a b -- c
```
the word + takes the two elements from the top of the DATA STACK, consumes them and leaves the result.
In addition to a modification in the stack, there may also be a lateral action, for example:

```
REDRAW | --
```
It does not consume or produce values in the stack but it updates the graphic screen with the buffer in memory, this is a side effect.

we use "|" to indicate comment until the end of the line (the first exception to word separation).


```
;		| End of Word
(  )	| Word block to build IF and WHILE
[  ]	| Word block to build nameless definitions
EX		| Run a word through your address
```

## Conditional, together with blocks make the control structures
```
0? 1?		| Zero and non-zero conditionals
+? -?		| Conditional positive and negative
<? >? =?	| Comparison conditions
> =? <=? <>? | Comparison conditions
AN? NA?		| Logical conditioners AND and NOT AND
BT?			| Conditional between
```

## Words to modify the DATA STACK
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

## Words to modify the RETURN STACK
```
>R		| a --		; r: -- a
R>		| -- a 		; r: a --
R@		| -- a 		; r: a -- a
```

## Logical operators
```
AND		| a b -- c
OR		| a b -- c
XOR		| a b -- c
NOT		| a -- b
```

## Arithmetic Operators
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

## Access to Memory

`@` fetch a value from memory
`!` store a value in memory

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

## Help registers facility

Registers for keep values to traverse memory and read, copy or fill values

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

## Copy and Memory Filling

Block memory operation, only for data memory

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

## Use and Interaction with the Operating System

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

## Prefixes in words

* `|` ignored until the end of the line, this is a comment
* `^` the name of the file to be included is taken until the end of the line, this allows names * with space.
* `"` the end of quotation marks is searched to delimit the content, if there is a double quotation mark" * "it is taken as a quotation mark included in the string.
* `:` define action
* `::` define action and this definition prevails when a file is included (* exported)
* `#` define data
* `##` define exported data
* `$` define hexadecimal number
* `%` defines binary number, allows the. like 0
* `'` means the direction of a word, this address is pushed to DATA STACK, it should be clarified that the words of the BASIC DICTIONARY have NO address, but those defined by the programmer, yes.

Programming occurs when we define our own words.
We can define words as actions with the prefix:

```
: addmul + * ;
```

or data with the prefix #

```
#lives 3
```

`: ` it is only the beginning of the program, a complete program in r3 can be the following

```
: sum3 dup dup + + ;

: 2 sum3 ;
```


## Conditional and Repeat

The way to build conditionals and repetitions is through the words `(` and `)`

for example:
```
5 >? ( drop 5 )
```

The meaning of these 6 words is: check the top of the stack with 5, if it is greater, remove this value and stack a 5.

The condition produces a jump at the end of the code block if it is not met. It becomes an IF block.

r3 identifies this construction when there is a conditional word before the word `(`. If this does not happen the block represents a repetition and, a conditional in that this repetition that is not an IF is used with the WHILE condition.

for example:
```
1 ( 10 <?
	1 + ) drop
```

account from 1 to 9, while the conditional is being fulfilled
