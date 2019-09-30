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

"INK","OP","LINE","CURVE","CURVE3","PLINE","PCURVE","PCURVE3","POLI",

"SYSCALL","SYSMEM",
"SYS",""
};

enum {
FIN,LIT,ADR,CALL,JMP,//JMPR, EXEC,//hasta JMPR no es visible
IF,PIF,NIF,UIF,IFN,IFL,IFG,IFLE,IFGE,IFNO,IFAND,IFNAND,// condicionales 0 - y +  y no 0
DUP,DROP,OVER,PICK2,PICK3,PICK4,SWAP,NIP,ROT,
DUP2,DROP2,DROP3,DROP4,OVER2,SWAP2,//--- pila
TOR,RFROM,ERRE,//ERREM,ERRFM,ERRSM,ERRDR,//--- pila direcciones
AND,OR,XOR,NOT,//--- logica
SUMA,RESTA,MUL,DIV,MULDIV,MULSHR,DIVMOD,MOD,ABS,
CSQRT,CLZ,CDIVSH,
NEG,INC,INC4,DEC,DIV2,MUL2,SHL,SHR,SHR0,//--- aritmetica
FECH,CFECH,WFECH,STOR,CSTOR,WSTOR,INCSTOR,CINCSTOR,WINCSTOR,//--- memoria
FECHPLUS,STOREPLUS,CFECHPLUS,CSTOREPLUS,WFECHPLUS,WSTOREPLUS,
TOA,ATO,AF,AS,AA,AFA,ASA,
TOB,BTO,BF,BS,BA,BFA,BSA,
MOVED,MOVEA,FILL,CMOVED,CMOVEA,CFILL,
MEM,
FFIRST,FNEXT,
LOAD,SAVE,APPEND,//--- bloques de memoria, bloques
UPDATE,
XYMOUSE,BMOUSE, //MOUSE
SKEY, KEY,
CNTJOY,GETJOY,
MSEC,TIME,IDATE,SISEND,SISRUN,//--- sistema
WIDTH,HEIGHT,
REDRAW,FRAMEV,//--- pantalla
COLOR,COLORA,ALPHA,//--- color
OP,LINE,CURVE,CURVE3,PLINE,PCURVE,PCURVE3,POLI,//--- dibujo
FCOL,FCEN,FMAT,SFILL,LFILL,RFILL,TFILL, //--- pintado

SLOAD,SPLAY,SINFO,SSET,

#ifdef NET
OPENURL,
#endif

SYSTEM,
ULTIMAPRIMITIVA// de aqui en mas.. apila los numeros 0..255-ULTIMAPRIMITIVA
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
if (modo<2) codetok((ini<<7)+2); // lit data
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
codetok((dicc[n].mem<<7)+1+((dicc[n].info>>4)&1));  //1 code 2 data
}

