#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
//#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_audio.h>
#ifdef __cplusplus
}
#endif

#include "graf.h"


//----------------------
/*------COMPILER------*/
//----------------------
// 0-imm 1-code 2-data 3-reserve 4-bytes 5-qwords
int modo=0; 

int nerror=0;	
int lerror=0;
int cerror=0;
char *werror;

int boot=-1;

int memc=0;
char memcode[0xfffff]; // 1MB

int memd=0;
int meminidata=0;
char memdata[0xffffff]; // 16Mb data total

char path[1024];

int base;
int nro=0;

//---- includes
struct Include { char *nombre;char *str; };

int cntincludes;
Include includes[128];
int cntstacki;
int stacki[128];

//---- diccionario local
struct Indice {	char *nombre;int mem;int info; };

int cntdicc;
int dicclocal;
Indice dicc[8192];

int level;
int cntstacka;
int stacka[256];

void iniA(void) { cntstacka=0; }
void pushA(int n) { stacka[cntstacka++]=n; }
int popA(void) { return stacka[--cntstacka]; }

const char *r3mac[]={
"nop",":","::","#","##","|","^", 	// 6
"Nd","Nh","Nb","Nf","str", 			// 11
"call","var","dcode","ddata", 		// 15
//";","jmp","jmpw","[","]"
};

const char *r3bas[]={
";","(",")","[","]",
"EX","0?","1?","+?","-?", 									// 10
"<?",">?","=?",">=?","<=?","<>?","AN?","NA?","BTW?", 		// 19
"DUP","DROP","OVER","PICK2","PICK3","PICK4","SWAP","NIP", 	// 27
"ROT","2DUP","2DROP","3DROP","4DROP","2OVER","2SWAP", 		// 34
">R","R>","R@",												// 37
"AND","OR","XOR","NOT","NEG", 								// 42
"+","-","*","/","*/", 										// 47
"/MOD","MOD","ABS","SQRT","CLZ", 							// 52
"<<",">>",">>>","*>>","<</", 								// 57
"@","C@","Q@","@+","C@+","Q@+",  							// 65
"!","C!","Q!","!+","C!+","Q!+",  							// 71
"+!","C+!","Q+!",  											// 74
">A","A>","A@","A!","A+","A@+","A!+", 						// 81
">B","B>","B@","B!","B+","B@+","B!+", 						// 88
"MOVE","MOVE>","FILL", 										// 91
"CMOVE","CMOVE>","CFILL", 									// 94
"QMOVE","QMOVE>","QFILL", 									// 97

"UPDATE","REDRAW",
"MEM","SW","SH","VFRAME",
"XYPEN","BPEN","KEY",
"MSEC","TIME","DATE",
"LOAD","SAVE","APPEND",
"FFIRST","FNEXT",

"SYSCALL","SYSMEM",
"SYS",""
};

//////////////////////////////////////
//////////////////////////////////////
void printword(char *s)
{
while (*s>32) putchar(*s++);
putchar(' ');
}
//////////////////////////////////////
//////////////////////////////////////

int isNro(char *p)
{//if (*p=='&') { p++;numero=*p;return 1;} // codigo ascii
int dig=0,signo=0;
if (*p=='-') { p++;signo=1; } else if (*p=='+') p++;
if (*p==0) return 0;// no es numero
switch(*p) {
  case '$': base=16;p++;break;// hexa
  case '%': base=2;p++;break;// binario
  default:  base=10;break; }; 
nro=0;if (*p==0) return 0;// no es numero
while ((unsigned char)*p>32) {
  if (*p<='9') dig=*p-'0'; 
  else if (*p>='A') dig=*p-'A'+10;
//  else if (*p=='.') dig=0; 
  else return 0;
  if (dig<0 || dig>=base) return 0;
  nro*=base;nro+=dig;
  p++;
  };
if (signo==1) nro=-nro;  
return -1; 
};

