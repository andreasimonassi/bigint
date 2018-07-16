.data

.code

;int BitScanReverse(
	; 	reg_t  A, 		RCX 
BitScanReverse proc
	bsr	rax, rcx
	ret
BitScanReverse endp

end