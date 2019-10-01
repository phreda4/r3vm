#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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

int memcsize;
int memc=0;
int *memcode;

int memdsize=0xfffff;
int memd=0;
int meminidata=0;
char *memdata;

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

const char *r3asm[]={
";","LIT1","ADR","CALL","VAR",
"JMP","JMPR","LIT2","LIT3"			// internal only
};

const char *r3bas[]={
";","(",")","[","]",
"|","|","|","|",			// internal only
"EX",
"0?","1?","+?","-?", 								
"<?",">?","=?",">=?","<=?","<>?","AN?","NA?","BTW?",
"DUP","DROP","OVER","PICK2","PICK3","PICK4","SWAP","NIP",
"ROT","2DUP","2DROP","3DROP","4DROP","2OVER","2SWAP",
">R","R>","R@",
"AND","OR","XOR","NOT","NEG",
"+","-","*","/","*/",
"/MOD","MOD","ABS","SQRT","CLZ",
"<<",">>",">>>","*>>","<</",
"@","C@","Q@","@+","C@+","Q@+",
"!","C!","Q!","!+","C!+","Q!+",
"+!","C+!","Q+!",
">A","A>","A@","A!","A+","A@+","A!+",
">B","B>","B@","B!","B+","B@+","B!+",
"MOVE","MOVE>","FILL",
"CMOVE","CMOVE>","CFILL",
"QMOVE","QMOVE>","QFILL",
"UPDATE","REDRAW",
"MEM","SW","SH","VFRAME",
"XYPEN","BPEN","KEY",
"MSEC","TIME","DATE",
"LOAD","SAVE","APPEND",
"FFIRST","FNEXT",
"INK","OP","LINE","CURVE","CURVE3","PLINE","PCURVE","PCURVE3","POLI",
"SYS",""
};

enum {
FIN,LIT,ADR,CALL,VAR, 
JMP,JMPR,LIT2,LIT3,
EX,
ZIF,UIF,PIF,NIF,
IFL,IFG,IFE,IFGE,IFLE,IFNE,IFAND,IFNAND,IFBT,
DUP,DROP,OVER,PICK2,PICK3,PICK4,SWAP,NIP,
ROT,DUP2,DROP2,DROP3,DROP4,OVER2,SWAP2,
TOR,RFROM,ERRE,
AND,OR,XOR,NOT,NEG,
ADD,SUB,MUL,DIV,MULDIV,
DIVMOD,MOD,ABS,CSQRT,CLZ,
SHL,SHR,SHR0,MULSHR,CDIVSH,
FECH,CFECH,QFECH,FECHPLUS,CFECHPLUS,QFECHPLUS,
STOR,CSTOR,QSTOR,STOREPLUS,CSTOREPLUS,QSTOREPLUS,
INCSTOR,CINCSTOR,QINCSTOR,
TOA,ATO,AF,AS,AA,AFA,ASA,
TOB,BTO,BF,BS,BA,BFA,BSA,
MOVED,MOVEA,FILL,
CMOVED,CMOVEA,CFILL,
QMOVED,QMOVEA,QFILL,
UPDATE,REDRAW,
MEM,SW,SH,FRAMEV,
XYPEN,BPEN,KEY,
MSEC,TIME,IDATE,
LOAD,SAVE,APPEND,
FFIRST,FNEXT,
INK,OP,LINE,CURVE,CURVE3,PLINE,PCURVE,PCURVE3,POLI,
SYS
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
  else if (*p>='a') dig=*p-'a'+10;  
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
while (((unsigned char)*s)<33&&*s!=0) s++;
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
s++;
while (*s!=0)	{
	if (*s==34) { 
		s++;if (*s!=34) { 
			s++;break; } }
	s++;
	}
return s;
}

int isBas(char *p)
{    
nro=0;
char **m=(char**)r3bas;
while (**m!=0) {
  if (strequal(*m,p)) return -1;
  *m++;nro++; }
return 0;  
};

int isWord(char *p) 
{ 
int i=cntdicc;
while (--i>-1) { 
	if (strequal(dicc[i].nombre,p) && ((dicc[i].info&1)==1 || i>=dicclocal)) return i;
	}
return -1;
};

