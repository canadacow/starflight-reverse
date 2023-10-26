; =============== S U B R O U T I N E =======================================


sub_E4          proc near               ; CODE XREF: seg000:0171↓p
                mov     al, 0B6h
                out     43h, al         ; Timer 8253-5 (AT: 8254.2).
                mov     al, dl
                out     42h, al         ; Timer 8253-5 (AT: 8254.2).
                mov     al, dh
                out     42h, al         ; Timer 8253-5 (AT: 8254.2).
                in      al, 61h         ; PC/XT PPI port B bits:
                                        ; 0: Tmr 2 gate ═╦═► OR 03H=spkr ON
                                        ; 1: Tmr 2 data ═╝  AND 0fcH=spkr OFF
                                        ; 3: 1=read high switches
                                        ; 4: 0=enable RAM parity checking
                                        ; 5: 0=enable I/O channel check
                                        ; 6: 0=hold keyboard clock low
                                        ; 7: 0=enable kbrd
                or      al, 3
                out     61h, al         ; PC/XT PPI port B bits:
                                        ; 0: Tmr 2 gate ═╦═► OR 03H=spkr ON
                                        ; 1: Tmr 2 data ═╝  AND 0fcH=spkr OFF
                                        ; 3: 1=read high switches
                                        ; 4: 0=enable RAM parity checking
                                        ; 5: 0=enable I/O channel check
                                        ; 6: 0=hold keyboard clock low
                                        ; 7: 0=enable kbrd
                retn
sub_E4          endp


; =============== S U B R O U T I N E =======================================


sub_F7          proc near               ; CODE XREF: seg000:012E↓p
                mov     dx, 61h ; 'a'
                in      al, dx          ; PC/XT PPI port B bits:
                                        ; 0: Tmr 2 gate ═╦═► OR 03H=spkr ON
                                        ; 1: Tmr 2 data ═╝  AND 0fcH=spkr OFF
                                        ; 3: 1=read high switches
                                        ; 4: 0=enable RAM parity checking
                                        ; 5: 0=enable I/O channel check
                                        ; 6: 0=hold keyboard clock low
                                        ; 7: 0=enable kbrd
                and     al, 0FCh
                out     dx, al          ; PC/XT PPI port B bits:
                                        ; 0: Tmr 2 gate ═╦═► OR 03H=spkr ON
                                        ; 1: Tmr 2 data ═╝  AND 0fcH=spkr OFF
                                        ; 3: 1=read high switches
                                        ; 4: 0=enable RAM parity checking
                                        ; 5: 0=enable I/O channel check
                                        ; 6: 0=hold keyboard clock low
                                        ; 7: 0=enable kbrd
                retn
sub_F7          endp

; ---------------------------------------------------------------------------
                push    si
                push    di
                push    ds
                push    dx
                push    cx
                push    bx
                push    ax
                push    cs
                pop     ds
                mov     al, byte ptr ds:word_2
                test    al, al
                jnz     short loc_113
                jmp     loc_1AB
; ---------------------------------------------------------------------------

loc_113:                                ; CODE XREF: seg000:010E↑j
                xor     bx, bx
                dec     byte ptr [bx+0Ah]
                jz      short loc_11D

loc_11A:                                ; CODE XREF: seg000:0135↓j
                jmp     loc_1AB
; ---------------------------------------------------------------------------

loc_11D:                                ; CODE XREF: seg000:0118↑j
                mov     al, [bx+0Dh]
                test    al, al
                jnz     short loc_137
                mov     al, [bx+0Ch]
                test    al, al
                jz      short loc_137

loc_12B:                                ; CODE XREF: seg000:015B↓j
                mov     [bx+0Ah], al
                call    sub_F7
                mov     byte ptr [bx+0Dh], 1
                jmp     short loc_11A
; ---------------------------------------------------------------------------

loc_137:                                ; CODE XREF: seg000:0122↑j
                                        ; seg000:0129↑j ...
                mov     si, [bx+7]
                cld
                lodsb
                and     al, 7Fh
                jz      short loc_17B
                xor     ah, ah
                test    al, 40h
                jz      short loc_14A
                and     al, 3Fh
                inc     ah

loc_14A:                                ; CODE XREF: seg000:0144↑j
                mov     [bx+0Bh], al
                mov     [bx+0Ch], ah
                lodsb
                mov     [bx+7], si
                cmp     al, 0FFh
                jnz     short loc_15D
                mov     al, [bx+0Bh]
                jmp     short loc_12B
; ---------------------------------------------------------------------------

loc_15D:                                ; CODE XREF: seg000:0156↑j
                mov     ah, [bx+0Bh]
                sub     ah, [bx+0Ch]
                mov     [bx+0Ah], ah
                xor     dh, dh
                mov     dl, al
                shl     dx, 1
                mov     di, dx
                mov     dx, [di+24h]
                call    sub_E4
                mov     byte ptr [bx+0Dh], 0
                jmp     loc_1AB
; ---------------------------------------------------------------------------

loc_17B:                                ; CODE XREF: seg000:013E↑j
                mov     si, [bx+5]
                dec     byte ptr [bx+9]
                jnz     short loc_19B
                add     si, 3
                mov     al, [si]
                test    al, al
                jnz     short loc_195
                xor     ax, ax
                mov     ds:word_2, ax
                jmp     loc_1AB
; ---------------------------------------------------------------------------

loc_195:                                ; CODE XREF: seg000:018A↑j
                mov     [bx+5], si
                mov     [bx+9], al

loc_19B:                                ; CODE XREF: seg000:0181↑j
                mov     ax, [si+1]
                mov     [bx+7], ax
                mov     byte ptr [bx+0Ah], 1
                mov     byte ptr [bx+0Dh], 1
                jmp     short loc_137
; ---------------------------------------------------------------------------

loc_1AB:                                ; CODE XREF: seg000:0110↑j
                                        ; seg000:loc_11A↑j ...
                pop     ax
                pop     bx
                pop     cx
                pop     dx
                pop     ds
                pop     di
                pop     si
                iret