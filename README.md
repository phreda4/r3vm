
# r3

r3 is a concatenative language of the forth family, more precisely it takes elements of the ColorForth, the colors that have the words internally are encoded by a prefix, in r3 this prefix is explicit.

This is the virtual machine, load main.r3, compile in bytecodes (really are dwordcodes) and interpret this code.

In windows this program compile with devcpp(5.11) with SDL and SDL_MIXER packages.
In Linux, Xubuntu 20, with gcc, need instaled SDL and SDL_MIXER, run "make".
Can compile in emscripten too. emscriptenbuild.sh is the make.

To compile the mac version:

c++  -Ofast -fpermissive   -c -o grafmac.o grafmac.cpp
c++  -Ofast -fpermissive   -c -o r3mac.o r3mac.cpp
g++ grafmac.o r3mac.o  -o r3mac -lSDL2main -lSDL2 -lSDL2_mixer

compiler suite and SDL2* installed through brew.