void codetok(int nro) 
{ 
memcode[memc++]=nro; 
}

void closevar() 
{
if (cntdicc==0) { return; }
if ((dicc[cntdicc-1].info&0x10)==0) { return; } // prev is var
if (dicc[cntdicc].mem<memd) { return; } // have val
memdata[memd]=0;memd+=4;
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
str++;
int ini=datasave(str);	
if (modo<2) codetok((ini<<8)+ADR); // lit data
}

void datanro(int n) { 
char *p=&memdata[memd];	
switch(modo){
	case 2:*(int*)p=(int)n;memd+=4;break;
	case 3:for(int i=0;i<n;i++) { *p++=0; };memd+=n;break;
	case 4:*p=(char)n;memd+=1;break;
	case 5:*(uint64_t*)p=(uint64_t)n;memd+=8;break;
	}
}

void compilaADDR(int n) 
{
// si es code, directo, si es data.. puntero	
if (modo>1) { datanro(dicc[n].mem);return; }
codetok((dicc[n].mem<<8)+LIT+((dicc[n].info>>4)&1));  //1 code 2 data
}

void compilaLIT(int n) 
{
if (modo>1) { datanro(n);return; }
codetok((n<<8)+LIT); 
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
	int op=memcode[i]&0xff;
	if (op>=ZIF && op<=IFBT && (memcode[i]>>8)==0) {
		memcode[i]+=(memc-i)<<8;	// patch while
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
	codetok((-(dist+1)<<8)+JMPR); 	// jmpr
} else {
	memcode[from-1]|=(dist<<8);		// patch if
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
codetok(n);	
if (n==0) { if (level==0) {modo=0;} } // ;
}

void compilaWORD(int n) 
{
if (modo>1) { datanro(n);return; }
codetok((dicc[n].mem<<8)+CALL+((dicc[n].info>>4)&1));
}

int r3token(char *str) 
{
level=0;
while(*str!=0) {
	str=trim(str);if (*str==0) return -1;
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
			nro=isWord(str+1);
			if (nro<0) { werror="adr not found";return 0; }
			compilaADDR(nro);str=nextw(str);break;		
		default:
			if (isNro(str)) { compilaLIT(nro);str=nextw(str);break; }
			if (isBas(str)) { compilaMAC(nro);str=nextw(str);break; }
			nro=isWord(str);
			if (nro<0) { werror="word not found";return 0; }
			if (modo==2) { compilaADDR(nro);str=nextw(str);break; }
			compilaWORD(nro);str=nextw(str);break;
		}
	}
return -1;
}

///////////////////////////////////////////////
char *openfile(char *filename)
{
//printf("loading %s\n",filename);
long len;
char *buff;
FILE *f=fopen(filename,"rb");if (!f) return 0;
fseek(f,0,SEEK_END);len=ftell(f);fseek(f,0,SEEK_SET);
buff=(char*)malloc(len+1);if (!buff) return 0;
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
	if (strequal(includes[i].nombre,ns)) return -1;
	}
includes[cntincludes].nombre=ns; // ./coso y coso son distintos !!
includes[cntincludes].str=openfile(filename); 
cntincludes++;	
return cntincludes-1;
}

void freeinc()
{
for (int i=0;i<cntincludes;i++){
	free(includes[cntincludes].str);
	}
}

void r3includes(char *str) 
{
int ninc;	
while(*str!=0) {
	str=trim(str);
	switch (*str) {
		case '^':	// include
			ninc=isinclude(str+1);
			if (ninc>=0) {
				r3includes(includes[ninc].str);
				stacki[cntstacki++]=ninc;
				}
			str=nextcr(str);
			break;
		case '|':	// comments	
			str=nextcr(str);break; 
		case ':':	// code
			modo=1;str=nextw(str);break;
		case '#':	// data	
			modo=0;str=nextw(str);break;			
		case '"':	// strings		
			memcsize+=modo;str=nextstr(str);break;
		default:	// resto
			memcsize+=modo;str=nextw(str);break;
		}
	}
return;
}

///////////////////////////////////////////////
void printcode(int n)
{
if ((n&0xff)<8&&n!=0) {
	printf(r3asm[n&0xff]);printf(" %d",n>>8);
} else
	printf(r3bas[n&0xff]);
printf("\n");
}

