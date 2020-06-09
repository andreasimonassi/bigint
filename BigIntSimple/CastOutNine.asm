.data

.code
    ;parameters:
    ;     int ASizeInQWords,     RCX
	;     qword  * A,         RDX    

CastingOutNines_asm proc		;computes A mod 2^64-1	

								;... we don't need any local stack frame

	xor rax,rax					;initial modulo is zero
	test rcx, rcx				;if ASizeInQWords = 0 ...
	jz branch2					; ... goto branch 2 (which is exit)

	clc							;set initial carry to zero
	sub rdx, 8					;since rcx counts from ASizeInQWords to 1 we decrease the rdx pointer by one qword (which is long 8 bytes)
	sum_modulo_loop:			;do {
		adc rax, [rdx+rcx*8]	;	rax = rax + rdx[rcx -1] + carry ... the -1 part is achieved by the sub rdx instruction above...
								;   ...the actual C instruction would be rax = rax + (rdx-1)[rcx] + carry;
		loop sum_modulo_loop	; } while rcx > 0 ... loop instruction decreases rcx and then jump to sum_modulo_loop if rcx not zero
	
	adc rax, 0					; add last carry to rax
	
	cmp rax, -1					; if rax = base-1 then goto branch1 else goto branch2
	jnz branch2
	branch1:
		xor rax,rax				; rax = 0
	branch2:
		ret						; returned value is rax
CastingOutNines_asm endp
end