.code
cpu_multiply proc
	mov rax, rdx	
	mul rcx
	mov qword ptr[r8], rdx
	ret
cpu_multiply endp
end