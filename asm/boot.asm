; boot.asm - MMUKO-OS Ring Boot Sector (512 bytes)
; OBINexus / NSIGII Human Rights Protocol
; nasm -f bin boot.asm -o boot.bin
; qemu-system-x86_64 -drive format=raw,file=boot.bin
BITS 16
ORG 0x7C00

; RIFT Header (8 bytes)
    db 'NXOB'           ; Magic
    db 0x02,0x00        ; Version, reserved
    db 0xFE,0x07        ; Checksum, flags

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    mov [drv], dl
    mov si, m0
    call puts

; PHASE 1: SPARSE - qubit ring init
ph1:
    mov si, m1
    call puts
    mov di, qr
    mov cx, 8
    mov al, 0x01
    rep stosb           ; Fill ring with half-spin
    ; Set compass directions via lookup
    mov si, qinit
    mov di, qr
    mov cx, 3
    rep movsb           ; Copy N/NE/E directions
    call probe
    cmp al, 0x55
    je ph2
    test al, al
    jz .w
    jmp failc
.w: mov si, mb
    call puts
    xor ah, ah
    int 0x16
    jmp ph1

; PHASE 2: REMEMBER - FS detect + S/SW/W
ph2:
    mov si, m2
    call puts
    mov al, [0x7DBE+4]
    cmp al, 0x07
    je .n
    cmp al, 0x0B
    je .f
    cmp al, 0x0C
    je .f
    mov si, mr
    jmp .pr
.n: mov si, mn
    jmp .pr
.f: mov si, mf
.pr:call puts
    ; Set S/SW/W qubits
    mov byte [qr+4], 0x41
    mov byte [qr+5], 0x51
    mov byte [qr+6], 0x61
    test byte [qr], 0x01
    jz faile

; PHASE 3: ACTIVE - complete ring
ph3:
    mov si, m3
    call puts
    mov byte [qr+3], 0x31
    mov byte [qr+7], 0x71
    mov cx, 8
    mov di, qr
.a: or byte [di], 0x80
    inc di
    loop .a

; PHASE 4: VERIFY - NSIGII trinary check
ph4:
    mov si, m4
    call puts
    xor dx, dx
    mov cx, 8
    mov si, qr
.v: lodsb
    test al, 0x81       ; Check ACTIVE+HALFSPIN
    jz .sk
    cmp al, 0x81
    jb .sk
    inc dx
.sk:loop .v
    cmp dx, 6
    jge .g
    cmp dx, 4
    jge .y
    jmp .rd
.g: mov si, mg
    call puts
    jmp ok
.y: mov si, my
    call puts
    call probe
    cmp al, 0x55
    je ok
    jmp failc
.rd:mov si, mrd
    call puts
    jmp failv

ok: mov si, mo
    call puts
    mov al, 0x55
    out 0x80, al
    mov ah, 0x02
    mov al, 1
    xor ch, ch
    mov cl, 2
    xor dh, dh
    mov dl, [drv]
    mov bx, 0x8000
    int 0x13
    jc .h
    jmp 0x0000:0x8000
.h: hlt
    jmp .h

failc:
    mov si, efc
    jmp die
faile:
    mov si, efe
    jmp die
failv:
    mov si, efv
die:call puts
    mov al, 0xAA
    out 0x80, al
    hlt
    jmp die

; NSIGII probe: AL=0x55(yes)/0x00(maybe)/0xAA(no)
probe:
    push cx
    mov al, [0x0417]
    test al, al
    jnz .y
    mov al, 0
    out 0x70, al
    in al, 0x71
    mov ah, al
    mov cx, 0xFF
.d: loop .d
    mov al, 0
    out 0x70, al
    in al, 0x71
    cmp al, ah
    jne .y
    xor al, al
    pop cx
    ret
.y: mov al, 0x55
    pop cx
    ret

puts:
    pusha
    mov ah, 0x0E
    xor bx, bx
.l: lodsb
    test al, al
    jz .x
    int 0x10
    jmp .l
.x: popa
    ret

; Data
drv: db 0
qr:  times 8 db 0
qinit: db 0x01,0x11,0x21  ; N/NE/E compass init

; Messages
m0: db 'MMUKO v2',13,10,0
m1: db '1:SP',13,10,0
m2: db '2:RM',13,10,0
m3: db '3:AC',13,10,0
m4: db '4:VF',13,10,0
mb: db 'key?',13,10,0
mn: db 'NTFS',13,10,0
mf: db 'FAT',13,10,0
mr: db 'RAW',13,10,0
mg: db 'GRN',13,10,0
my: db 'YLW',13,10,0
mrd:db 'RED',13,10,0
mo: db 13,10,'OK',13,10,0
efc:db 'E:C',13,10,0
efe:db 'E:E',13,10,0
efv:db 'E:V',13,10,0

times 510-($-$$) db 0
dw 0xAA55
