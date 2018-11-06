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

typedef uint32_t numsize_t;
typedef uint64_t reg_t;
typedef uint32_t multiply_small;
typedef uint64_t multiply_big;
typedef int _result_t;

/* please ensure sizeof(_multiply_t) == sizeof(_reg_t) || sizeof(_multiply_t) == 2 * sizeof(reg_t) */
typedef union
{
	multiply_big dword;
	struct
	{
		multiply_small L; //little endian
		multiply_small H;
	} Pair;
} multiply_t;




/*
Will not check array bounds on R, thus must have space to accomodate MAX(ASIZEInBytes,BSIZEInBytes)+1 bytes
*/

EXTERN numsize_t LongSumAsm(reg_t * A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);
EXTERN numsize_t LongSubAsm(reg_t * A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);
EXTERN numsize_t LongSubAsmVariant_1(reg_t * A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);
EXTERN numsize_t BitScanReverse(reg_t A);


/*
Caller Must check
	* R, thus must have space to accomodate MAX(ASIZE,BSIZE)+1 digits

	return value position of most significant digit on result (one of MAX(ASIZE,BSIZE)+1 or MAX(ASIZE,BSIZE))

*/
numsize_t LongSumWithCarryDetection(reg_t* A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);


/*
Caller must check:
* R should have MAX(ASize, BSize) space, will not check bounds
* ASize must be > BSize  or ASize can be == BSize if A > B
* Most significant digit of A is not zero

Return value is number of significant digits in result

*/
numsize_t LongSub(reg_t* A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);

/*R must be preallocated it should be able to contain up to m*n digits, return value may have 
leading zeroes*/
numsize_t LongMultiplication(reg_t* A, numsize_t m, reg_t * B, numsize_t n, reg_t* R);

/*
	Compare number A and B returns -1 if A < B 0 if A == B and 1 if A > B
*/
int CompareWithPossibleLeadingZeroes(reg_t * A, numsize_t ASize, reg_t * B, numsize_t BSize);

_result_t parseFromHex(wchar_t const * const nullTerminatedString, reg_t * outBuffer, numsize_t bufferSizeInWords, numsize_t * outNumberSizeInWords);

int parseFromByteArrayLittleEndian(unsigned char * input, numsize_t sizeOfInput, reg_t * const outBuffer, numsize_t bufferSizeInWords, numsize_t * outNumberSizeInWords);

reg_t* AllocNumOrFail(numsize_t size);

_result_t FillHexString(wchar_t*buffer, unsigned bufferSize, reg_t* number, numsize_t size);