////////////////////////////////////////////////////////////
// r3 concatenative programing language - Pablo Hugo Reda
//
// Compiler to dword-code and virtual machine for r3 lang, 
//  with cell size of 64 bits, 
//	SDL graphics windows
//
//#define DEBUGWORD
#define VIDEOWORD
#define SOUNDWORD

#include <stdio.h>
#include <time.h>

#include "graf.h"

#ifdef VIDEOWORD
#include "video.h"
#endif

//----------------------
/*------COMPILER------*/
//----------------------
// 0-imm 1-code 2-data 3-reserve 4-bytes 5-qwords
int modo=0; 

char *cerror=0;
char *werror;

int boot=-1;

int memcsize=0;
int memc=0;
int *memcode;

int memdsize=0x100000;			// 1MB data 
int scrf=0,srcw=800,srch=600;
int memd=0;
char *memdata;

char path[1024];

int64_t nro=0;

//---- includes
struct Include { char *nombre;char *str; };

int cntincludes;
Include includes[128];
int cntstacki;
int stacki[128];

//---- local dicctionary
struct Indice {	char *nombre;int mem;int info; };

int cntdicc;
int dicclocal;
Indice dicc[8192];

//----- aux stack for compilation
int level;
int cntstacka;
int stacka[256];
int lastblock;

void iniA(void) { cntstacka=0; }
void pushA(int n) { stacka[cntstacka++]=n; }
int popA(void) { return stacka[--cntstacka]; }

//----- internal tokens, replace 8 first names
const char *r3asm[]={
";","LIT1","ADR","CALL","VAR"
};

//------ base dictionary, machine-forth or machine-r3
const char *r3bas[]={
";","(",")","[","]",
"EX",
"0?","1?","+?","-?", 								
"<?",">?","=?",">=?","<=?","<>?","AN?","NA?","BT?",
"DUP","DROP","OVER","PICK2","PICK3","PICK4","SWAP","NIP",
"ROT","2DUP","2DROP","3DROP","4DROP","2OVER","2SWAP",
">R","R>","R@",
"AND","OR","XOR",
"+","-","*","/",
"<<",">>",">>>",
"MOD","/MOD","*/","*>>","<</",
"NOT","NEG","ABS","SQRT","CLZ",
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
"XYPEN","BPEN","KEY","CHAR",
"MSEC","TIME","DATE",
"LOAD","SAVE","APPEND",
"FFIRST","FNEXT",

"INK","'INK","ALPHA","OPX","OPY", // 'INK allow compile replace
"OP","LINE","CURVE","CURVE3","PLINE","PCURVE","PCURVE3","POLI",

"SYS",

#ifdef VIDEOWORD
"VIDEO","VIDEOSHOW","VIDEOSIZE",
#endif

#ifdef SOUNDWORD
"SLOAD","SPLAY",
#endif

#ifdef DEBUGWORD
"DEBUG","TDEBUG",	// DEBUG
#endif

"",// !!cut the dicc!!!
"JMP","JMPR","LIT2","LIT3",	// internal only
"AND_L","OR_L","XOR_L",		// OPTIMIZATION WORDS
"+_L","-_L","*_L","/_L",
"<<_L",">>_L",">>>_L",
"MOD_L","/MOD_L","*/_L","*>>_L","<</_L",
"<?_L",">?_L","=?_L",">=?_L","<=?_L","<>?_L","AN?_L","NA?_L",

};

//------ enumaration for table jump
enum {
FIN,LIT,ADR,CALL,VAR, 
EX,
ZIF,UIF,PIF,NIF,
IFL,IFG,IFE,IFGE,IFLE,IFNE,IFAND,IFNAND,IFBT,
DUP,DROP,OVER,PICK2,PICK3,PICK4,SWAP,NIP,
ROT,DUP2,DROP2,DROP3,DROP4,OVER2,SWAP2,
TOR,RFROM,ERRE,
AND,OR,XOR,
ADD,SUB,MUL,DIV,
SHL,SHR,SHR0,
MOD,DIVMOD,MULDIV,MULSHR,CDIVSH,
NOT,NEG,ABS,CSQRT,CLZ,
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
XYPEN,BPEN,KEY,KCHAR,
MSEC,TIME,IDATE,
LOAD,SAVE,APPEND,
FFIRST,FNEXT,

INK,INKA,ALPHA,OPX,OPY,
OP,LINE,CURVE,CURVE3,PLINE,PCURVE,PCURVE3,POLI,

SYS,
#ifdef VIDEOWORD
VIDEO,VIDEOSHOW,VIDEOSIZE,
#endif

#ifdef SOUNDWORD
SLOAD,SPLAY,
#endif

#ifdef DEBUGWORD
DEBUG,TDEBUG,	// DEBUG
#endif

ENDWORD, // !! cut the dicc !!!
JMP,JMPR,LIT2,LIT3,	// internal
AND1,OR1,XOR1,		// OPTIMIZATION WORDS
ADD1,SUB1,MUL1,DIV1,
SHL1,SHR1,SHR01,
MOD1,DIVMOD1,MULDIV1,MULSHR1,CDIVSH1,
IFL1,IFG1,IFE1,IFGE1,IFLE1,IFNE1,IFAND1,IFNAND1
};