void dumpcode()
{
printf("code\n");
printf("boot:%d\n",boot);
for(int i=1;i<memc;i++) {
	printf("%d.",i);
	printcode(memcode[i]); //printf("%d ",memcode[i]);
	}
printf("\n");
}

void dumpinc()
{
printf("includes\n");
for(int i=0;i<cntincludes;i++) {
	printf("%d. ",i);
	printword(includes[i].nombre);
	printf("\n");
	}
for(int i=0;i<cntstacki;i++) {
	printf("%d. %d\n",i,stacki[i]);
	}
}

void dumpdicc()
{
printf("diccionario\n");
for(int i=0;i<cntdicc;i++) {
	printf("%d. ",i);
	printword(dicc[i].nombre);
	printf("%d \n",dicc[i].info);	
	}
}

///////////////////////////////////////////////
int r3compile(char *name) 
{
char filename[1024];
char *str;

strcpy(path,"r3/");
sprintf(filename,"%s%s",path,name);
str=openfile(filename);
//printf(str);

memcsize=0;
meminidata=0;
cntincludes=0;
cntstacki=0;
r3includes(str); // load includes

//dumpinc();

cntdicc=0;
dicclocal=0;
boot=-1;
memc=1; // direccion 0 para null
memd=0;

memcode=(int*)malloc(sizeof(int)*memcsize);
memdata=(char*)malloc(memdsize);
// tokenize
for (int i=0;i<cntstacki;i++) {
//	printword(includes[stacki[i]].nombre);printf("\n");
	if (!r3token(includes[stacki[i]].str)) return 0;
//	r3token(includes[stacki[i]].str);
	dicclocal=cntdicc;
	}
	
// last tokenizer		
if (!r3token(str)) return 0;
//memcode[memc++]=0;

//printf("estimate tokens:%d\ntokens:%d\n",memcsize,memc);
//dumpdicc();

dumpcode();

freeinc();
free(str);

return -1;
}

	
//----------------------
/*--------RUNER--------*/
//----------------------
#ifdef __GNUC__
#define iclz(x) __builtin_clz(x)
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
static inline int iclz(int x)
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
static inline int isqrt(int value)
{
if (value==0) return 0;
int g = 0;
int bshft = (31-iclz(value))>>1;  // spot the difference!
int b = 1<<bshft;
do {
	int temp = (g+g+b)<<bshft;
	if (value >= temp) { g += b;value -= temp;	}
	b>>=1;
} while (bshft--);
return g;
}

//---------------------------//
SDL_Event evt;
WIN32_FIND_DATA ffd;
HANDLE hFind=NULL;

time_t sit;
tm *sitime;

int sw,sh;
int xm=0;
int ym=0;
int bm=0;
int gx1=0;
int gy1=0;

int ke=0;
int kc=0;

void r3update()
{
//SDL_WaitEvent(&evt);
SDL_PollEvent(&evt);
switch (evt.type) {
	case SDL_KEYDOWN:
		ke=evt.key.keysym.sym;
      	break;
	case SDL_KEYUP:
		ke=evt.key.keysym.sym;
      	break;
	case SDL_MOUSEBUTTONDOWN:	      	
		bm|=evt.button.button;
		break;
	case SDL_MOUSEBUTTONUP:	      	
		bm&=~evt.button.button;
		break;
	case SDL_MOUSEMOTION:
	    xm=evt.motion.x;
	    ym=evt.motion.y;		
	    break;
	}

}
         
//---------------------------//
// TOS..DSTACK--> <--RSTACK  //
//---------------------------//
int64_t stack[256];
FILE *file;

