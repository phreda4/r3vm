
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

we use "|" to indicate comment until the end of the line (the first exception to word separation).


```
;		| End of Word
()		| Word block to build IF and WHILE
[]		| Word block to build nameless definitions
EX		| Run a word through your address
0? 1?	| Zero and non-zero conditionals
+? -?	| Conditional positive and negative
<? >? =? | Comparison conditions
> =? <=? <>? | Comparison conditions
AN? NA? | Logical conditioners AND and NOT AND
BT?		| Conditional between
```

## Words to modify the DATA STACK
```
DUP DROP OVER PICK2
PICK3 PICK4 SWAP PIN
ROT 2DUP 2DROP 3DROP
4DROP 2OVER 2SWAP
```

## Words to modify the RETURN STACK
```
> R R> R @
```

## Logical operators
```
AND OR XOR
```

## Arithmetic Operators
```
+ - * /
<< >> >>>
MOD / MOD * / * >> << /
NOT NEG ABS SQRT CLZ
```

## Access to Memory
```
@ C @ Q @ @ + C @ + Q @ +
! C! Q! ! + C! + Q! +
+! C +! Q +!
```

## Help registers facility
```
> A A> A @ A! A + A @ + A! +
> B B> B @ B! B + B @ + B! +
```

## Copy and Memory Filling
```
MOVE MOVE> FILL
CMOVE CMOVE> CFILL
QMOVE QMOVE> QFILL
```

## Use and Interaction with the Operating System
```
UPDATE REDRAW
MEM SW SH VFRAME
XYPEN BPEN KEY CHAR
MSEC TIME DATE
LOAD SAVE APPEND
FFIRST FNEXT
SYS
```

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

## Prefixes in words

Programming occurs when we define our own words.
We can define words as actions with the prefix:

```
: addmul + *;
```

or data with the prefix #

```
#lives 3
```

## The complete list of prefixes is as follows:

* `|` ignored until the end of the line, this is a comment
* `^` the name of the file to be included is taken until the end of the line, this allows names * with space.
* `"` the end of quotation marks is searched to delimit the content, if there is a double quotation mark" * "it is taken as a quotation mark included in the string.
* `:` define action
* `::` define action and this definition prevails when a file is included (* exported)
* `#` define data
* `##` define exported data
* `$` define hexadecimal number
* `%` defines binary number, allows the. like 0
* `'` means the direction of a word and what it does is to stack in the DATA BATTERY the address, of the word, whether it is a DATA or an ACTION, it should be clarified that the words of the BASIC DICTIONARY have NO address, but those defined by the programmer, yes.

`: ` it is only the beginning of the program, a complete program in r3 can be the following

```
: sum3 dup dup + +;

: 3 sum3;
```
