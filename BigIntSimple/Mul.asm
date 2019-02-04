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
	
	;stack frame
	push rbp
	mov rbp, rsp

	;volatile
	push rdi
	mov rdi, qword ptr [rbp + 60o]
	;compute result size for zeroing output array
	mov rax, rdx 
	add rax, r9
	
	reset_output_loop_start:
		test rax,rax 
		je reset_output_loop_end
		mov qword ptr [rdi+rax*8], 0
		dec rax
		jmp reset_output_loop_start
	reset_output_loop_end:
	
	left_number_loop_start:


	for (numsize_t j = 0; j < m; j++)// read  left number
	{
		if (A[j] == 0)
			continue; // relatively low-cost optimization for multiply by 0, it should be measured..

		for (numsize_t i = 0; i < n; i++) // read right number
		{
			unsigned k = i + j;
			if (B[i] == 0)
				continue;

			b.dword = B[i];
			b.dword = b.dword * A[j];

			r = b.Pair.H;
			b.dword = (multiply_big)R[k] + b.Pair.L;
			R[k] = b.Pair.L;

			if (k+1 > outBuffSize && R[k] > 0)
				outBuffSize = k+1;

			++k;
			b.dword = (multiply_big)R[k] + r + b.Pair.H;
			R[k] = b.Pair.L;
			r = b.Pair.H;

			while (r)
			{
				b.dword = r + (multiply_big)R[++k];       //sum the previous carry to Result[i+j+k]	
				R[k] = b.Pair.L;				
				r = b.Pair.H; //if there is still a carry move it to low word.
			}
			if (k + 1 > outBuffSize && R[k] > 0)
				outBuffSize = k + 1;
		}
	}

	return outBuffSize;

									
	

	; resume stack frame
	pop rdi
	
	mov rsp, rbp
	pop rbp
ret
LongSumAsm endp
end