int isNrof(char *p)         // decimal punto fijo 16.16
{
int64_t parte0;
int dig=0,signo=0;
if (*p=='-') { p++;signo=1; } else if (*p=='+') p++;
if (*p==0) return 0;// no es numero
nro=0;
while ((unsigned char)*p>32) {
  if (*p=='.') { parte0=nro;nro=1;if (nro==0 && *(p+1)<33) return 0; } 
  else  {
  	if (*p<='9') dig=*p-'0'; else dig=-1;
  	if (dig<0 || dig>=10) return 0;
  	nro=(nro*10)+dig;
  	}
  p++;
  };  
int decim=1,num=nro;
while (num>1) { decim*=10;num/=10; }
num=0x10000*nro/decim;
nro=(num&0xffff)|(parte0<<16);
if (signo==1) nro=-nro;
return -1; 
};

char toupp(char c)
{ 
return c&0xdf;
}
	
int strequal(char *s1,char *s2)
{
while ((unsigned char)*s1>32) {
	if (toupp(*s2++)!=toupp(*s1++)) return 0;
	}
if (((unsigned char)*s2)>32) return 0;
return -1;
}
	
char *trim(char *s)	
{
while (((unsigned char)*s)<33) s++;
return s;
}

char *nextw(char *s)	
{
while (((unsigned char)*s)>32) s++;
return s;
}

char *nextcr(char *s)	
{
while (((unsigned char)*s)>31) s++;
return s;
}

char *nextstr(char *s)
{
while (*s!=0)	{
	if (*s==34) { s++;if (*s!=34) { s++;break; } }
	s++;
	}
return s;
}

int isBas(char *p)
{    
nro=0;
char **m=(char**)r3bas;
while (**m!=0) {
	//printword(*m);
  if (strequal(*m,p)) return -1;
  *m++;nro++; }
return 0;  
};

int isWord(char *p) 
{ 
int i=cntdicc;
while (--i) { 
	if (strequal(dicc[i].nombre,p) && ((dicc[i].info&1)==1 || i>=dicclocal)) { break; } 
	}
return i;
};

void codetok(int nro) { 
memcode[memc++]=nro; 
}

void closevar() 
{
if (cntdicc==0) { return; }
if ((dicc[cntdicc-1].info&0x10)==0) { return; } // prev is var
if (dicc[cntdicc].mem<memd) { return; } // have val
*(int*)memdata[memd]=0;memd+=4;
}

void compilaDATA(char *str) 
{ 
int ex=0;
closevar();
if (*(str+1)=='#') { ex=1;str++; }
dicc[cntdicc].nombre=str+1;
dicc[cntdicc].mem=memd;
dicc[cntdicc].info=ex+0x10;	// 0x10 es dato
cntdicc++;
modo=2;
}

	
void compilaCODE(char *str) 
{ 
int ex=0;
closevar();
if (*(str+1)==':') { ex=1;str++; }
if (*(str+1)<33) { boot=memc; }
dicc[cntdicc].nombre=str+1;
dicc[cntdicc].mem=memc;
dicc[cntdicc].info=ex;	// 0x10 es dato
cntdicc++;
modo=1;
}


void datanro(int n) { 
//printf("nn %d %d ",modo,n);
switch(modo){
	case 2:*(int*)memdata[memd]=(int)n;memd+=4;break;
	case 3:for(int i=0;i<n;i++) { memdata[memd++]=0; };break;
	case 4:memdata[memd]=(char)n;memd+=1;break;
	case 5:*(uint64_t*)memdata[memd]=(uint64_t)n;memd+=8;break;
	}
}

void compilaADDR(int n) 
{
if (modo>1) { datanro(dicc[n].mem);return; }
codetok((dicc[n].mem<<7)+14+((dicc[n].info>>4)&1)); 
}


void compilaLIT(int n) 
{
//printf("lit %d_",n);

if (modo>1) { datanro(n);return; }
//	if (n>-257 && n<256) { codetok(((n<<7)&0xff80)+7);return; }
if (n==(n<<6)>>6) { // un bit mas por signo (token 8 y 9)
//		codetok((n<<7)+8+((n>>25)&1));
	codetok((n<<7)+8);
	return;
	} 
codetok((n&0xffffff80)+9); // falta cte en mem
codetok(((n&0x7f)<<7)+10); // falta cte en mem	
}

int datasave(char *str) 
{
int r=memd;
for(;*str!=0;str++) { 
	if (*str==34) { str++;if (*str!=34) break; }
	memdata[memd++]=*str;
	}
memdata[memd++]=0;	
return r;
}