//////////////////////////////////////
// DEBUG -- remove when all work ok
//////////////////////////////////////
void printword(char *s)
{
while (*s>32) putchar(*s++);
putchar(' ');
}

void printcode(int n)
{
if ((n&0xff)<5 && n!=0) {
	printf(r3asm[n&0xff]);printf(" %d",n>>8);
} else if (((n&0xff)>=IFL && (n&0xff)<=IFNAND) || (n&0xff)==JMPR) {	
	printf(r3bas[n&0xff]);printf(" >> %d",n>>8);
} else if ((n&0xff)>=IFL1 && (n&0xff)<=IFNAND1) {	
	printf(r3bas[n&0xff]);printf(" %d",n>>16);printf(" >> %d",n<<16>>24);
} else if ((n&0xff)>ENDWORD ) {
	printf(r3bas[n&0xff]);printf(" %d",n>>8);	
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
	printcode(memcode[i]);
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


//////////////////////////////////////
// Compiler: from text to dwordcodes
//////////////////////////////////////

// scan for a valid number begin in *p char
// return number in global var "nro"

int isNro(char *p)
{
//if (*p=='&') { p++;nro=*p;return -1;} // codigo ascii
int dig=0,signo=0,base;
if (*p=='-') { p++;signo=1; } else if (*p=='+') p++;
if ((unsigned char)*p<33) return 0;// no es numero
switch(*p) {
  case '$': base=16;p++;break;// hexa
  case '%': base=2;p++;break;// binario
  default:  base=10;break; 
  }; 
nro=0;if ((unsigned char)*p<33) return 0;// no es numero
while ((unsigned char)*p>32) {
  if (base!=10 && *p=='.') dig=0;  
  else if (*p<='9') dig=*p-'0'; 
  else if (*p>='a') dig=*p-'a'+10;  
  else if (*p>='A') dig=*p-'A'+10;  
  else return 0;
  if (dig<0 || dig>=base) return 0;
  nro*=base;nro+=dig;
  p++;
  };
if (signo==1) nro=-nro;  
return -1; 
};

// scan for a valid fixed point number begin in *p char
// return number in global var "nro"

int isNrof(char *p)         // decimal punto fijo 16.16
{
int64_t parte0;
int dig=0,signo=0;
if (*p=='-') { p++;signo=1; } else if (*p=='+') p++;
if ((unsigned char)*p<33) return 0;// no es numero
nro=0;
while ((unsigned char)*p>32) {
  if (*p=='.') { parte0=nro;nro=1;if (*(p+1)<33) return 0; } 
  else  {
  	if (*p<='9') dig=*p-'0'; else dig=-1;
  	if (dig<0 || dig>9) return 0;
  	nro=(nro*10)+dig;
  	}
  p++;
  };  
int decim=1;
int64_t num=nro;
while (num>1) { decim*=10;num/=10; }
num=0x10000*nro/decim;
nro=(num&0xffff)|(parte0<<16);
if (signo==1) nro=-nro;
return -1; 
};

// uppercase a char
char toupp(char c)
{ 
return c&0xdf;
}

// compare two words, until space	
int strequal(char *s1,char *s2)
{
while ((unsigned char)*s1>32) {
	if (toupp(*s2++)!=toupp(*s1++)) return 0;
	}
if (((unsigned char)*s2)>32) return 0;
return -1;
}
	
// advance pointer with space	
char *trim(char *s)	
{
while (((unsigned char)*s)<33&&*s!=0) s++;
return s;
}

// advance to next word
char *nextw(char *s)	
{
while (((unsigned char)*s)>32) s++;
return s;
}

// advance to next line
char *nextcr(char *s)	
{
while (((unsigned char)*s)>31||*s==9) s++;
return s;
}

// advance to next string ("), admit "" for insert " in a string, multiline is ok too
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

// ask for a word in the basic dicc
int isBas(char *p)
{    
nro=0;
char **m=(char**)r3bas;
while (**m!=0) {
  if (strequal(*m,p)) return -1;
  *m++;nro++; }
return 0;  
};

// ask for a word in the dicc, calculate local or exported too
int isWord(char *p) 
{ 
int i=cntdicc;
while (--i>-1) { 
	if (strequal(dicc[i].nombre,p) && ((dicc[i].info&1)==1 || i>=dicclocal)) return i;
	}
return -1;
};

// compile a token (int)
void codetok(int nro) 
{ 
memcode[memc++]=nro; 
}

// close variable definition with a place when no definition
void closevar() 
{
if (cntdicc==0) return;
if (!dicc[cntdicc-1].info&0x10) return; // prev is var
if (dicc[cntdicc-1].mem<memd) return;  		// have val
memdata[memd]=0;memd+=4;
}

// compile data definition, a VAR
void compilaDATA(char *str) 
{ 
int ex=0;
closevar();
if (*(str+1)=='#') { ex=1;str++; } // exported
dicc[cntdicc].nombre=str+1;
memd+=memd&3; // align data!!! (FILL break error)
dicc[cntdicc].mem=memd;
dicc[cntdicc].info=ex+0x10;	// 0x10 es dato
cntdicc++;
modo=2;
}

// compile a code definition, a WORD	
void compilaCODE(char *str) 
{ 
int ex=0;
closevar();
if (*(str+1)==':') { ex=1;str++; } // exported
if (*(str+1)<33) { boot=memc; }
dicc[cntdicc].nombre=str+1;
dicc[cntdicc].mem=memc;
dicc[cntdicc].info=ex;	// 0x10 es dato
cntdicc++;
modo=1;
}

// store in datamemory a string
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

// compile a string, in code save the token to retrieve too.
void compilaSTR(char *str) 
{
str++;
int ini=datasave(str);	
if (modo<2) codetok((ini<<8)+ADR); // lit data
}

// Store in datamemory a number or reserve mem
void datanro(int64_t n) { 
char *p=&memdata[memd];	
switch(modo){
	case 2:*(int*)p=(int)n;memd+=4;break;
	case 3:	for(int i=0;i<n;i++) { *p++=0; };memd+=n;break;
	case 4:*p=(char)n;memd+=1;break;
	case 5:*(int64_t*)p=(int64_t)n;memd+=8;break;
	}
}

// Compile adress of var
void compilaADDR(int n) 
{
if (modo>1) { 
	if ((dicc[n].info&0x10)==0)
		datanro(dicc[n].mem);
	else
		datanro((int64_t)&memdata[dicc[n].mem]);	
	return; 
	}
codetok((dicc[n].mem<<8)+LIT+((dicc[n].info>>4)&1));  //1 code 2 data
}

// Compile literal
void compilaLIT(int64_t n) 
{
if (modo>1) { datanro(n);return; }
int token=n;
codetok((token<<8)+LIT); 
if ((token<<8>>8)==n) return;
token=n>>24;
codetok((token<<8)+LIT2); 
if ((token<<8>>8)==(n>>24)) return;
token=n>>48;
codetok((token<<8)+LIT3); 
}

// Start block code (
void blockIn(void)
{
pushA(memc);
level++;
}

// solve conditional void
int solvejmp(int from,int to) 
{
int whi=false;
for (int i=from;i<to;i++) {
	int op=memcode[i]&0xff;
	if (op>=ZIF && op<=IFBT && (memcode[i]>>8)==0) { // patch while 
		memcode[i]|=(memc-i)<<8;	// full dir
		whi=true;
	} else if (op>=IFL1 && op<=IFNAND1 && (memcode[i]&0xff00)==0) { // patch while 
		memcode[i]|=((memc-i)<<8)&0xff00; // byte dir
		whi=true;
		}
	}
return whi;
}

// end block )
void blockOut(void)
{
int from=popA();
int dist=memc-from;
if (solvejmp(from,memc)) { // salta
	codetok((-(dist+1)<<8)+JMPR); 	// jmpr
} else { // patch if	
	if ((memcode[from-1]&0xff)>=IFL1 && (memcode[from-1]&0xff)<=IFNAND1) { 
		memcode[from-1]|=(dist<<8)&0xff00;	// byte dir
	} else {
		memcode[from-1]|=(dist<<8);		// full dir
		}
	}
level--;	
lastblock=memc;
}

// start anonymous definition (adress only word)
void anonIn(void)
{
pushA(memc);
codetok(JMP);	
level++;	
}

// end anonymous definition, save adress in stack
void anonOut(void)
{
int from=popA();
memcode[from]|=(memc<<8);	// patch jmp
codetok((from+1)<<8|LIT);
level--;	
}

// dicc base in data definition
void dataMAC(int n)
{
if (n==1) modo=4; // (	bytes
if (n==2) modo=2; // )
if (n==3) modo=5; // [	qwords
if (n==4) modo=2; // ]
if (n==MUL) modo=3; // * reserva bytes Qword Dword Kbytes
}

// compile word from base diccionary
void compilaMAC(int n) 
{
if (modo>1) { dataMAC(n);return; }
if (n==0) { 					// ;
	if (level==0) modo=0; 
	if ((memcode[memc-1]&0xff)==CALL && lastblock!=memc) { // avoid jmp to block
		memcode[memc-1]=(memcode[memc-1]^CALL)|JMP; // call->jmp avoid ret
		return;
		}
	}
if (n==1) { blockIn();return; }		//(	etiqueta
if (n==2) { blockOut();return; }	//)	salto
if (n==3) { anonIn();return; }		//[	salto:etiqueta
if (n==4) { anonOut();return; }		//]	etiqueta;push

int token=memcode[memc-1];

// optimize conditional jump to short version
if (n>=IFL && n<=IFNAND && (token&0xff)==LIT && (token<<8>>16)==(token>>8)) { 
	memcode[memc-1]=((token<<8)&0xffff0000)|(n-IFL+IFL1);
	return; 
	}

// optimize operation with constant
if (n>=AND && n<=CDIVSH && (token&0xff)==LIT) { 
	memcode[memc-1]=(token^LIT)|(n-ADD+ADD1);
	return; 
	}

codetok(n);	
}

// compile word
void compilaWORD(int n) 
{
if (modo>1) { datanro(n);return; }
codetok((dicc[n].mem<<8)+CALL+((dicc[n].info>>4)&1));
}

// --- error in code --
void seterror(char *f,char *s)
{
werror=s;
cerror=f;	
}

// print error info 
void printerror(char *name,char *src)
{
int line=1;
char *lc=src;
for (char *p=src;p<cerror;p++)
	if (*p==10) { 
		line++;lc=trim(p);
		}
*nextcr(lc)=0; // put 0 in the end of line
printf("in: ");printword(name);printf("\n\n");	
printf("%s\n",lc);
for(char *p=lc;p<cerror;p++) printf(" ");
printf("^-- ");
printf("ERROR %s in line %d\n\n",werror,line);	
}

// tokeniza string
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
			if (nro<0) { 
				if (isBas(str)) // 'ink allow compile replace
					{ compilaMAC(nro);str=nextw(str);break; }
				seterror(str,"adr not found");return 0; 
				}
			compilaADDR(nro);str=nextw(str);break;		
		default:
			if (isNro(str)||isNrof(str)) 
				{ compilaLIT(nro);str=nextw(str);break; }
			if (isBas(str)) 
				{ compilaMAC(nro);str=nextw(str);break; }
			nro=isWord(str);
			if (nro<0) { seterror(str,"word not found");return 0; }
			if (modo==1) 
				compilaWORD(nro); 
			else 
				compilaADDR(nro);
			str=nextw(str);break;
		}
	}
