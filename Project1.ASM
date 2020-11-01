format PE console
entry start

include 'win32a.inc'

;--------------------------------------------------------------------------
section '.data' data readable writable

        mes1 db 'input string: ', 0
        strScan  db '%c', 0
        p db 'pause',0
        strVecElemOut  db '[%c] = %d', 10, 0
        string       dd ?
        i            dd ?
        tmp          dd ?
        mas          rd 100
        vec_size     dd 95   ;because we use only 32-126 ASCII-symbols
;--------------------------------------------------------------------------
section '.code' code readable executable
start:
; 1) filling vector mas with 0
         call getVector
; 2) input string and count symbols
        call strInput
; 3) vector mas out
       call  vectorOut
finish:
        call [getch]
        push 0
        call [ExitProcess]
;--------------------------------------------------------------------------

getVector:                   ;filling mas with 0
        xor ecx, ecx
        mov ebx, mas
getVecLoop:
        mov [tmp], ebx
        cmp ecx, [vec_size]
        jge endFillingVector
        mov [i], ecx
        mov ebx,0
        mov ecx, [i]
        inc ecx
        mov ebx, [tmp]
        add ebx, 4
        jmp getVecLoop
endFillingVector:
        ret
;--------------------------------------------------------------------------
strInput:                     ;invite to input string
        push mes1
        call [printf]
        add esp, 4
checkNewElem:                 ;check what is input symbol
push string
        push strScan
        call [scanf]
        add esp, 8
        mov esi,string
        lodsb                 ;read elem in register esi
        cmp al,0x0A           ;while key Enter didn't press
        je endStrInput
        sub al,32             ;because we use only 32-126 ascii symbols
        mov edi,eax
        mov eax,[mas+edi*4]   ;increase count of input element in mas by 1
        inc eax
        mov [mas+edi*4],eax
        jmp checkNewElem
endStrInput:
        ret
;--------------------------------------------------------------------------
 vectorOut:
        xor ecx,ecx           ;cout vector
        mov ebx,mas

        putVecLoop:
        mov [tmp], ebx
        cmp ecx,[vec_size]
        je endOutputVector
        mov [i], ecx
        add ecx,32            ;because we use only 32-126 ascii symbols
        push dword [ebx]
        push ecx
        push strVecElemOut
        call [printf]
        sub ecx,32
        mov ecx, [i]
        inc ecx
        mov ebx, [tmp]
        add ebx, 4
        jmp putVecLoop
endOutputVector:
        cinvoke system,p
;--------------------------------------------------------------------------
section '.idata' import data readable
    library kernel, 'kernel32.dll',\
            msvcrt, 'msvcrt.dll',\
            user32,'USER32.DLL'

include 'api\user32.inc'
include 'api\kernel32.inc'
    import kernel,\
           ExitProcess, 'ExitProcess',\
           HeapCreate,'HeapCreate',\
           HeapAlloc,'HeapAlloc'
  include 'api\kernel32.inc'
    import msvcrt,\
           printf, 'printf',\
           scanf, 'scanf',\
           getch, '_getch',\
           system,'system'
