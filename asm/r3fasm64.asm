;-------------------------------------
; win64 Interface
; PHREDA
;-------------------------------------
hInstance = $100000
format PE64 GUI 5.0 at hInstance

entry start

XRES equ 1024
YRES equ 600
STYLE  = WS_VISIBLE or WS_CAPTION or WS_SYSMENU
STYLEX = WS_EX_APPWINDOW

include 'include/win64w.inc'
include 'sdl2.inc'

section '' code readable executable

start:
        enter 8*20,0

        invoke SDL_Init,SDL_INIT_VIDEO or SDL_INIT_AUDIO
;        test eax,eax
;        jnz errorexit

;       invoke SDL_GetDesktopDisplayMode,0,desktop
;       test    eax,eax
;       jnz     create_window

;       mov     eax,[desktop.h]
;       mul     [viewport.w]
;       div     [desktop.w]
;       mov     [viewport.h],eax

;  create_window:

        invoke SDL_CreateWindow,_titulo,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,800,600,0 ;SDL_WINDOW_SHOWN

;       test    eax,eax
;       jz      initialization_error

;        mov     [window],eax

        ;SDL_ShowCursor(SDL_DISABLE);

;        invoke SDL_GetWindowSurface,window
;        mov [screen],eax

;        mov eax,[eax+SDL_Surface.pixels]

;        mov [SYSFRAME],eax

;    gr_buffer= screen->pixels;
;       invoke SDL_CreateRenderer,eax,-1,SDL_RENDERER_PRESENTVSYNC
;       test    eax,eax
;       jnz     renderer_ready
;       invoke SDL_CreateRenderer,[screen],-1,0
;       test    eax,eax
;       jz      initialization_error
 ;   renderer_ready:
;       mov     [renderer],eax
;       invoke SDL_RenderSetLogicalSize,[renderer],[viewport.w],[viewport.h]


;       invoke VirtualAlloc,0,MAXMEM,MEM_COMMIT+MEM_RESERVE,PAGE_READWRITE ;
        or rax,rax
;       jz SYSEND
        mov [FREE_MEM],rax

        jmp SYSEND

;---------- INICIO
;restart:
;        mov rbp,DSTACK
;        xor rax,rax
;        call inicio
;        jmp SYSEND
;----- CODE -----
inicio:
        mov esi,dword[SYSFRAME]
        mov ecx,640*480
        mov eax,$ff

l:
        mov dword[esi+ecx*4],eax
        dec ecx
        cmp ecx,0
        jg l

        call SYSREDRAW
;        call SYSUPDATE
;        mov eax,dword[SYSKEY]
;        cmp eax,1
 ;       jne inicio
;---------------------------
errorexit:
        jmp SYSEND
        ret

SYSREDRAW:
        invoke SDL_UpdateWindowSurface,[window]
        ret

SYSUPDATE:
        xor eax,eax
        mov [SYSKEY],eax
        mov [SYSCHAR],eax

        invoke SDL_Delay,10
        invoke SDL_PollEvent,evt
        mov eax,[evt.type]
        cmp eax,SDL_KEYDOWN
        je upkeyd
        cmp eax,SDL_KEYUP
        je upkeyu
        cmp eax,SDL_MOUSEBUTTONDOWN
        je upmobd
        cmp eax,SDL_MOUSEBUTTONUP
        je upmobu
        cmp eax,SDL_MOUSEMOTION
        je upmomo
        cmp eax,SDL_TEXTINPUT
        je uptext
        ret
upkeyd: ;key=(evt.key.keysym.sym&0xffff)|evt.key.keysym.sym>>16;break;
        ret
upkeyu: ;key=0x1000|(evt.key.keysym.sym&0xffff)|evt.key.keysym.sym>>16;break;
        ret
upmobd: ;bm|=evt.button.button;break;
        ret
upmobu: ;bm&=~evt.button.button;break;
        ret
upmomo: ;xm=evt.motion.x;ym=evt.motion.y;break;
        ret
uptext: ;keychar=*(int*)evt.text.text;break;
        ret
;key=0;
;keychar=0;
;SDL_Delay(10); // cpu free time
;if (SDL_PollEvent(&evt)) {
;       switch (evt.type) {
;       case SDL_KEYDOWN:key=(evt.key.keysym.sym&0xffff)|evt.key.keysym.sym>>16;break;
;       case SDL_KEYUP: key=0x1000|(evt.key.keysym.sym&0xffff)|evt.key.keysym.sym>>16;break;
;       case SDL_MOUSEBUTTONDOWN:bm|=evt.button.button;break;
;       case SDL_MOUSEBUTTONUP:bm&=~evt.button.button;break;
;       case SDL_MOUSEMOTION:xm=evt.motion.x;ym=evt.motion.y;break;
;       case SDL_TEXTINPUT: keychar=*(int*)evt.text.text;break;
;               }
;       }

SYSEND:
        invoke SDL_DestroyWindow,[window]
        invoke SDL_Quit
        leave
        ret

section '.data' data readable writeable

align 16

;        viewport SDL_Rect 0,0,960,540
;        desktop SDL_DisplayMode

        window dq ? ;SDL_Window
        screen dq ? ;SDL_Surface

        renderer dq ?
        keyboard dq ?

        evt SDL_Event
        _titulo db 'r3d',0

;        msg     MSG
;        hWnd    dq ?
;       hDC     dq ?
;       rec     RECT
;        bmi     BITMAPINFOHEADER
;        SysTime SYSTEMTIME
;        class   WNDCLASSEX sizeof.WNDCLASSEX,0,win_proc,0,0,hInstance,0,0,0,0,CLASSNAME,0
;        CLASSNAME TCHAR 'r3d',0

align 16
;        include 'data.asm'

align 16
        hdir            dq ?
        afile           dq ?
        cntr            dq ?
        SYSXYM          dd ?
        SYSBM           dd ?
        SYSKEY          dd ?
        SYSCHAR         dd ?
        FREE_MEM        dq ?

align 16 ; CUADRO DE VIDEO (FrameBuffer)
        DSTACK    rq 256
        SYSFRAME  dq ? ;rd XRES*YRES

section '' import readable writeable

  library kernel32,'KERNEL32',\
          user32,'USER32',\
          sdl2,'SDL2'

  include 'include\api\kernel32.inc'
  include 'include\api\user32.inc'
  include 'sdl2_api.inc'