return -1;
}

// open, alloc and load file to string in memory
char *openfile(char *filename)
{
long len;
char *buff;
FILE *f=fopen(filename,"rb");
if (!f) {
	printf("FILE %s not found\n",filename);
	cerror=(char*)1;
	return 0;
	}
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

// include logic, not load many times
int isinclude(char *str)
{
char filename[1024];
char *fn=filename;	
char *ns=str;	

if (*str=='.') {
	str++;
	strcpy(filename,path);
	while (*fn!=0) fn++;
	}
	
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

// free source code of includes
void freeinc()
{
for (int i=0;i<cntincludes;i++){
	free(includes[cntincludes].str);
	}
}

//----------- comments / configuration
// |FULL			set fullscreen mode
// |SCR 640 480		set screen size
// |MEM 640 		set data memory size (in kb) min 1kb
char *syscom(char *str)
{
if (strnicmp(str,"|MEM ",5)==0) {	// memory in Kb
	if (isNro(trim(str+5))) {
		memdsize=nro<<10;
		if (memdsize<1024)	memdsize=1<<10;
		}
	}	
if (strnicmp(str,"|SCR ",5)==0) {	// screen size
	char *n=trim(str+5);
	if (isNro(n)) { srcw=nro;
		if (isNro(trim(nextw(n)))) srch=nro;
		}
	}
if (strnicmp(str,"|FULL",5)==0) {	// fullscreen mode
	scrf=1;
	}
return nextcr(str);
}

// resolve includes, recursive definition
void r3includes(char *str) 
{
if (str==0) return;
if (*str=='.') {
	
	}
	
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
			str=syscom(str);break;
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

// Compile code in file
int r3compile(char *name) 
{
printf("\nr3vm - PHREDA\n");
printf("compile:%s...",name);

char *sourcecode;

strcpy(path,name);// para ^. ahora pone el path del codigo origen
char *aa=path+strlen(path);
while (path<aa) { if (*aa=='/'||*aa=='\\') { *aa=0;break; } aa--; }
 
//printf("*%s*",path);

sourcecode=openfile(name);
if (sourcecode==0) return 0;
memcsize=0;
cntincludes=0;
cntstacki=0;
r3includes(sourcecode); // load includes

if (cerror!=0) return 0;
//dumpinc();

cntdicc=0;
dicclocal=0;
boot=-1;
memc=1; // direccion 0 para null
memd=0;

memcode=(int*)malloc(sizeof(int)*memcsize);
memdata=(char*)malloc(memdsize);

// tokenize includes
for (int i=0;i<cntstacki;i++) {
	if (!r3token(includes[stacki[i]].str)) {
		printerror(includes[stacki[i]].nombre,includes[stacki[i]].str);
		return 0;
		}
	dicclocal=cntdicc;
	}
// last tokenizer		
if (!r3token(sourcecode)) {
	printerror(name,sourcecode);
	return 0;
	}

//memd+=memd&3; // align

//dumpdicc();
//dumpcode();

printf(" ok.\n");
printf("inc:%d - words:%d - code:%dKb - data:%dKb - free:%dKb\n\n",cntincludes,cntdicc,memc>>8,memd>>10,(memdsize-memd)>>10);
freeinc();
free(sourcecode);
return -1;
}

	
//----------------------
/*--------RUNER--------*/
//----------------------
#define iclz(x) __builtin_clz(x)

// http://www.devmaster.net/articles/fixed-point-optimizations/
static inline int64_t isqrt(int64_t value)
{
if (value==0) return 0;
int bshft = (63-iclz(value))>>1;  // spot the difference!
int64_t g = 0;
int64_t b = 1<<bshft;
do {
	int64_t temp = (g+g+b)<<bshft;
	if (value >= temp) { g += b;value -= temp;	}
	b>>=1;
} while (bshft--);
return g;
}

//---------------------------//
// TOS..DSTACK--> <--RSTACK  //
//---------------------------//
#define STACKSIZE 256
int64_t stack[STACKSIZE];

SDL_Event evt;

WIN32_FIND_DATA ffd;
HANDLE hFind=NULL;

FILE *file;

time_t sit;
tm *sitime;

PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter
STARTUPINFO StartupInfo; //This is an [in] parameter

int sw,sh;
int xm=0;
int ym=0;
int bm=0;
int gx1=0;
int gy1=0;

int key=0;
char keychar;

// Update event for OS interaction
void r3update()
{
key=0;
keychar=0;
SDL_Delay(10); // cpu free time
if (SDL_PollEvent(&evt)) {
	switch (evt.type) {
	case SDL_KEYDOWN:key=(evt.key.keysym.sym&0xffff)|evt.key.keysym.sym>>16;break;
	case SDL_KEYUP:	key=0x1000|(evt.key.keysym.sym&0xffff)|evt.key.keysym.sym>>16;break;
	case SDL_MOUSEBUTTONDOWN:bm|=evt.button.button;break;
	case SDL_MOUSEBUTTONUP:bm&=~evt.button.button;break;
	case SDL_MOUSEMOTION:xm=evt.motion.x;ym=evt.motion.y;break;
	case SDL_TEXTINPUT: keychar=*(int*)evt.text.text;break;
		}
	}	
}
         
// run code, from adress "boot"
void runr3(int boot) 
{
stack[STACKSIZE-1]=0;	
register int64_t TOS=0;
register int64_t *NOS=&stack[0];
register int64_t *RTOS=&stack[STACKSIZE-1];
register int64_t REGA=0;
register int64_t REGB=0;
register int64_t op=0;
register int ip=boot;
register int64_t W=0;

while(ip!=0) { 
	op=memcode[ip++]; 

//	printcode(op);
	
	switch(op&0xff){
	case FIN:ip=*RTOS;RTOS++;continue; 							// ;
	case LIT:NOS++;*NOS=TOS;TOS=op>>8;continue;					// LIT1
	case ADR:NOS++;*NOS=TOS;TOS=(int64_t)&memdata[op>>8];continue;		// LIT adr
	case CALL:RTOS--;*RTOS=ip;ip=(unsigned int)op>>8;continue;	// CALL
	case VAR:NOS++;*NOS=TOS;TOS=*(int*)&memdata[op>>8];continue;// VAR
	case EX:RTOS--;*RTOS=ip;ip=TOS;TOS=*NOS;NOS--;continue;		//EX
	case ZIF:if (TOS!=0) {ip+=(op>>8);}; continue;//ZIF
	case UIF:if (TOS==0) {ip+=(op>>8);}; continue;//UIF
	case PIF:if (TOS<0) {ip+=(op>>8);}; continue;//PIF
	case NIF:if (TOS>=0) {ip+=(op>>8);}; continue;//NIF
	case IFL:if (TOS<=*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFL
	case IFG:if (TOS>=*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFG
	case IFE:if (TOS!=*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFN	
	case IFGE:if (TOS>*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFGE
	case IFLE:if (TOS<*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFLE
	case IFNE:if (TOS==*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFNO
	case IFAND:if (!(TOS&*NOS)) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFNA
	case IFNAND:if (TOS&*NOS) {ip+=(op>>8);} TOS=*NOS;NOS--;continue;//IFAN
	case IFBT:if (*(NOS-1)>TOS||*(NOS-1)<*NOS) {ip+=(op>>8);} 
		TOS=*(NOS-1);NOS-=2;continue;//BTW (need bit trick) 	//(TOS-*(NOS-1))|(*(NOS-1)-*NOS)>0
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
	case ADD:TOS=*NOS+TOS;NOS--;continue;				//SUMA
	case SUB:TOS=*NOS-TOS;NOS--;continue;				//RESTA
	case MUL:TOS=*NOS*TOS;NOS--;continue;				//MUL
	case DIV:TOS=(*NOS/TOS);NOS--;continue;				//DIV
	case SHL:TOS=*NOS<<TOS;NOS--;continue;				//SAl
	case SHR:TOS=*NOS>>TOS;NOS--;continue;				//SAR
	case SHR0:TOS=((uint64_t)*NOS)>>TOS;NOS--;continue;	//SHR
	case MOD:TOS=*NOS%TOS;NOS--;continue;					//MOD
	case DIVMOD:W=*NOS;*NOS=W/TOS;TOS=W%TOS;continue;	//DIVMOD
	case MULDIV:TOS=(*(NOS-1)*(*NOS)/TOS);NOS-=2;continue;	//MULDIV
	case MULSHR:TOS=(*(NOS-1)*(*NOS))>>TOS;NOS-=2;continue;	//MULSHR
	case CDIVSH:TOS=(*(NOS-1)<<TOS)/(*NOS);NOS-=2;continue;//CDIVSH
	case NOT:TOS=~TOS;continue;							//NOT
	case NEG:TOS=-TOS;continue;							//NEG
	case ABS:W=(TOS>>63);TOS=(TOS+W)^W;continue;		//ABS
	case CSQRT:TOS=isqrt(TOS);continue;					//CSQRT
	case CLZ:TOS=iclz(TOS);continue;					//CLZ
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
//		W=(int64_t)*(NOS-1);op=(int64_t)*NOS;
//		while (TOS--) { *(int*)W=*(int*)op;W+=4;op+=4; }
		memcpy((void*)*(NOS-1),(void*)*NOS,TOS<<2);
		NOS-=2;TOS=*NOS;NOS--;continue;
	case MOVEA://MOVE> 
//		W=(int64_t)*(NOS-1)+(TOS<<2);op=(int64_t)(*NOS)+(TOS<<2);
//		while (TOS--) { W-=4;op-=4;*(int*)W=*(int*)op; }
		memmove((void*)*(NOS-1),(void*)*NOS,TOS<<2);
		NOS-=2;TOS=*NOS;NOS--;continue;
	case FILL://FILL
		W=*(NOS-1);op=*NOS;
		while (TOS--) { *(int*)W=op;W+=4; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case CMOVED://CMOVE 
//		W=(int64_t)*(NOS-1);op=(int64_t)*NOS;
//		while (TOS--) { *(char*)W=*(char*)op;W++;op++; }
		memcpy((void*)*(NOS-1),(void*)*NOS,TOS);
		NOS-=2;TOS=*NOS;NOS--;continue;
	case CMOVEA://CMOVE> 
//		W=(int64_t)*(NOS-1)+TOS;op=(int64_t)*NOS+TOS;
//		while (TOS--) { W--;op--;*(char*)W=*(char*)op; }
		memmove((void*)*(NOS-1),(void*)*NOS,TOS);
		NOS-=2;TOS=*NOS;NOS--;continue;
	case CFILL://CFILL
		W=(int64_t)*(NOS-1);op=*NOS;
		while (TOS--) { *(char*)W=op;W++; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case QMOVED://QMOVE 
//		W=(uint64_t)*(NOS-1);op=(uint64_t)*NOS;
//		while (TOS--) { *(uint64_t*)W=*(uint64_t*)op;W+=8;op+=8; }
		memcpy((void*)*(NOS-1),(void*)*NOS,TOS<<3);	
		NOS-=2;TOS=*NOS;NOS--;continue;
	case QMOVEA://MOVE> 
//		W=(uint64_t)*(NOS-1)+(TOS<<3);op=(uint64_t)*NOS+(TOS<<3);
//		while (TOS--) { W-=8;op-=8;*(uint64_t*)W=*(uint64_t*)op; }
		memmove((void*)*(NOS-1),(void*)*NOS,TOS<<3);		
		NOS-=2;TOS=*NOS;NOS--;continue;
	case QFILL://QFILL
		W=(uint64_t)*(NOS-1);op=*NOS;
		while (TOS--) { *(uint64_t*)W=op;W+=8; }
		NOS-=2;TOS=*NOS;NOS--;continue;
	case UPDATE://"UPDATE"
		r3update();continue;
	case REDRAW://"REDRAW"
		gr_redraw();continue;
	case MEM://"MEM"
		NOS++;*NOS=TOS;TOS=(int64_t)&memdata[memd];continue;
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
		NOS++;*NOS=TOS;TOS=key;continue;	
	case KCHAR://"CHAR"
		NOS++;*NOS=TOS;TOS=keychar;continue;	
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
        
	case INK:	// INK
		NOS++;*NOS=TOS;TOS=gr_color1;continue;
	case INKA:	//'INK
		NOS++;*NOS=TOS;TOS=(int64_t)&gr_color1;continue;
	case ALPHA:	// ALPHA
		gr_alpha(TOS);TOS=*NOS;NOS--;continue;
	case OPX:	// OPX
		NOS++;*NOS=TOS;TOS=gx1;continue;	
	case OPY:	// OPY
		NOS++;*NOS=TOS;TOS=gy1;continue;	
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

	case SYS: 
//		printf("%s",TOS);
    	if (TOS==0) {	// 0 sys | end process
            if (ProcessInfo.hProcess!=0) {
               TerminateProcess(ProcessInfo.hProcess,0);
               CloseHandle(ProcessInfo.hThread);
               CloseHandle(ProcessInfo.hProcess);
               ProcessInfo.hProcess=0;
               }
            TOS=-1;
            continue; }
        if (TOS==-1) {	// -1 sys | end process??
            if (ProcessInfo.hProcess==0) continue;
            W=WaitForSingleObject(ProcessInfo.hProcess,0);
            if (W==WAIT_TIMEOUT) TOS=0; else TOS=-1;
            continue; }
        ZeroMemory(&StartupInfo, sizeof(StartupInfo));
        StartupInfo.cb=sizeof StartupInfo ; //Only compulsory field
        //TOS=CreateProcess(NULL,(char*)TOS,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&StartupInfo,&ProcessInfo);
		TOS=CreateProcess(NULL,(char*)TOS,NULL,NULL,FALSE,NULL,NULL,NULL,&StartupInfo,&ProcessInfo);        
		WaitForSingleObject(ProcessInfo.hProcess,INFINITE);// wait termination
		continue;
		
#ifdef VIDEOWORD
	case VIDEO:  // "file" w h -- 
        if (TOS==0) { NOS-=2;TOS=*NOS;NOS--;videoclose();continue; }
		videoopen((char*)*(NOS-1),*NOS,TOS);
		NOS-=2;TOS=*NOS;NOS--;
		continue;
	case VIDEOSHOW: // w h -- v
		TOS=redrawframe(*NOS,TOS);		
		NOS--;
		continue;		
	case VIDEOSIZE: // w h --
		videoresize(*NOS,TOS);
		NOS--;TOS=*NOS;NOS--;
		continue;		
		
#endif

#ifdef SOUNDWORD
    case SLOAD: // "" -- pp
        TOS=(int)Mix_LoadWAV((char *)TOS);
        continue;
    case SPLAY: // pp --
        if (TOS!=0) Mix_PlayChannel(-1,(Mix_Chunk *)TOS, 0);
//        else FSOUND_StopSound(FSOUND_ALL);
        TOS=*NOS;NOS--;
        continue;
/*
    case SINFO: // "" -- mm
         TOS=0;
         for(int i=0;i<FSOUND_GetMaxChannels();i++) TOS|=FSOUND_IsPlaying(i); 
         continue;
    case SSET: // pan vol frec mm --
         if (TOS!=0) FSOUND_Sample_SetDefaults((FSOUND_SAMPLE *)TOS,int(*NOS),int(*(NOS-1)),int(*(NOS-2)),-1);
        TOS=*(NOS-3);NOS-=4;continue;
*/

#endif

#ifdef DEBUGWORD //----------------- DEBUG
	case DEBUG:printf((char*)TOS);TOS=*NOS;NOS--;continue;
	case TDEBUG:printf("%d ",TOS);continue;	
#endif//----------------- DEBUG	
	
	case ENDWORD: continue;
//----------------- ONLY INTERNAL
	case JMP:ip=(op>>8);continue;//JMP							// JMP
	case JMPR:ip+=(op>>8);continue;//JMP						// JMPR	
	case LIT2:TOS=(TOS&0xffffff)|((op>>8)<<24);continue;		// LIT ....xxxxxxaaaaaa
	case LIT3:TOS=(TOS&0xffffffffffff)|((op>>8)<<48);continue;	// LIT xxxx......aaaaaa	
//----------------- OPTIMIZED WORD
	case AND1:TOS&=op>>8;continue;
	case OR1:TOS|=op>>8;continue;
	case XOR1:TOS^=op>>8;continue;
	case ADD1:TOS+=op>>8;continue;
	case SUB1:TOS-=op>>8;continue;
	case MUL1:TOS*=op>>8;continue;
	case DIV1:TOS/=op>>8;continue;
	case SHL1:TOS<<=op>>8;continue;
	case SHR1:TOS>>=op>>8;continue;
	case SHR01:TOS=(uint64_t)TOS>>(op>>8);continue;
	case MOD1:TOS=TOS%(op>>8);continue;
	case DIVMOD1:op>>=8;NOS++;*NOS=TOS/op;TOS=TOS%op;continue;	//DIVMOD
	case MULDIV1:op>>=8;TOS=(*NOS)*TOS/op;NOS--;continue;		//MULDIV
	case MULSHR1:op>>=8;TOS=((*NOS)*TOS)>>op;NOS--;continue;	//MULSHR
	case CDIVSH1:op>>=8;TOS=((*NOS)<<op)/TOS;NOS--;continue;	//CDIVSH
	case IFL1:if ((op<<32>>48)<=TOS) ip+=(op<<48>>56);continue;	//IFL
	case IFG1:if ((op<<32>>48)>=TOS) ip+=(op<<48>>56);continue;	//IFG
	case IFE1:if ((op<<32>>48)!=TOS) ip+=(op<<48>>56);continue;	//IFN
	case IFGE1:if ((op<<32>>48)>TOS) ip+=(op<<48>>56);continue;	//IFGE
	case IFLE1:if ((op<<32>>48)<TOS) ip+=(op<<48>>56);continue;	//IFLE
	case IFNE1:if ((op<<32>>48)==TOS) ip+=(op<<48>>56);continue;//IFNO
	case IFAND1:if (!((op<<32>>48)&TOS)) ip+=(op<<48>>56);continue;//IFNA
	case IFNAND1:if ((op<<32>>48)&TOS) ip+=(op<<48>>56);continue;//IFAN
		
	}
   }

}
	
	
////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
char filename[1024];
if (argc>1) 
	strcpy(filename,argv[1]); 
else 
	strcpy(filename,"main.r3");

if (!r3compile(filename)) return -1;

#ifdef VIDEOWORD
av_register_all();
avformat_network_init();
#endif

#ifdef SOUNDWORD
Mix_OpenAudio(22050,MIX_DEFAULT_FORMAT,2,4096);
#endif

gr_init(filename,srcw,srch,scrf);
SDL_StartTextInput();
runr3(boot);
SDL_StopTextInput();

#ifdef SOUNDWORD
Mix_CloseAudio();
#endif

#ifdef VIDEOWORD
videoclose();
avformat_network_deinit();
#endif

gr_fin();
return 0;
}