void compilaLIT(int n) 
{
if (modo>1) { datanro(n);return; }
codetok((n<<7)+1); 
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
//printf("token...\n")	;
level=0;
while(*str!=0) {
	str=trim(str);if (*str==0) return -1;
//	printf(">");printword(str);printf("<");
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
//			printword(str+1);printf("<dir>%d",nro);printf("\n");			
			if (nro<0) return 0;
			compilaADDR(nro);str=nextw(str);break;		
		default:
//	printword(str);printf(" ");			
			if (isNro(str)) { compilaLIT(nro);str=nextw(str);break; }
			if (isBas(str)) { compilaMAC(nro);str=nextw(str);break; }
			nro=isWord(str);
//			printf("<wrd>");printword(str);printf("<wrd>%d",nro);printf("\n");
			if (nro<0) { printf("palabra no encontrada\n");return 0; }
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
void dumpcode()
{
printf("code\n");
printf("boot:%d\n",boot);
for(int i=0;i<memc;i++)
	printf("%d ",memcode[i]);
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
//	printf("%d %d!",i,cntstacki);
	printword(includes[stacki[i]].nombre);printf("\n");
	
	if (!r3token(includes[stacki[i]].str)) return 0;
//	r3token(includes[stacki[i]].str);
	dicclocal=cntdicc;
	}
	
// last tokenizer		
if (!r3token(str)) return 0;
memcode[memc++]=0;

//printf("estimate tokens:%d\ntokens:%d\n",memcsize,memc);

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
int sw,sh;
int xm=0;
int ym=0;
int bm=0;
int gx1=0;
int gy1=0;

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

register int op=0;
register int64_t W=0;
register int64_t W1=0; 

register unsigned char NOS=0;
register unsigned char RTOS=255;
stack[255]=0;
while(ip!=0) { 
	op=memcode[ip++]; 
	switch(op&0xff){
	case 0:ip=stack[RTOS];RTOS++;continue; 						// ;
	case 1:NOS++;stack[NOS]=TOS;TOS=op>>8;continue;				// LIT1
	case 2:NOS++;stack[NOS]=TOS;TOS=(int64_t)&memdata[(unsigned int)op>>8];continue;	// LIT data

	case 3:RTOS--;stack[RTOS]=ip;ip=(unsigned int)op>>8;continue;			// CALL
	case 4:ip=(op>>7);continue;//JMP	
	
	case 5:
	case 6:				
	case 7: NOS++;stack[NOS]=TOS;TOS=(op<<16)>>23;continue;		// LIT9
	case 8: NOS++;stack[NOS]=TOS;TOS=op>>7;continue;				// LITres
	case 9: NOS++;stack[NOS]=TOS;TOS=(op&0xffffff80);continue;		// LIT1
	case 10:TOS|=(op>>7)&0x7f;continue;		// LIT2
	case 11:NOS++;stack[NOS]=TOS;TOS=op>>7;continue;				// STR
	case 12:RTOS--;stack[RTOS]=ip;ip=(unsigned int)op>>7;continue;				// CALL
	case 13:NOS++;stack[NOS]=TOS;TOS=memdata[(unsigned int)op>>7];continue;	// VAR
	case 14:NOS++;stack[NOS]=TOS;TOS=op>>7;continue;				// DWORD
	case 15:NOS++;stack[NOS]=TOS;TOS=op>>7;continue;				// DVAR
	case 16:ip=stack[RTOS];RTOS++;continue; 						// ;
	case 17:ip=(op>>7);continue;//JMP
	case 18:ip+=(op>>7);continue;//JMPR
	case 19:continue;
	case 20:continue;
	
	
	case 21:W=TOS;TOS=stack[NOS];NOS--;RTOS--;stack[RTOS]=ip;ip=W;continue;//EX

	case 22:if (TOS!=0) {ip+=(op>>7);}; continue;//ZIF
	case 23:if (TOS==0) {ip+=(op>>7);}; continue;//UIF
	case 24:if (TOS<0) {ip+=(op>>7);}; continue;//PIF
	case 25:if (TOS>=0) {ip+=(op>>7);}; continue;//NIF
	
	case 26:if (TOS<stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;continue;//IFGE
	case 27:if (TOS>stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;continue;//IFLE
	case 28:if (TOS!=stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;continue;//IFN
	case 29:if (TOS>=stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;continue;//IFG
	case 30:if (TOS<=stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;continue;//IFL
	case 31:if (TOS==stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;continue;//IFNO
	case 32:if (!(TOS&stack[NOS])) {ip+=(op>>7);} TOS=stack[NOS];NOS--;continue;//IFNA
	case 33:if (TOS&stack[NOS]) {ip+=(op>>7);} TOS=stack[NOS];NOS--;continue;//IFAN
	case 34:if (TOS<=stack[NOS]&&stack[NOS]<=stack[NOS]) {ip+=(op>>7);} 
				TOS=stack[NOS-1];NOS-=2;continue;//BTW (need bit trick)

	case 35:NOS++;stack[NOS]=TOS;continue;						//DUP
	case 36:TOS=stack[NOS];NOS--;continue;						//DROP
	case 37:NOS++;stack[NOS]=TOS;TOS=stack[NOS-1];continue;	//OVER
	case 38:NOS++;stack[NOS]=TOS;TOS=stack[NOS-2];continue;	//PICK2
	case 39:NOS++;stack[NOS]=TOS;TOS=stack[NOS-3];continue;	//PICK3
	case 40:NOS++;stack[NOS]=TOS;TOS=stack[NOS-4];continue;	//PICK4
	case 41:W=stack[NOS];stack[NOS]=TOS;TOS=W;continue;		//SWAP
	case 42:NOS--;continue; 									//NIP
	case 43:W=TOS;TOS=stack[NOS-1];stack[NOS-1]=stack[NOS];stack[NOS]=W;continue;//ROT
	case 44:W=stack[NOS];NOS++;stack[NOS]=TOS;NOS++;stack[NOS]=W;continue;//DUP2
	case 45:NOS--;TOS=stack[NOS];NOS--;continue;				//DROP2
	case 46:NOS-=2;TOS=stack[NOS];NOS--;continue;				//DROP3
	case 47:NOS-=3;TOS=stack[NOS];NOS--;continue;				//DROP4
	case 48:NOS++;stack[NOS]=TOS;TOS=stack[NOS-3];
			NOS++;stack[NOS]=TOS;TOS=stack[NOS-3];continue;	//OVER2
	case 49:W=stack[NOS];stack[NOS]=stack[NOS-2];stack[NOS-2]=W;
			W=TOS;TOS=stack[NOS-1];stack[NOS-1]=W;continue;	//SWAP2
	
	case 50:RTOS--;stack[RTOS]=TOS;TOS=stack[NOS];NOS--;continue;	//>r
	case 51:NOS++;stack[NOS]=TOS;TOS=stack[RTOS];RTOS++;continue;	//r>
	case 52:NOS++;stack[NOS]=TOS;TOS=stack[RTOS];continue;			//r@
	
	case 53:TOS&=stack[NOS];NOS--;continue;					//AND
	case 54:TOS|=stack[NOS];NOS--;continue;					//OR
	case 55:TOS^=stack[NOS];NOS--;continue;					//XOR
	case 56:TOS=~TOS;continue;									//NOT
	case 57:TOS=-TOS;continue;									//NEG
	case 58:TOS=stack[NOS]+TOS;NOS--;continue;					//SUMA
	case 59:TOS=stack[NOS]-TOS;NOS--;continue;					//RESTA
	case 60:TOS=stack[NOS]*TOS;NOS--;continue;					//MUL
	case 61:TOS=(stack[NOS]/TOS);NOS--;continue;					//DIV
	case 62:TOS=((stack[NOS-1]*stack[NOS])/TOS);NOS-=2;continue;	//MULDIV
	case 63:W=stack[NOS]%TOS;stack[NOS]=(stack[NOS]/TOS);TOS=W;continue;//DIVMOD
	case 64:TOS=stack[NOS]%TOS;NOS--;continue;					//MOD
	
	//case 65:W=(TOS>>31);TOS=(TOS+W)^W;continue;				//ABS
	case 65:W=(TOS>>63);TOS=(TOS+W)^W;continue;				//ABS
	case 66:TOS=isqrt(TOS);continue;						//CSQRT
	case 67:TOS=iclz(TOS);continue;						//CLZ
	case 68:TOS=stack[NOS]<<TOS;NOS--;continue;				//SAR
	case 69:TOS=stack[NOS]>>TOS;NOS--;continue;				//SAL
	case 70:TOS=((unsigned)stack[NOS])>>TOS;NOS--;continue;				//SHL
	case 71:TOS=(stack[NOS-1]*stack[NOS])>>TOS;NOS-=2;continue;//MULSHR
	case 72:TOS=((stack[NOS-1]<<TOS)/stack[NOS]);NOS-=2;continue;//CDIVSH
	
	case 73:TOS=*(int*)memdata[TOS];continue;//@
	case 74:TOS=*(char*)memdata[TOS];continue;//C@
	case 75:TOS=*(int64_t*)memdata[TOS];continue;//Q@	

	case 76:NOS++;stack[NOS]=TOS+4;TOS=*(int*)memdata[TOS];continue;//@+
	case 77:NOS++;stack[NOS]=TOS+1;TOS=*(char*)memdata[TOS];continue;// C@+
	case 78:NOS++;stack[NOS]=TOS+8;TOS=*(int64_t*)memdata[TOS];continue;//Q@+		
			
	case 79:*(int*)memdata[TOS]=stack[NOS];NOS--;TOS=stack[NOS];NOS--;continue;// !
	case 80:*(char*)memdata[TOS]=stack[NOS];NOS--;TOS=stack[NOS];NOS--;continue;//C!
	case 81:*(int64_t*)memdata[TOS]=stack[NOS];NOS--;TOS=stack[NOS];NOS--;continue;//Q!
	
	case 82:*(int*)memdata[TOS]=stack[NOS];NOS--;TOS+=4;continue;// !+
	case 83:*(char*)memdata[TOS]=stack[NOS];NOS--;TOS++;continue;//C!+
	case 84:*(int64_t*)memdata[TOS]=stack[NOS];NOS--;TOS+=8;continue;//Q!+
	
	case 85:*(int*)memdata[TOS]+=stack[NOS];NOS--;TOS=stack[NOS];NOS--;continue;//+!
	case 86:*(char*)memdata[TOS]+=stack[NOS];NOS--;TOS=stack[NOS];NOS--;continue;//C+!
	case 87:*(int64_t*)memdata[TOS]+=stack[NOS];NOS--;TOS=stack[NOS];NOS--;continue;//Q+!
	
	case 88:REGA=TOS;TOS=stack[NOS];NOS--;continue; //>A
	case 89:NOS++;stack[NOS]=TOS;TOS=REGA;continue; //A> 
	case 90:NOS++;stack[NOS]=TOS;TOS=*(int*)memdata[REGA];continue;//A@
	case 91:*(int*)memdata[REGA]=TOS;TOS=stack[NOS];NOS--;continue;//A! 
	case 92:REGA+=TOS;TOS=stack[NOS];NOS--;continue;//A+ 
	case 93:NOS++;stack[NOS]=TOS;TOS=*(int*)memdata[REGA];REGA+=4;continue;//A@+ 
	case 94:*(int*)memdata[REGA]=TOS;TOS=stack[NOS];NOS--;REGA+=4;continue;//A!+

	case 95:REGB=TOS;TOS=stack[NOS];NOS--;continue; //>B
	case 96:NOS++;stack[NOS]=TOS;TOS=REGB;continue; //B> 
	case 97:NOS++;stack[NOS]=TOS;TOS=*(int*)memdata[REGB];continue;//B@
	case 98:*(int*)memdata[REGB]=TOS;TOS=stack[NOS];NOS--;continue;//B! 
	case 99:REGB+=TOS;TOS=stack[NOS];NOS--;continue;//B+ 
	case 100:NOS++;stack[NOS]=TOS;TOS=*(int*)memdata[REGB];REGB+=4;continue;//B@+ 
	case 101:*(int*)memdata[REGB]=TOS;TOS=stack[NOS];NOS--;REGB+=4;continue;//B!+
	case 102://MOVE 
		W=(int64_t)&stack[NOS-1];W1=(int64_t)&stack[NOS];
		while (TOS--) { *(int*)W=*(int*)W1;W+=4;W1+=4; }
		NOS-=2;TOS=stack[NOS];NOS--;continue;
	case 103://MOVE> 
		W=(int64_t)&stack[NOS-1]+(TOS<<2);W1=(int64_t)&stack[NOS]+(TOS<<2);
		while (TOS--) { W-=4;W1-=4;*(int*)W=*(int*)W1; }
		NOS-=2;TOS=stack[NOS];NOS--;continue;
	case 104://FILL
		W1=(int64_t)&stack[NOS-1];op=stack[NOS];
		while (TOS--) { *(int*)W=op;W+=4; }
		NOS-=2;TOS=stack[NOS];NOS--;continue;
	case 105://CMOVE 
		W=(int64_t)&stack[NOS-1];W1=(int64_t)&stack[NOS];
		while (TOS--) { *(char*)W=*(char*)W1;W++;W1++; }
		NOS-=2;TOS=stack[NOS];NOS--;continue;
	case 106://CMOVE> 
		W=(int64_t)&stack[NOS-1]+TOS;W1=(int64_t)&stack[NOS]+TOS;
		while (TOS--) { W--;W1--;*(char*)W=*(char*)W1; }
		NOS-=2;TOS=stack[NOS];NOS--;continue;
	case 107://CFILL
		W1=(int64_t)&stack[NOS-1];op=stack[NOS];
		while (TOS--) { *(char*)W=op;W++; }
		NOS-=2;TOS=stack[NOS];NOS--;continue;
	case 108://QMOVE 
		W=(int64_t)&stack[NOS-1];W1=(int64_t)&stack[NOS];
		while (TOS--) { *(int64_t*)W=*(int64_t*)W1;W++;W1++; }
		NOS-=2;TOS=stack[NOS];NOS--;continue;
	case 109://MOVE> 
		W=(int64_t)&stack[NOS-1]+(TOS<<3);W1=(int64_t)&stack[NOS]+(TOS<<3);
		while (TOS--) { W--;W1--;*(int64_t*)W=*(int64_t*)W1; }
		NOS-=2;TOS=stack[NOS];NOS--;continue;
	case 110://QFILL
		W1=(int64_t)&stack[NOS-1];op=stack[NOS];
		while (TOS--) { *(int64_t*)W=op;W++; }
		NOS-=2;TOS=stack[NOS];NOS--;continue;
	case 111://"UPDATE"
	case 112://"REDRAW"
	case 113://"MEM"
	case 114://"SW"
	case 115://"SH"
	case 116://"VFRAME"
	case 117://"XYPEN"
	case 118://"BPEN"
	case 119://"KEY"
	case 120://"MSEC"
	case 121://"TIME"
	case 122://"DATE"
	case 123://"LOAD"
	case 124://"SAVE"
	case 125://"APPEND"
	case 126://"FFIRST"
	case 127://"FNEXT"
		continue;
	case 128://"INK",
		gr_alpha((TOS>>24)^0xff);gr_color1=TOS;
		TOS=stack[NOS];NOS--;continue;
	case 129://"OP",
		gy1=TOS;gx1=stack[NOS];NOS--;TOS=stack[NOS];NOS--;continue;
	case 130:
		gr_line(gx1,gy1,stack[NOS],TOS);gx1=stack[NOS];gy1=TOS;
        NOS--;TOS=stack[NOS];NOS--;continue;
    case 131: 
		gr_spline(gx1,gy1,stack[NOS],TOS,stack[NOS-2],stack[NOS-1]);gx1=stack[NOS-2];gy1=stack[NOS-1];
		NOS-=3;TOS=stack[NOS];NOS--;continue;
    case 132: 
		gr_spline3(gx1,gy1,stack[NOS],TOS,stack[NOS-2],stack[NOS-1],stack[NOS-4],stack[NOS-3]);gx1=stack[NOS-4];gy1=stack[NOS-3];
		NOS-=5;TOS=stack[NOS];NOS--;continue;
	case 133:
		gr_pline(gx1,gy1,stack[NOS],TOS);gx1=stack[NOS];gy1=TOS;
        NOS--;TOS=stack[NOS];NOS--;continue;
    case 134:
		gr_pcurve(gx1,gy1,stack[NOS],TOS,stack[NOS-2],stack[NOS-1]);gx1=stack[NOS-2];gy1=stack[NOS-1];
		NOS-=3;TOS=stack[NOS];NOS--;continue;
    case 135:
		gr_pcurve3(gx1,gy1,stack[NOS],TOS,stack[NOS-2],stack[NOS-1],stack[NOS-4],stack[NOS-3]);gx1=stack[NOS-4];gy1=stack[NOS-3];
		NOS-=5;TOS=stack[NOS];NOS--;continue;
	case 136: 
		gr_drawPoli();continue;
//	case 111:systemcall(TOS,stack[NOS]);TOS=stack[NOS-1];NOS-=2;continue; //SYSCALL | nro int -- 
//	case 112:TOS=systemmem(TOS);continue;//SYSMEM | nro -- ini
	}
   }
}
	
SDL_Event evt;
	
////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{

r3compile("main.r3");
//dumpdicc();

dumpcode();

//freeinc();
//free(str);

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
