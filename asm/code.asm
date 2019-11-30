INICIO:

loop1:
    call paint
       call SYSREDRAW
        call SYSUPDATE
        mov eax,[SYSKEY]
        cmp eax,27
        jne loop1
        ret

;----------------------------------
paint:
        mov rdi,[SYSFRAME]
        mov edx,YRES

        call SYSMSEC
        shr rax,1
        mov r8d,eax

;        call SYSTIME
;        sub rbp,8
 ;       mov r8d,eax

       ; call SYSMSEC

    paint1:
        mov eax,r8d
         add eax,edx
        mov     ecx,XRES
        push    rdi
        rep     stosd
        pop     rdi
        mov     eax,[rbx+SDL_Surface.pitch]
        add     rdi,rax
        dec     edx
        jnz     paint1
        ret

