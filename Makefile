
CPP      = g++
CC       = gcc
OBJ      = graf.o r3.o 
LINKOBJ  = graf.o r3.o 
LIBS     = -lSDL2main -lSDL2 -lSDL2_mixer -s
INCS     = 
CXXINCS  = 
BIN      = r3lin
CXXFLAGS = $(CXXINCS) -Ofast -fpermissive
CFLAGS   = $(INCS) -Ofast -Dmain=SDL_main
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

graf.o: graf.cpp
	$(CPP) -c graf.cpp -o graf.o $(CXXFLAGS)

r3.o: r3.cpp
	$(CPP) -c r3.cpp -o r3.o $(CXXFLAGS)


