// Pablo Hugo Reda <pabloreda@gmail.com>
// rutinas graficas

#ifndef GRAF_H
#define GRAF_H

#ifdef __cplusplus
extern "C" {
#endif
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_mixer.h>
#ifdef __cplusplus
}
#endif

extern SDL_Window *window;
//extern SDL_Renderer *renderer;
//extern SDL_Texture *texture;

extern Uint32 *gr_buffer; // buffer de pantalla

extern int gr_ancho,gr_alto;
extern Uint32 gr_color1,gr_color2,col1,col2;
extern int MA,MB,MTX,MTY; // matrix de transformacion
extern int *mTex; // textura

int gr_init(char *title,int XRES,int YRES,int f);
void gr_fin(void);
void gr_cls(int color);
void gr_redraw(void);
//---- lineas rectas
void gr_hline(int x1,int y1,int x2);
void gr_vline(int x1,int y1,int y2);
//---- basicas
void gr_setpixel(int x,int y);
void gr_setpixela(int x,int y);
void gr_line(int x1,int y1,int x2,int y2);
void gr_spline(int x1,int y1,int x2,int y2,int x3,int y3);
void gr_spline3(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4);
//---- ALPHA
void gr_solid(void);
void gr_alpha(int a);
//---- FILL POLY
void fillSol(void);
void fillLin(void);
void fillRad(void);
void fillTex(void);
//---- matriz transf
inline void fillcent(int mx,int my)     { MTX=mx;MTY=my; }
inline void fillmat(int a,int b)        { MA=a;MB=b; }
inline void fillcol(Uint32 c1,Uint32 c2)  { col1=c1;col2=c2; }

//---- poligono
void gr_psegmento(int x1,int y1,int x2,int y2);
void gr_pspline(int x1,int y1,int x2,int y2,int x3,int y3);
void gr_pspline3(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4);

//#define LLQ
//#define LOWQ
#define MEDQ
//#define HIQ

//////////////////////////////////////////////////////////////
#ifdef HIQ
      #define BPP        4 
      #define TOLERANCE  24
      #define QALPHA(a)  (a)
#elif defined(MEDQ)
      #define BPP        3 
      #define TOLERANCE  12
      #define QALPHA(a)  ((a)&0x3|(a)<<2)
#elif defined(LOWQ)
      #define BPP        2 
      #define TOLERANCE  8
      #define QALPHA(a)  ((a)<<4|(a))      
#else 
      #define BPP        1
      #define TOLERANCE  4
      #define QALPHA(a)  ((a)<<6|(a)<<4|(a)<<2|(a))      
#endif

#define VALUES     (1<<BPP)
#define QFULL       VALUES*VALUES
#define MASK       (VALUES-1)
#define FTOI(v)    (v<<BPP)

inline void gr_pline(int x1,int y1,int x2,int y2)
{ gr_psegmento(FTOI(x1),FTOI(y1),FTOI(x2),FTOI(y2)); }
inline void gr_pcurve(int x1,int y1,int x2,int y2,int x3,int y3)
{ gr_pspline(FTOI(x1),FTOI(y1),FTOI(x2),FTOI(y2),FTOI(x3),FTOI(y3)); }
inline void gr_pcurve3(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4)
{ gr_pspline3(FTOI(x1),FTOI(y1),FTOI(x2),FTOI(y2),FTOI(x3),FTOI(y3),FTOI(x4),FTOI(y4)); }

void gr_drawPoli(void);	

#endif
