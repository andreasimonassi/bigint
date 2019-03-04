.data

.code
	;qword cpu_divide(
	; 	qword  A, 		RCX 
	; 	qword  B, 		RDX
	; 	qword  * high, 			R8
	; );			
; RAX, RCX, RDX, R8, R9, R10, R11  VOLATILE
; RAX used to store cpu flags
cpu_multiply proc
	mov rax, rdx	
	mul rcx
	mov qword ptr[r8], rdx
	ret
cpu_multiply endp
end