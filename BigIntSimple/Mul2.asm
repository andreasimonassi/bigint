.data

.code
	;int LongMulAsm2(
	; 	qword  * A, 		RCX 
	; 	int ASizeInQWords, 	eDX
	; 	qword * B, 			R8
	; 	int BSizeInQWords, 	R9d
	; 	qword * R			ON STACK
	; );			
; RAX, RCX, RDX, R8, R9, R10, R11  VOLATILE
; RAX used to store cpu flags
LongMulAsmVariant_1 proc
	
	; mul by zero ret 0
	xor rax, rax
	test rdx, rdx
	jz immediate_ret 	
	test r9,r9
	jz immediate_ret

	push rbp
	mov rbp, rsp
	
	push r12
	push r13
	push r14
	push r15

	mov r10, rdx
	
	mov r11, [rbp + 60o]  ; r11 <- R

	;reset R array	
	add rdx, r9	
    xor rax,rax
	sub edx, 1
_reset_loop:
    mov  [r11+rdx*8], rax           
    sub  edx, 1  
    jns  _reset_loop
	

	xor r12, r12

	
_outer_loop:
	cmp r12, r10 ; r12 = j
	je _outer_loop_end

		; it is unlikely that one digit is zero so... don't optimize that case
		xor r13,r13
		
		; k= i + j

		mov r15, [rcx +  r12 * 8]
		
		_inner_loop:
			cmp r13, r9
			je _inner_loop_end		
		
			lea r14, [r11 + r12 * 8]

			; pointer to result
			; multiply *A * *B
			mov rax, [r8 + r13 * 8]

			lea r14, [r14 + r13*8]   ;randomly mixing instructions in the hope to achieve better results

			mul r15	
			
		
			add rax, [r14]		; R[i+j] += loword (first part)
			adc rdx, [r14+8]	; R[i+j+1] += highword + carry (first part)
			mov [r14], rax		; R[i+j] += loword (second part)
			mov [r14+8], rdx	; R[i+j+1] += highword + carry (second part)
			jnc _inner_loop_continue
				lea r14, [r14+8]
				_carry_loop:				
					lea r14, [r14+8]
					mov rax, [r14]
					add rax, 1
					mov [r14], rax
					jnc _carry_loop_ends
					jmp _carry_loop
				_carry_loop_ends:

			_inner_loop_continue:
			
			inc r13
			jmp _inner_loop
		_inner_loop_end:

	_outer_loop_continue:
	add r12,1
	jmp _outer_loop
_outer_loop_end:
	
	;compute size of result, the number of iteration will be 1 or 2 if
	; input numbers have no leading zeroes

		
	lea rax, [r11 + r10 * 8]
	lea rax, [rax + r9 * 8]

		_compute_size_loop:
			cmp r11, rax ;    pointer to begin of array == pointer to end of array ?
			jg  _end_compute_size_loop	
			sub rax, 8	; pointer to end --
			mov r10, [rax]
			test r10, r10  ; *pointer_to_end == 0 ?
			jz _compute_size_loop
		_end_compute_size_loop:
	
	add rax, 8; need size not index
	sub rax, r11 ; distance in bytes	
	shr eax, 3 ; distance in words , this is the return value
	
	
; resume stack frame & non volatile

	pop r15	
	pop r14
	pop r13
	pop r12

	mov rsp, rbp
	pop rbp

	immediate_ret:
ret
LongMulAsmVariant_1 endp
end