void compilaSTR(char *str) 
{
int ini=datasave(str);	
if (modo<2) {codetok((ini<<7)+11);}
}


void blockIn(void)
{
pushA(memc);
level++;
}

int solvejmp(int from,int to) 
{
int whi=false;
for (int i=from;i<to;i++) {
	unsigned char op=memcode[i];
	if (op>21 && op<35 && (memcode[i]>>7)==0) {
		memcode[i]+=(memc-i)<<7;	// patch while
		whi=true;
		}
	}
return whi;
}

void blockOut(void)
{
int from=popA();
int dist=memc-from;
if (solvejmp(from,memc)) { // salta
	codetok((-(dist+1)<<7)+18); 	// jmpr
} else {
	memcode[from-1]+=(dist<<7);		// patch if
	}
level--;
}

void anonIn(void)
{
	
}

void anonOut(void)
{
}

void dataMAC(int n)
{
//printf("mac %d ",n);
if (n==44) {modo=3;} // * reserva bytes
if (n==1) {modo=4;} // (	bytes
if (n==2) {modo=2;} // )
if (n==3) {modo=5;} // [	qwords
if (n==4) {modo=2;} // ]
}

void compilaMAC(int n) 
{
if (modo>1) { dataMAC(n);return; }
if (n==1) { blockIn();return; }		//(	etiqueta
if (n==2) { blockOut();return; }	//)	salto
if (n==3) { anonIn();return; }		//[	salto:etiqueta
if (n==4) { anonOut();return; }		//]	etiqueta;push
codetok(n+16);	
if (n==0) { if (level==0) {modo=0;} } // ;
}

void compilaWORD(int n) 
{
if (modo>1) { datanro(n);return; }
codetok((dicc[n].mem<<7)+12+((dicc[n].info>>4)&1));
}


int r3token(char *str) 
{
printf("token...\n")	;
level=0;
while(*str!=0) {
	str=trim(str);
	printword(str);printf("\n");
	switch (*str) {
		case '^':	// include
			str=nextcr(str);break;
		case '|':	// comments	
			str=nextcr(str);break; 
		case '"':	// strings		
			compilaSTR(str);str=nextstr(str);break;
		case ':':	// $3a :  Definicion	// :CODE
			compilaCODE(str);str=nextw(str);break;
		case '#':	// $23 #  Variable	// #DATA
			compilaDATA(str);str=nextw(str);break;	
		case 0x27:	// $27 ' Direccion	// 'ADR
			nro=isWord(str);
			if (nro<0) return 2;
			compilaADDR(nro);
			str=nextw(str);
			break;		
		default:
			if (isNro(str)) { compilaLIT(nro);str=nextw(str);break; }
			if (isBas(str)) { compilaMAC(nro);str=nextw(str);break; }
			nro=isWord(str);
			if (nro<0) return 0; 
			if (modo==2) { compilaADDR(nro);str=nextw(str);break; }
			compilaWORD(nro);
			str=nextw(str);
			break;
		}
	}
if (memcode[memc-1]!=16) { memcode[memc++]=16; } // last;
return -1;
}

char *openfile(char *filename)
{
//printf("loading %s\n",filename);

long len;
char *buff;
FILE *f=fopen(filename,"rb");	
if (!f) return 0;
fseek(f,0,SEEK_END);len=ftell(f);fseek(f,0,SEEK_SET);
buff=(char*)malloc(len+1);
if (!buff) return 0;
fread(buff,1,len,f); 
fclose(f);
buff[len]=0;
return buff;
/*	
int fd=open(name,O_RDONLY);
int len=lseek(fd,0,SEEK_END);
void *data=mmap(0,len,PROT_READ,MAP_PRIVATE,fd,0);
*/  
}

int isinclude(char *str)
{
//printf("inc\n");	
char filename[1024];
char *fn=filename;	
char *ns=str;	

//if (*str=='.') {
	strcpy(filename,path);
	while (*fn!=0) fn++;
//	}
while ((unsigned char)*str>31) { *fn++=*str++; }
*fn=0;
//printf("[%s]",filename);
for (int i=0;i<cntincludes;i++){
	if (strequal(includes[i].nombre,filename)) return -1;
	}
includes[cntincludes].nombre=ns; // ./coso y coso son distintos !!
includes[cntincludes].str=openfile(filename); 
cntincludes++;	
return cntincludes-1;
}

