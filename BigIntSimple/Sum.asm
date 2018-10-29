.data

.code
	;int LongSumAsm(
	; 	qword  * A, 		RCX 
	; 	int ASizeInQWords, 	RDX
	; 	qword * B, 			R8
	; 	int BSizeInQWords, 	R9
	; 	qword * R			ON STACK
	; );			
; RAX, RCX, RDX, R8, R9, R10, R11  VOLATILE
; RAX used to store cpu flags
LongSumAsm proc
	
	;stack frame
	push rbp
	mov rbp, rsp
	push rbx	;rbx is non volatile...
	
	
	; the following three instructions move the minimum of ASize, BSize into R9	
	mov r11, rdx	; suppoes minimum is ASize, thus Asize in R11
	cmp rdx, r9		; compare ASize, BSize
	
	mov [rbp + 20o], rdx	;ASize in [rbp + 20o] that is second qword in shadow space
	mov [rbp + 30o], r9     ;BSize in [rbp + 30o] that is third qword in shadow space	
	
	cmova r11, r9  ; if ASize > BSize , move BSize into R11
	
	
	clc	;clear carry
	lahf ;load flags in ah

	; use shadow stack space to save inputs
		
	mov r10, qword ptr [rbp + 60o]	; keep R in R10, R is in stack at rbp + 60o
	
	xor r9, r9 ; zero in r9	, from now on r9 will be the index on the arrays
	
	lea r11, [r11*8]
	
	A_And_B_Loop:							; while (ASize > i && BSize > i)				
		cmp r11, r9							; compare MinSize (r11) and array index (r9)
		jle A_And_B_Loop_Ends				; if index >= minsize quit loop
		mov rbx, qword ptr [rcx + r9]	    ; A[index] in rbx
		mov rdx, qword ptr [r8 + r9]	    ; B[index] in r11
		sahf								; reload flags from rax
		adc rbx, rdx						; add with carry A[index] + B[index]
		
		lahf								; store flags into rax
		mov qword ptr [r10+ r9], rbx;	; save to R[index] the result of A[index] + b[index]
		
		add r9, 8; ++i							; increase index
		jmp A_And_B_Loop					; continue loop
		
	A_And_B_Loop_Ends:
	
	xor r11, r11							; zero into r11	
	mov rdx, [rbp + 20o]					; rdx = size A
	lea rdx, [rdx*8]
	A_Only_Loop:							; while (ASize > index)	  
		cmp rdx, r9							; compare ASize (rdx) and array index
		jle A_Only_Loop_Ends                ; if(index <= ASize) quit loop
		mov rbx, qword ptr [rcx + r9] 	; A[index] into rbx
		sahf                                ; reload flags from rax into flags register
		adc rbx, 0                          ; add with carry A[index] + 0 (0 in r11)		
		lahf                                ; store flags into rax
		jnc A_NoCarry_Loop					; carry is zero no need to do more additions
		mov qword ptr [r10 + r9 ], rbx;	; save to R[index]	the result that's in rbx
		add r9,8 ; ++i						; increase index
		jmp A_Only_Loop                     ; continue loop

	A_NoCarry_Loop:
		mov qword ptr [r10+ r9], rbx;	; save to R[index] the result of A[index] + b[index]
		add r9, 8; ++i							; increase index
		cmp rdx, r9							; compare MinSize (rdx) and array index (r9)		
		mov rbx, qword ptr [rcx + r9] 	; A[index] into rbx
		jle A_Only_Loop_Ends				; if index >= minsize quit loop
		jmp A_NoCarry_Loop					; continue loop
	
	A_Only_Loop_Ends:
	
	mov rdx, [rbp + 30o]					; move BSize into rdx
	lea rdx, [rdx*8]
	B_Only_Loop:							; while (BSize > index)
		cmp rdx, r9							; compare BSize (rdx) with array index
		jle B_Only_Loop_Ends                ; if index <= BSize quit loop		
		mov rbx, qword ptr [r8 + r9] 	; B[index] into rbx				
		sahf                                ; reload carry flag into flag registers
		adc rbx, r11						; add with carry B[index] + 0 (0 in r11)		
		lahf								; store flags into rax
		jnc B_NoCarry_Loop					; carry is zero no need to do more additions
		mov qword ptr [r10 + r9], rbx;	; save to R[index] the result that's in rbx	
		add r9, 8; ++i							; increase index
		jmp B_Only_Loop						; continue loop

	B_NoCarry_Loop:
		mov qword ptr [r10+ r9], rbx;	; save to R[index] the result of B[index]
		add r9, 8; ++i							; increase index
		cmp rdx, r9							; compare BSIze (rdx) and array index (r9)
		jle B_Only_Loop_Ends				; if index >= BSIZE quit loop
		mov rbx, qword ptr [r8 + r9] 	; rbx into result
		jmp B_NoCarry_Loop					; continue loop

	B_Only_Loop_Ends:
	
	sahf									; reload flags into flag registers
	jnc Prepare_to_return 
	mov r11, 1							; add with carry 0 + 0 (last carry in r11)	
	mov qword ptr[r10 + r9], r11;       ; store the last carry flag into R[index]
	add r9, 8

									; return value
	Prepare_to_return:
	mov rax, r9
	shr rax, 3

	; resume stack frame
	
	pop rbx 
	mov rsp, rbp
	pop rbp
ret
LongSumAsm endp
end