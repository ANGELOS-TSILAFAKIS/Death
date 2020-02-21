
section .text
	global write_original_chunk
	extern memcpy

write_original_chunk:
	; TODO repnz stosb
	mov rdx, [rsp + 8]
	mov rsi, [rsp + 16]
	mov rdi, [rsp + 24]
	call memcpy
	ret
