/home/phr/emsdk/upstream/emscripten/em++ -O3 -s USE_SDL=2 -s USE_SDL_MIXER=2 graf.cpp -o graf.bc
/home/phr/emsdk/upstream/emscripten/em++ -O3 -s USE_SDL=2 -s USE_SDL_MIXER=2 r3.cpp -o r3.bc
/home/phr/emsdk/upstream/emscripten/em++ -O3 -s USE_SDL=2 -s USE_SDL_MIXER=2 r3.bc graf.bc --preload-file main.r3  -o r3web.html

