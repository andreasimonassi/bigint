#pragma once
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif
#include <stdio.h>
#include <stdint.h>

/* define _R such that appends L to int literal if necessary to keep portability, 
but should not be necessary since compiler automatically promote int to longs  */
#define _R(a) ((reg_t)(a ## L))


typedef unsigned long long reg_t;

/*
Will not check array bounds on R, thus must have space to accomodate MAX(ASIZEInBytes,BSIZEInBytes)+1 bytes
*/

EXTERN reg_t LongSumAsm(reg_t * A, reg_t ASize, reg_t * B, reg_t BSize, reg_t* R);
EXTERN reg_t BitScanReverse(reg_t A);


/*
Caller Must check
	* R, thus must have space to accomodate MAX(ASIZE,BSIZE)+1 digits

	return value position of most significant digit on result (one of MAX(ASIZE,BSIZE)+1 or MAX(ASIZE,BSIZE))

*/
reg_t LongSumWithCarryDetection(reg_t* A, reg_t ASize, reg_t * B, reg_t BSize, reg_t* R);


/*
Caller must check:
* R should have MAX(ASize, BSize) space, will not check bounds
* ASize must be > BSize  or ASize can be == BSize if A > B
* Most significant digit of A is not zero

Return value is number of significant digits in result

*/
reg_t LongSub(reg_t* A, reg_t ASize, reg_t * B, reg_t BSize, reg_t* R);

/*
	Compare number A and B returns -1 if A < B 0 if A == B and 1 if A > B
*/
int CompareWithPossibleLeadingZeroes(reg_t * A, reg_t ASize, reg_t * B, reg_t BSize);

int parseFromHex(wchar_t const * const nullTerminatedString, reg_t * const outBuffer, reg_t bufferSizeInWords, reg_t * outNumberSizeInWords);

int parseFromByteArrayLittleEndian(unsigned char * input, reg_t sizeOfInput, reg_t * const outBuffer, reg_t bufferSizeInWords, reg_t * outNumberSizeInWords);