void r3includes(char *str) 
{
int ninc;	
while(*str!=0) {
//	printf(str);
	str=trim(str);
	switch (*str) {
		case '^':	// include
			//printf(str);
			ninc=isinclude(str+1);
			if (ninc>=0) {
				r3includes(includes[ninc].str);
				stacki[cntstacki++]=ninc;
				}
			str=nextcr(str);
			break;
		case '|':	// comments	
			str=nextcr(str);break; 
		case '"':	// strings		
			str=nextstr(str);break;
		default:
			str=nextw(str);break;
		}
	}
//printf("fin\n");
return;
}

void dumpcode()
{
for(int i=0;i<memc;i++)
	printf("%d ",memcode[i]);
printf("\n");
}

int r3compile(char *name) 
{
char filename[1024];
char *str;

strcpy(path,"r3/");
sprintf(filename,"%s%s",path,name);

str=openfile(filename);
//printf(str);

meminidata=0;
cntincludes=0;
cntdicc=0;
dicclocal=0;
cntstacki=0;
r3includes(str); // load includes

boot=-1;
memc=1; // direccion 0 para null
memd=0;
// tokenize
for (int i=0;i<cntstacki;i++) {
//	printf("%d %d!",i,cntstacki);
	if (r3token(includes[stacki[i]].str)) return nerror;
//	r3token(includes[stacki[i]].str);
	dicclocal=cntdicc;
	}
	
// last tokenizer		
if (r3token(str)!=0) return nerror;
return -1;
}

	
//----------------------
/*--------RUNER--------*/
//----------------------
#ifdef __GNUC__
#define iclz32(x) __builtin_clz(x)
#else
static inline int popcnt(int x)
{
    x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);
    return x & 0x0000003f;
}
static inline int iclz32(int x)
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return 32 - popcnt(x);
}
#endif

// http://www.devmaster.net/articles/fixed-point-optimizations/
static inline int isqrt32(int value)
{
if (value==0) return 0;
int g = 0;
int bshft = (31-iclz32(value))>>1;  // spot the difference!
int b = 1<<bshft;
do {
	int temp = (g+g+b)<<bshft;
	if (value >= temp) { g += b;value -= temp;	}
	b>>=1;
} while (bshft--);
return g;
}

int sw,sh;
int xm=0;
int ym=0;
int bm=0;

int ke=0;
int kc=0;

//---------------------------//
// TOS..DSTACK--> <--RSTACK  //
//---------------------------//
int64_t stack[256];

