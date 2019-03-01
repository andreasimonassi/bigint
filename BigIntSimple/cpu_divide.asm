.data

.code
	;qword cpu_divide(
	; 	qword  low, 		RCX 
	; 	qword  high, 		RDX
	; 	qword  divisor, 			R8
	; 	qword * R,			R9
	; );			
; RAX, RCX, RDX, R8, R9, R10, R11  VOLATILE
; RAX used to store cpu flags
cpu_divide proc
	mov rax, rcx	
	div r8
	mov qword ptr[r9], rdx
	ret
cpu_divide endp
end