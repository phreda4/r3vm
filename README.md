
# r3

r3 is a concatenative language of the forth family, more precisely it takes elements of the ColorForth, the colors that have the words internally are encoded by a prefix, in r3 this prefix is explicit.

This is the virtual machine, load main.r3, compile in bytecodes (really are dwordcodes) and interpret this code.

## compilation

### Windows

Change in r3.cpp comment all
```
//#define DEBUGWORD
//#define VIDEOWORD
//#define LINUX
//#define RPI   // Tested on a Raspberry PI 4
```

Compile with devcpp(5.11), is a GCC for windows, with SDL and SDL_MIXER packages.


### Linux

Change in r3.cpp uncomment LINUX
```
//#define DEBUGWORD
//#define VIDEOWORD
#define LINUX
//#define RPI   // Tested on a Raspberry PI 4
```
I do this with, Xubuntu 20, with GCC, need instaled SDL and SDL_MIXER

```
$ make clean
$ make 
$ chmod +x ./r3lin
```

### EMSCRIPTEN

Can compile in emscripten too. emscriptenbuild.sh is the make.

### MAC

To compile the mac version:

A separate files, r3mac.cpp and r3graf.cpp.

```
c++  -Ofast -fpermissive   -c -o grafmac.o grafmac.cpp
c++  -Ofast -fpermissive   -c -o r3mac.o r3mac.cpp
g++ grafmac.o r3mac.o  -o r3mac -lSDL2main -lSDL2 -lSDL2_mixer
```

compiler suite and SDL2* installed through brew.
