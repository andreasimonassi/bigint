.data

.code
	;int LongSubAsm(
	; 	qword  * A, 		RCX 
	; 	int ASizeInQWords, 	RDX
	; 	qword * B, 			R8
	; 	int BSizeInQWords, 	R9
	; 	qword * R			ON STACK
	; );			
; RAX, RCX, RDX, R8, R9, R10, R11  VOLATILE
; RAX used to store cpu flags
LongSubAsmVariant_1 proc
	
	;stack frame
	push rbp
	mov rbp, rsp
	sub rsp, 32     ;reserve space for 32 bytes of local variables = 8 integers					
	and spl, -16    ;align stack by 16 bytes (for sse code)

	push rbx ; rbx volatile -- used to store MSD position
	push r12;
	mov R11, 0
	mov r10, qword ptr [rbp + 60o]	; keep R in R10, R is in stack at rbp + 60o

	mov [rbp + 20o], rdx	;ASize in [rbp + 20o] that is second qword in shadow space

	clc	; clear carry
	lahf
	mov r12, -1 

	;undefined behavior if B > A
	_LOOP_B:
		cmp r9, r11
		jle _LOOP_B_END
		
		mov rbx, qword ptr [rcx + r11*8]	; A[index] in rbx
		mov rdx, qword ptr [r8 + r11*8]	; B[index] in rdx
		sahf ;restore flags
		sbb rbx, rdx 
		lahf
		cmovnz r12, r11
		mov qword ptr [r10 + r11*8], rbx
		add r11, 1				
		jmp _LOOP_B;

	_LOOP_B_END:

	mov rdx, [rbp + 20o]					; rdx = size A

	_LOOP_A:
		cmp rdx, r11
		jle _LOOP_A_END
		mov rbx, qword ptr [rcx + r11*8]	; A[index] in rbx
		sahf
		sbb rbx, 0
		jnc _LOOP_A_NOBORROW_ENT
		lahf
		cmovnz r12, r11
		mov qword ptr [r10+ r11*8], rbx
		add r11,1		
		jmp _LOOP_A


		_LOOP_A_NOBORROW:
		cmp rdx, r11
		jle _LOOP_A_END
		mov rbx, qword ptr [rcx + r11*8]	; A[index] in rbx	
		_LOOP_A_NOBORROW_ENT:
		cmovnz r12, r11
		mov qword ptr [r10+ r11*8], rbx
		add r11,1		
		jmp _LOOP_A_NOBORROW

	_LOOP_A_END:
	
	

	mov rax, r12
	add rax, 1

	; resume stack frame
	pop r12;
	pop rbx;

	mov rsp, rbp
	pop rbp
ret
LongSubAsmVariant_1 endp
end