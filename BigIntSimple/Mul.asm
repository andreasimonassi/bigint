.data

.code
	;int LongMulAsm(
	; 	qword  * A, 		RCX 
	; 	int ASizeInQWords, 	RDX
	; 	qword * B, 			R8
	; 	int BSizeInQWords, 	R9
	; 	qword * R			ON STACK
	; );			
; RAX, RCX, RDX, R8, R9, R10, R11  VOLATILE
; RAX used to store cpu flags
LongMulAsm proc
	
	; mul by zero ret 0
	xor rax, rax
	test rdx, rdx
	jz immediate_ret 	
	test r9,r9
	jz immediate_ret

	;stack frame
	push rbp
	mov rbp, rsp

	;non volatile
	push rdi
	push rsi
	push rbx
	push r12
	push r13
	push r14
	push r15
	

	mov rdi, qword ptr [rbp + 60o] 
	mov rsi, rdx
	

	;compute result size for zeroing output array
	mov rax, rdx 
	add rax, r9
	sub rax, 1
	
	reset_output_loop_start:
		js reset_output_loop_end
		mov qword ptr [rdi+rax*8], 0
		sub rax, 1 		
		jmp reset_output_loop_start
	reset_output_loop_end:
	
	; j = 0
	xor rbx, rbx 
	xor r15,r15
	sub r15, 1

	;while (j < m )
	left_number_loop_start:	

	cmp rbx, rdx 
	jnl left_number_loop_ends 

		; A[j] in rax
		mov rax, qword ptr [rcx + rbx * 8]   
		
		; if(A[j] == 0) continue; low cost optimization skip inner loop if number is zero
		test rax, rax
		jz left_number_loop_continue   

		;i=0;
		xor r10, r10 
		mov [rbp + 60o], rax     ;save A[j] in [rbp + 60o]

		;while(i < n)
		right_number_loop_start:
		cmp r10, r9 ; r9 size of right number
		jnl right_number_loop_ends

				;unsigned k = i + j;				
				mov r11, rbx ; k = j								
				add r11, r10 ; k += i
				
				; B[i] in r13
				mov r13, qword ptr [r8 + r10 * 8] ; B[i] in R13
				; if(B[i] == 0) continue;
				test r13,r13
				jz right_number_loop_continue ;low cost optimization, skip loop if zero

				;rax <-A[j]
				mov rax, [rbp + 60o]; 
				; rdx:rax = B[i] * A[j]
				mul r13 
				; if cf then overflow so dx has higher order digit
				mov r13, rax ; save value of rax, i need rax to store flags				
				mov r14, r11
				add r14, 1
				add qword ptr [rdi+r11*8], r13 ; R[k] = R[k] + loword							
				cmovnz r15, r11
				adc qword ptr [rdi+r14*8], rdx ; 
				cmovnz r15, r14

				propagate_carry:
					jnc no_more_carry
					inc r14 ; inc preserve carry
					adc qword ptr [rdi+r14*8], 0; 
					cmovnz r15, r14
					jmp propagate_Carry

				no_more_carry:

			right_number_loop_continue:
			add r10, 1
			jmp  right_number_loop_start

		right_number_loop_ends:

	left_number_loop_continue:
	add rbx, 1
	mov rdx, rsi; recover rdx from shadow copy
	jmp  left_number_loop_start
	left_number_loop_ends:
	; msd in rax to return
	mov rax , r15
	add rax, 1
	
	; resume stack frame & non volatile
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rsi
	pop rdi
	
	mov rsp, rbp
	pop rbp
	immediate_ret:
ret
LongMulAsm endp
end