void runr3(int adr) 
{
register int ip=adr;
register int64_t TOS=0;
register int64_t REGA=0;
register int64_t REGB=0;
register int NOS=0;
register int RTOS=255;
register unsigned char op=0;
register int W=0;
register int W1=0; 
stack[255]=0;
while(ip!=0) { 
	op=memcode[ip++]; 
	switch(op&0x7f){
	case 0:ip=stack[RTOS];RTOS++;break; 						// ;
	
	case 7: NOS++;stack[NOS]=TOS;TOS=(op<<16)>>23;break;		// LIT9
	case 8: NOS++;stack[NOS]=TOS;TOS=op>>7;break;				// LITres
	case 9: NOS++;stack[NOS]=TOS;TOS=(op&0xffffff80);break;		// LIT1
	case 10:TOS|=(op>>7)&0x7f;break;		// LIT2
	case 11:NOS++;stack[NOS]=TOS;TOS=op>>7;break;				// STR
	case 12:RTOS--;stack[RTOS]=ip;ip=op>>7;break;				// CALL
	case 13:NOS++;stack[NOS]=TOS;TOS=memdata[op>>7];break;	// VAR
	case 14:NOS++;stack[NOS]=TOS;TOS=op>>7;break;				// DWORD
	case 15:NOS++;stack[NOS]=TOS;TOS=op>>7;break;				// DVAR
	case 16:ip=stack[RTOS];RTOS++;break; 						// ;
	case 17:ip=(op>>7);break;//JMP
	case 18:ip+=(op>>7);break;//JMPR
	case 19:break;
	case 20:break;
	case 21:W=TOS;TOS=stack[NOS];NOS--;RTOS--;stack[RTOS]=ip;ip=W;break;//EX

	case 22:if (TOS!=0) {ip+=(op>>7);}; break;//ZIF
	case 23:if (TOS==0) {ip+=(op>>7);}; break;//UIF
	case 24:if (TOS<0) {ip+=(op>>7);}; break;//PIF
	case 25:if (TOS>=0) {ip+=(op>>7);}; break;//NIF
	
	case 26:if (TOS<stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;break;//IFGE
	case 27:if (TOS>stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;break;//IFLE
	case 28:if (TOS!=stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;break;//IFN
	case 29:if (TOS>=stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;break;//IFG
	case 30:if (TOS<=stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;break;//IFL
	case 31:if (TOS==stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;break;//IFNO
	case 32:if (!(TOS&stack[NOS])) {ip+=(op>>7);} TOS=stack[NOS];NOS--;break;//IFNA
	case 33:if (TOS&stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;break;//IFAN
	case 34:if (TOS<=stack[NOS]&&stack[NOS]<=stack[NOS]) {ip+=(op>>7);} 
				TOS=stack[NOS-1];NOS-=2;break;//BTW (need bit trick)

	case 35:NOS++;stack[NOS]=TOS;break;						//DUP
	case 36:TOS=stack[NOS];NOS--;break;						//DROP
	case 37:NOS++;stack[NOS]=TOS;TOS=stack[NOS-1];break;	//OVER
	case 38:NOS++;stack[NOS]=TOS;TOS=stack[NOS-2];break;	//PICK2
	case 39:NOS++;stack[NOS]=TOS;TOS=stack[NOS-3];break;	//PICK3
	case 40:NOS++;stack[NOS]=TOS;TOS=stack[NOS-4];break;	//PICK4
	case 41:W=stack[NOS];stack[NOS]=TOS;TOS=W;break;		//SWAP
	case 42:NOS--;break; 									//NIP
	case 43:W=TOS;TOS=stack[NOS-1];stack[NOS-1]=stack[NOS];stack[NOS]=W;break;//ROT
	case 44:W=stack[NOS];NOS++;stack[NOS]=TOS;NOS++;stack[NOS]=W;break;//DUP2
	case 45:NOS--;TOS=stack[NOS];NOS--;break;				//DROP2
	case 46:NOS-=2;TOS=stack[NOS];NOS--;break;				//DROP3
	case 47:NOS-=3;TOS=stack[NOS];NOS--;break;				//DROP4
	case 48:NOS++;stack[NOS]=TOS;TOS=stack[NOS-3];
			NOS++;stack[NOS]=TOS;TOS=stack[NOS-3];break;	//OVER2
	case 49:W=stack[NOS];stack[NOS]=stack[NOS-2];stack[NOS-2]=W;
			W=TOS;TOS=stack[NOS-1];stack[NOS-1]=W;break;	//SWAP2
	
	case 50:RTOS--;stack[RTOS]=TOS;TOS=stack[NOS];NOS--;break;	//>r
	case 51:NOS++;stack[NOS]=TOS;TOS=stack[RTOS];RTOS++;break;	//r>
	case 52:NOS++;stack[NOS]=TOS;TOS=stack[RTOS];break;			//r@
	
	case 53:TOS&=stack[NOS];NOS--;break;					//AND
	case 54:TOS|=stack[NOS];NOS--;break;					//OR
	case 55:TOS^=stack[NOS];NOS--;break;					//XOR
	case 56:TOS=~TOS;break;									//NOT
	case 57:TOS=-TOS;break;									//NEG
	case 58:TOS=stack[NOS]+TOS;NOS--;break;					//SUMA
	case 59:TOS=stack[NOS]-TOS;NOS--;break;					//RESTA
	case 60:TOS=stack[NOS]*TOS;NOS--;break;					//MUL
	case 61:TOS=(stack[NOS]/TOS)|0;NOS--;break;					//DIV
	case 62:TOS=((stack[NOS-1]*stack[NOS])/TOS)|0;NOS-=2;break;	//MULDIV
	case 63:W=stack[NOS]%TOS;stack[NOS]=(stack[NOS]/TOS)|0;TOS=W;break;//DIVMOD
	case 64:TOS=stack[NOS]%TOS;NOS--;break;					//MOD
	case 65:W=(TOS>>31);TOS=(TOS+W)^W;break;				//ABS
	case 66:TOS=isqrt32(TOS)|0;break;						//CSQRT
	case 67:TOS=iclz32(TOS);break;						//CLZ
	case 68:TOS=stack[NOS]<<TOS;NOS--;break;				//SAR
	case 69:TOS=stack[NOS]>>TOS;NOS--;break;				//SAL
	case 70:TOS=((unsigned)stack[NOS])>>TOS;NOS--;break;				//SHL
	case 71:TOS=(stack[NOS-1]*stack[NOS])>>TOS;NOS-=2;break;//MULSHR
	case 72:TOS=((stack[NOS-1]<<TOS)/stack[NOS])|0;NOS-=2;break;//CDIVSH
	
	case 73:TOS=*(int*)memdata[TOS];break;//@
	case 74:TOS=*(char*)memdata[TOS];break;//C@
	case 75:TOS=*(int64_t*)memdata[TOS];break;//Q@	

	case 76:NOS++;stack[NOS]=TOS+4;TOS=*(int*)memdata[TOS];break;//@+
	case 77:NOS++;stack[NOS]=TOS+1;TOS=*(char*)memdata[TOS];break;// C@+
	case 78:NOS++;stack[NOS]=TOS+8;TOS=*(int64_t*)memdata[TOS];break;//Q@+		
			
	case 79:*(int*)memdata[TOS]=stack[NOS];NOS--;TOS=stack[NOS];NOS--;break;// !
	case 80:*(char*)memdata[TOS]=stack[NOS];NOS--;TOS=stack[NOS];NOS--;break;//C!
	case 81:*(int64_t*)memdata[TOS]=stack[NOS];NOS--;TOS=stack[NOS];NOS--;break;//Q!
	
	case 82:*(int*)memdata[TOS]=stack[NOS];NOS--;TOS+=4;break;// !+
	case 83:*(char*)memdata[TOS]=stack[NOS];NOS--;TOS++;break;//C!+
	case 84:*(int64_t*)memdata[TOS]=stack[NOS];NOS--;TOS+=8;break;//Q!+
	
	case 85:*(int*)memdata[TOS]+=stack[NOS];NOS--;TOS=stack[NOS];NOS--;break;//+!
	case 86:*(char*)memdata[TOS]+=stack[NOS];NOS--;TOS=stack[NOS];NOS--;break;//C+!
	case 87:*(int64_t*)memdata[TOS]+=stack[NOS];NOS--;TOS=stack[NOS];NOS--;break;//Q+!
	
	case 88:REGA=TOS;TOS=stack[NOS];NOS--;break; //>A
	case 89:NOS++;stack[NOS]=TOS;TOS=REGA;break; //A> 
	case 90:NOS++;stack[NOS]=TOS;TOS=*(int*)memdata[REGA];break;//A@
	case 91:*(int*)memdata[REGA]=TOS;TOS=stack[NOS];NOS--;break;//A! 
	case 92:REGA+=TOS;TOS=stack[NOS];NOS--;break;//A+ 
	case 93:NOS++;stack[NOS]=TOS;TOS=*(int*)memdata[REGA];REGA+=4;break;//A@+ 
	case 94:*(int*)memdata[REGA]=TOS;TOS=stack[NOS];NOS--;REGA+=4;break;//A!+

	case 95:REGB=TOS;TOS=stack[NOS];NOS--;break; //>B
	case 96:NOS++;stack[NOS]=TOS;TOS=REGB;break; //B> 
	case 97:NOS++;stack[NOS]=TOS;TOS=*(int*)memdata[REGB];break;//B@
	case 98:*(int*)memdata[REGB]=TOS;TOS=stack[NOS];NOS--;break;//B! 
	case 99:REGB+=TOS;TOS=stack[NOS];NOS--;break;//B+ 
	case 100:NOS++;stack[NOS]=TOS;TOS=*(int*)memdata[REGB];REGB+=4;break;//B@+ 
	case 101:*(int*)memdata[REGB]=TOS;TOS=stack[NOS];NOS--;REGB+=4;break;//B!+
/*
	case 102://MOVE 
		W=&memdata[stack[NOS-1]];W1=&memdata[stack[NOS]];
		while (TOS--) { *W=*W1;W+=4;W1+=4; }
		NOS-=2;TOS=stack[NOS];NOS--;break;
	case 103://MOVE> 
		W=memdata[stack[NOS-1]+(TOS<<2)];W1=memdata[stack[NOS]+(TOS<<2)];
		while (TOS--) { W-=4;W1-=4;*W=*W1; }
		NOS-=2;TOS=stack[NOS];NOS--;break;
	case 104://FILL
		W1=stack[NOS-1];W=stack[NOS];
		while (TOS--) { mem.setInt32(W,W1);W+=4; }
		NOS-=2;TOS=stack[NOS];NOS--;break;
	case 105://CMOVE 
		W=stack[NOS-1];W1=stack[NOS];
		while (TOS--) { mem.setInt8(W,mem.getInt8(W1));W++;W1++; }
		NOS-=2;TOS=stack[NOS];NOS--;break;
	case 106://CMOVE> 
		W=stack[NOS-1]+TOS;W1=stack[NOS]+TOS;
		while (TOS--) { W--;W1--;mem.setInt8(W,mem.getInt8(W1)); }
		NOS-=2;TOS=stack[NOS];NOS--;break;
	case 107://CFILL
		W1=stack[NOS-1];W=stack[NOS];
		while (TOS--) { mem.setInt8(W,W1);W++; }
		NOS-=2;TOS=stack[NOS];NOS--;break;
	case 108://QMOVE 
		W=stack[NOS-1];W1=stack[NOS];
		while (TOS--) { mem.setInt64(W,mem.getInt64(W1));W+=8;W1+=8; }
		NOS-=2;TOS=stack[NOS];NOS--;break;
	case 109://MOVE> 
		W=stack[NOS-1]+(TOS<<3);W1=stack[NOS]+(TOS<<3);
		while (TOS--) { W-=8;W1-=8;mem.setInt64(W,mem.getInt64(W1)); }
		NOS-=2;TOS=stack[NOS];NOS--;break;
	case 110://QFILL
		W1=stack[NOS-1];W=stack[NOS];
		while (TOS--) { mem.setInt64(W,W1);W+=8; }
		NOS-=2;TOS=stack[NOS];NOS--;break;
*/
//	case 111:systemcall(TOS,stack[NOS]);TOS=stack[NOS-1];NOS-=2;break; //SYSCALL | nro int -- 
//	case 112:TOS=systemmem(TOS);break;//SYSMEM | nro -- ini
	}
   }
}
	
SDL_Event evt;
	
////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
r3compile("main.r3");

dumpcode();
//------------------------------------------------------------------------	
gr_init("r3tv",800,600);

int salida=0;
while (!salida) {
	
	SDL_WaitEvent(&evt);
	switch (evt.type) {
	    case SDL_KEYDOWN:
	      switch(evt.key.keysym.sym) {
/*	      	
	    	case SDLK_F1: videoclose();videoopen("salud.mp4",500,300);break;
	    	case SDLK_F2: videoclose();videoopen("bigbuckbunny_480x272.h265",500,300);break;
	    	case SDLK_F3: videoclose();videoopen("Titanic.ts",500,300);break;
	    	case SDLK_F4: videoclose();videoopen("coso.mp3",500,300);break;
*/	    	
	    	case SDLK_ESCAPE: salida=1;break;
			default:break;
      		}
      	  break;
		//default: break;
		}
			
	gr_cls(0);
	gr_color1=0xf000f0;
	gr_psegmento(100,100,600,200);
	gr_psegmento(600,200,2500,2400);
	gr_psegmento(2500,2400,50,450);
	gr_psegmento(50,450,100,100);
	gr_drawPoli();
	gr_color1=0xff;	
	gr_psegmento(200,200,200,1800);
	gr_psegmento(200,1800,1200,1800);	
	gr_psegmento(1200,200,1200,1800);		
	gr_psegmento(200,200,1200,200);			
	gr_drawPoli();	
	gr_color1=0xff00;gr_line(10,450,600,40);
	gr_color1=0xff0000;gr_line(600,450,10,40);
	gr_redraw();
	}
	
gr_fin();
return 0;
}