void runr3(int boot) 
{
stack[255]=0;	
register int ip=boot;
register int64_t TOS=0;
register int64_t *NOS=&stack[0];
register int64_t *RTOS=&stack[255];
register int64_t REGA=0;
register int64_t REGB=0;
register int op=0;
register int64_t W=0;
register int64_t W1=0; 
while(ip!=0) { 
	op=memcode[ip++]; 
	
//	printcode(op);
	
	switch(op&0xff){
	case FIN:ip=*RTOS;RTOS++;continue; 							// ;
	case LIT:NOS++;*NOS=TOS;TOS=op>>8;continue;					// LIT1
	case ADR:NOS++;*NOS=TOS;TOS=(int64_t)&memdata[op>>8];continue;		// LIT adr
	case CALL:RTOS--;*RTOS=ip;ip=(unsigned int)op>>8;continue;	// CALL
	case VAR:NOS++;*NOS=TOS;TOS=memdata[op>>8];continue;		// VAR
	case JMP:ip=(op>>8);continue;//JMP							// JMP
	case JMPR:ip+=(op>>8);continue;//JMP						// JMPR	
	case LIT2:TOS^=(op&0xffffff00)<<24;continue;				// LIT xor xxxxxx....aaaaaa
	case LIT3:TOS^=(op&0xffffff00)<<16;continue;				// LIT xor ....xxxxxxaaaaaa	
	case EX:W=TOS;TOS=*NOS;NOS--;RTOS--;*RTOS=ip;ip=W;continue;//EX
	case ZIF:if (TOS!=0) {ip+=(op>>8);}; continue;//ZIF
	case UIF:if (TOS==0) {ip+=(op>>8);}; continue;//UIF
	case PIF:if (TOS<0) {ip+=(op>>8);}; continue;//PIF
	case NIF:if (TOS>=0) {ip+=(op>>8);}; continue;//NIF
	case IFL:if (TOS<=*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFL
	case IFG:if (TOS>=*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFG
	case IFE:if (TOS!=*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFN
	case IFGE:if (TOS<*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFGE
	case IFLE:if (TOS>*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFLE
	case IFNE:if (TOS==*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFNO
	case IFAND:if (!(TOS&*NOS)) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFNA
	case IFNAND:if (TOS&*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFAN
	case IFBT:if (TOS<=*NOS&&*NOS<=*(NOS+1)) {ip+=(op>>8);} 
		TOS=*(NOS-1);NOS-=2;continue;//BTW (need bit trick)
	case DUP:NOS++;*NOS=TOS;continue;				//DUP
	case DROP:TOS=*NOS;NOS--;continue;				//DROP
	case OVER:NOS++;*NOS=TOS;TOS=*(NOS-1);continue;	//OVER
	case PICK2:NOS++;*NOS=TOS;TOS=*(NOS-2);continue;//PICK2
	case PICK3:NOS++;*NOS=TOS;TOS=*(NOS-3);continue;//PICK3
	case PICK4:NOS++;*NOS=TOS;TOS=*(NOS-4);continue;//PICK4
	case SWAP:W=*NOS;*NOS=TOS;TOS=W;continue;		//SWAP
	case NIP:NOS--;continue; 						//NIP
	case ROT:W=TOS;TOS=*(NOS-1);*(NOS-1)=*NOS;*NOS=W;continue;	//ROT
	case DUP2:W=*NOS;NOS++;*NOS=TOS;NOS++;*NOS=W;continue;//DUP2
	case DROP2:NOS--;TOS=*NOS;NOS--;continue;				//DROP2
	case DROP3:NOS-=2;TOS=*NOS;NOS--;continue;				//DROP3
	case DROP4:NOS-=3;TOS=*NOS;NOS--;continue;				//DROP4
	case OVER2:NOS++;*NOS=TOS;TOS=*(NOS-3);
		NOS++;*NOS=TOS;TOS=*(NOS-3);continue;	//OVER2
	case SWAP2:W=*NOS;*NOS=*(NOS-2);*(NOS-2)=W;
		W=TOS;TOS=*(NOS-1);*(NOS-1)=W;continue;	//SWAP2
	case TOR:RTOS--;*RTOS=TOS;TOS=*NOS;NOS--;continue;	//>r
	case RFROM:NOS++;*NOS=TOS;TOS=*RTOS;RTOS++;continue;	//r>
	case ERRE:NOS++;*NOS=TOS;TOS=*RTOS;continue;			//r@
	case AND:TOS&=*NOS;NOS--;continue;					//AND
	case OR:TOS|=*NOS;NOS--;continue;					//OR
	case XOR:TOS^=*NOS;NOS--;continue;					//XOR
	case NOT:TOS=~TOS;continue;									//NOT
	case NEG:TOS=-TOS;continue;									//NEG
	case ADD:TOS=*NOS+TOS;NOS--;continue;					//SUMA
	case SUB:TOS=*NOS-TOS;NOS--;continue;					//RESTA
	case MUL:TOS=*NOS*TOS;NOS--;continue;					//MUL
	case DIV:TOS=(*NOS/TOS);NOS--;continue;					//DIV
	case MULDIV:TOS=(*(NOS-1)*(*NOS)/TOS);NOS-=2;continue;	//MULDIV
	case DIVMOD:W=*NOS%TOS;*NOS=(*NOS/TOS);TOS=W;continue;	//DIVMOD
	case MOD:TOS=*NOS%TOS;NOS--;continue;					//MOD
	//case ABS:W=(TOS>>31);TOS=(TOS+W)^W;continue;			//ABS
	case ABS:W=(TOS>>63);TOS=(TOS+W)^W;continue;			//ABS
	case CSQRT:TOS=isqrt(TOS);continue;					//CSQRT
	case CLZ:TOS=iclz(TOS);continue;					//CLZ
	case SHL:TOS=*NOS<<TOS;NOS--;continue;				//SAl
	case SHR:TOS=*NOS>>TOS;NOS--;continue;				//SAR
	case SHR0:TOS=((unsigned)*NOS)>>TOS;NOS--;continue;	//SHR
	case MULSHR:TOS=(*(NOS-1)*(*NOS))>>TOS;NOS-=2;continue;	//MULSHR
	case CDIVSH:TOS=((*(NOS-1)<<TOS)/(*NOS));NOS-=2;continue;//CDIVSH
	case FECH:TOS=*(int*)TOS;continue;//@
	case CFECH:TOS=*(char*)TOS;continue;//C@
	case QFECH:TOS=*(int64_t*)TOS;continue;//Q@	
	case FECHPLUS:NOS++;*NOS=TOS+4;TOS=*(int*)TOS;continue;//@+
	case CFECHPLUS:NOS++;*NOS=TOS+1;TOS=*(char*)TOS;continue;// C@+
	case QFECHPLUS:NOS++;*NOS=TOS+8;TOS=*(int64_t*)TOS;continue;//Q@+		
	case STOR:*(int*)TOS=(int)*NOS;NOS--;TOS=*NOS;NOS--;continue;// !
	case CSTOR:*(char*)TOS=(char)*NOS;NOS--;TOS=*NOS;NOS--;continue;//C!
	case QSTOR:*(int64_t*)TOS=*NOS;NOS--;TOS=*NOS;NOS--;continue;//Q!
	case STOREPLUS:*(int*)TOS=*NOS;NOS--;TOS+=4;continue;// !+
	case CSTOREPLUS:*(char*)TOS=*NOS;NOS--;TOS++;continue;//C!+
	case QSTOREPLUS:*(int64_t*)TOS=*NOS;NOS--;TOS+=8;continue;//Q!+
	
	case INCSTOR:*(int*)TOS+=*NOS;NOS--;TOS=*NOS;NOS--;continue;//+!
	case CINCSTOR:*(char*)TOS+=*NOS;NOS--;TOS=*NOS;NOS--;continue;//C+!
	case QINCSTOR:*(int64_t*)TOS+=*NOS;NOS--;TOS=*NOS;NOS--;continue;//Q+!
	case TOA:REGA=TOS;TOS=*NOS;NOS--;continue; //>A
	case ATO:NOS++;*NOS=TOS;TOS=REGA;continue; //A> 
	case AF:NOS++;*NOS=TOS;TOS=*(int*)REGA;continue;//A@
	case AS:*(int*)REGA=TOS;TOS=*NOS;NOS--;continue;//A! 
	case AA:REGA+=TOS;TOS=*NOS;NOS--;continue;//A+ 
	case AFA:NOS++;*NOS=TOS;TOS=*(int*)REGA;REGA+=4;continue;//A@+ 
	case ASA:*(int*)REGA=TOS;TOS=*NOS;NOS--;REGA+=4;continue;//A!+
	case TOB:REGB=TOS;TOS=*NOS;NOS--;continue; //>B
	case BTO:NOS++;*NOS=TOS;TOS=REGB;continue; //B> 
	case BF:NOS++;*NOS=TOS;TOS=*(int*)REGB;continue;//B@
	case BS:*(int*)REGB=TOS;TOS=*NOS;NOS--;continue;//B! 
	case BA:REGB+=TOS;TOS=*NOS;NOS--;continue;//B+ 
	case BFA:NOS++;*NOS=TOS;TOS=*(int*)REGB;REGB+=4;continue;//B@+ 
	case BSA:*(int*)REGB=TOS;TOS=*NOS;NOS--;REGB+=4;continue;//B!+
	case MOVED://MOVE 
		W=(int64_t)&*(NOS-1);W1=(int64_t)&(*NOS);
		while (TOS--) { *(int*)W=*(int*)W1;(int)W++;W1++; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case MOVEA://MOVE> 
		W=(int64_t)&*(NOS-1)+(TOS<<2);W1=(int64_t)&(*NOS)+(TOS<<2);
		while (TOS--) { W-=4;W1-=4;*(int*)W=*(int*)W1; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case FILL://FILL
		W1=(int64_t)&*(NOS-1);op=*NOS;
		while (TOS--) { *(int*)W=op;W+=4; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case CMOVED://CMOVE 
		W=(int64_t)&*(NOS-1);W1=(int64_t)&*NOS;
		while (TOS--) { *(char*)W=*(char*)W1;W++;W1++; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case CMOVEA://CMOVE> 
		W=(int64_t)&*(NOS-1)+TOS;W1=(int64_t)&*NOS+TOS;
		while (TOS--) { W--;W1--;*(char*)W=*(char*)W1; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case CFILL://CFILL
		W1=(int64_t)&*(NOS-1);op=*NOS;
		while (TOS--) { *(char*)W=op;W++; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case QMOVED://QMOVE 
		W=(int64_t)&*(NOS-1);W1=(int64_t)&*NOS;
		while (TOS--) { *(int64_t*)W=*(int64_t*)W1;W++;W1++; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case QMOVEA://MOVE> 
		W=(int64_t)&*(NOS-1)+(TOS<<3);W1=(int64_t)&*NOS+(TOS<<3);
		while (TOS--) { W--;W1--;*(int64_t*)W=*(int64_t*)W1; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case QFILL://QFILL
		W1=(int64_t)&*(NOS-1);op=*NOS;
		while (TOS--) { *(int64_t*)W=op;W++; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case UPDATE://"UPDATE"
		r3update();continue;
	case REDRAW://"REDRAW"
		gr_redraw();continue;
	case MEM://"MEM"
		NOS++;*NOS=TOS;TOS=(int64_t)&memdata[meminidata];continue;
	case SW://"SW"
		NOS++;*NOS=TOS;TOS=gr_ancho;continue;
	case SH://"SH"
		NOS++;*NOS=TOS;TOS=gr_alto;continue;
	case FRAMEV://"VFRAME"
		NOS++;*NOS=TOS;TOS=(int64_t)gr_buffer;continue;
	case XYPEN://"XYPEN"
		NOS++;*NOS=TOS;NOS++;*NOS=xm;TOS=ym;continue;
	case BPEN://"BPEN"
		NOS++;*NOS=TOS;TOS=bm;continue;
	case KEY://"KEY"
		NOS++;*NOS=TOS;TOS=ke;continue;	
	case MSEC://"MSEC"
		NOS++;*NOS=TOS;TOS=SDL_GetTicks();continue;		
	case TIME://"TIME"
		time(&sit);sitime=localtime(&sit);
		NOS++;*NOS=TOS;TOS=(sitime->tm_hour<<16)|(sitime->tm_min<<8)|sitime->tm_sec;continue;	
	case IDATE://"DATE"
		time(&sit);sitime=localtime(&sit);
		NOS++;*NOS=TOS;TOS=(sitime->tm_year+1900)<<16|(sitime->tm_mon+1)<<8|sitime->tm_mday;continue;
	case LOAD: //LOAD: // 'from "filename" -- 'to
        if (TOS==0||*NOS==0) { TOS=*NOS;NOS--;continue; }
        file=fopen((char*)TOS,"rb");
        TOS=*NOS;NOS--;
        if (file==NULL) continue;
        do { W=fread((void*)TOS,sizeof(char),1024,file); TOS+=W; } while (W==1024);
        fclose(file);continue;
    case SAVE: //SAVE: // 'from cnt "filename" --
        if (TOS==0||*NOS==0) 
			{ DeleteFile((char*)TOS);NOS-=2;TOS=*NOS;NOS--;continue; }
        file=fopen((char*)TOS,"wb");
        TOS=*NOS;NOS--;
        if (file==NULL) { NOS--;TOS=*NOS;NOS--;continue; }
        fwrite((void*)*NOS,sizeof(char),TOS,file);
        fclose(file);
        NOS--;TOS=*NOS;NOS--;continue;
    case APPEND: //APPEND: // 'from cnt "filename" --
        if (TOS==0||*NOS==0) { NOS-=2;TOS=*NOS;NOS--;continue; }
        file=fopen((char*)TOS,"ab");
        TOS=*NOS;NOS--;
        if (file==NULL) { NOS--;TOS=*NOS;NOS--;continue; }
        fwrite((void*)*NOS,sizeof(char),TOS,file);
        fclose(file);
        NOS--;TOS=*NOS;NOS--;continue;
	case FFIRST://"FFIRST"
        if (hFind!=NULL) FindClose(hFind);
        strcpy(path,(char*)TOS);strcat(path,"\\*");
        hFind=FindFirstFile(path, &ffd);
        if (hFind == INVALID_HANDLE_VALUE) TOS=0; else TOS=(int64_t)&ffd;
        continue;
	case FNEXT://"FNEXT"
        NOS++;*NOS=TOS;
        if (FindNextFile(hFind, &ffd)==0) TOS=0; else TOS=(int64_t)&ffd;
        continue ;
	case INK://"INK",
		gr_alpha((TOS>>24)^0xff);gr_color1=TOS;
		TOS=*NOS;NOS--;continue;
	case OP://"OP",
		gy1=TOS;gx1=*NOS;NOS--;TOS=*NOS;NOS--;continue;
	case LINE:
		gr_line(gx1,gy1,*NOS,TOS);gx1=*NOS;gy1=TOS;
        NOS--;TOS=*NOS;NOS--;continue;
    case CURVE: 
		gr_spline(gx1,gy1,*NOS,TOS,*(NOS-2),*(NOS-1));gx1=*(NOS-2);gy1=*(NOS-1);
		NOS-=3;TOS=*NOS;NOS--;continue;
    case CURVE3: 
		gr_spline3(gx1,gy1,*NOS,TOS,*(NOS-2),*(NOS-1),*(NOS-4),*(NOS-3));gx1=*(NOS-4);gy1=*(NOS-3);
		NOS-=5;TOS=*NOS;NOS--;continue;
	case PLINE:
		gr_pline(gx1,gy1,*NOS,TOS);gx1=*NOS;gy1=TOS;
        NOS--;TOS=*NOS;NOS--;continue;
    case PCURVE:
		gr_pcurve(gx1,gy1,*NOS,TOS,*(NOS-2),*(NOS-1));gx1=*(NOS-2);gy1=*(NOS-1);
		NOS-=3;TOS=*NOS;NOS--;continue;
    case PCURVE3:
		gr_pcurve3(gx1,gy1,*NOS,TOS,*(NOS-2),*(NOS-1),*(NOS-4),*(NOS-3));gx1=*(NOS-4);gy1=*(NOS-3);
		NOS-=5;TOS=*NOS;NOS--;continue;
	case POLI: 
		gr_drawPoli();continue;
//	case 111:systemcall(TOS,stack[NOS]);TOS=stack[NOS-1];NOS-=2;continue; //SYSCALL | nro int -- 
//	case 112:TOS=systemmem(TOS);continue;//SYSMEM | nro -- ini
//case SYS:
	}
   }
}
	
	
////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
gr_init("r3",800,600);

r3compile("main.r3");
runr3(boot);

gr_fin();
return 0;
}
