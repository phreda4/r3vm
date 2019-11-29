format PE64 GUI 5.0

entry start

XRES equ 1024
YRES equ 600

include 'include/win64w.inc'
include 'sdl2.inc'

section '' code readable executable

start:
        sub     rsp,40
        cinvoke SDL_CreateWindow,_title,\
                          SDL_WINDOWPOS_UNDEFINED,\
                          SDL_WINDOWPOS_UNDEFINED,\
                          XRES,YRES,\
                          SDL_WINDOW_SHOWN
        test    eax,eax
        jz      initialization_error
        mov     [window],eax
        cinvoke SDL_ShowCursor,0
        cinvoke SDL_GetWindowSurface,[window]
        mov     rbx,rax
        mov     [screen],eax
        mov     rdi,[rbx+SDL_Surface.pixels]
        mov     [SYSFRAME],rdi

loop1:
        call paint

        call SYSREDRAW
        call SYSUPDATE
        mov eax,[SYSKEY]
        cmp eax,27
        je SYSEND
        jmp loop1
;----------------------------------
paint:
        mov rdi,[SYSFRAME]
        mov edx,[viewport.h]

        mov r8d,[color]
        add [color],1

       ; call SYSMSEC

    paint1:
        mov eax,r8d
;        mov     al,dl
;        and     eax,7Fh
;        imul    eax,020102h
;        or      eax,0FF000000h
        mov     ecx,[viewport.w]
        push    rdi
        rep     stosd
        pop     rdi
        mov     eax,[rbx+SDL_Surface.pitch]
        add     rdi,rax
        dec     edx
        jnz     paint1
        ret

;----------------------------------
SYSMSEC:
  ;sub rsp,8
      mov rax,rsp
      and rax,$f
      sub rsp,rax
      push rax rbx
   cinvoke SDL_GetTicks
      pop rbx rax
      add rsp,rax
   ;add rsp,8
   ret

SYSREDRAW:
        cinvoke SDL_UpdateWindowSurface,[window]
        ret

SYSUPDATE:
        xor eax,eax
        mov [SYSKEY],eax
        mov [SYSCHAR],eax
        sub rsp,8
        cinvoke SDL_Delay,10
        cinvoke SDL_PollEvent,evt
        add rsp,8
;        test eax,eax
;        jz .endr
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
        cmp eax,SDL_QUIT
        je SYSEND
;.endr:
        ret

upkeyd: ;key=(evt.key.keysym.sym&0xffff)|evt.key.keysym.sym>>16;break;
        mov eax,[evt.key.keysym.sym]
        and eax,0xffff
        mov ebx,[evt.key.keysym.sym]
        shr ebx,16
        or eax,ebx
        mov [SYSKEY],eax
        ret
upkeyu: ;key=0x1000|(evt.key.keysym.sym&0xffff)|evt.key.keysym.sym>>16;break;
        mov eax,[evt.key.keysym.sym]
        and eax,0xffff
        mov ebx,[evt.key.keysym.sym]
        shr ebx,16
        or eax,0x1000
        or eax,ebx
        mov [SYSKEY],eax
        ret
upmobd: ;bm|=evt.button.button;break;
        movzx eax,byte[evt.button.button]
        or [SYSBM],eax
        ret
upmobu: ;bm&=~evt.button.button;break;
        movzx eax,[evt.button.button]
        not eax
        and [SYSBM],eax
        ret
upmomo: ;xm=evt.motion.x;ym=evt.motion.y;break;
        mov eax,[evt.motion.x]
        mov [SYSXM],eax
        mov eax,[evt.motion.y]
        mov [SYSYM],eax
        ret
uptext: ;keychar=*(int*)evt.text.text;break;
        movzx eax,byte[evt.text.text]
        mov [SYSCHAR],eax
        ret

initialization_error:
        invoke  MessageBox,HWND_DESKTOP,_error,_title,MB_ICONERROR+MB_OK

SYSEND:
        cinvoke SDL_DestroyWindow,[window]
        cinvoke SDL_Quit
        add     rsp,40
        ret

section '.data' data readable writeable

  _title db "r3d",0
  _error db "err",0

  viewport SDL_Rect 0,0,XRES,YRES

  window dd ?
  screen dd ?

color dd 0
  evt SDL_Event

        SYSXM          dd ?
        SYSYM          dd ?
        SYSBM           dd ?
        SYSKEY          dd ?
        SYSCHAR         dd ?
        FREE_MEM        dq ?
        DSTACK          rq 256
        SYSFRAME        dq ? ;rd XRES*YRES

section '.idata' import readable

  library kernel32,'KERNEL32',\
          user32,'USER32',\
          sdl2,'SDL2'
  include 'include\api\kernel32.inc'
  include 'include\api\user32.inc'
  include 'sdl2_api.inc'