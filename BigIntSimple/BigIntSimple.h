#pragma once
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/*#define _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA*/ /* counting number of multiplication, divisions, this code have to be removed i just use it once to better undestand inner working of divisions*/
#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
extern long long cpu_mul_count;
extern long long cpu_div_count;
extern long long sub_count;
extern long long compare_count;
#endif


/* define _R such that appends L to int literal if necessary to keep portability, 
but should not be necessary since compiler automatically promote int to longs  */
#define _R(a) ((reg_t)(a ## L))
typedef uint32_t numsize_t;
typedef uint64_t reg_t;
typedef int _result_t;

/* define that if the target compiler does not have a dword unsigned integer type */
#define NO_DWORD_INTS
typedef uint32_t multiply_small;
typedef uint64_t multiply_big;


typedef enum _div_result {
	OK = 0, DIV_BY_ZERO = 1, GENERIC_FAILURE
} _div_result_t;

#define LEFTBIT  (1ULL << (sizeof(reg_t)*8ULL-1ULL))

/* please ensure sizeof(_multiply_t) == sizeof(_reg_t) || sizeof(_multiply_t) == 2 * sizeof(reg_t) */
typedef union
{
	multiply_big dword;
	struct
	{
		multiply_small L; /*little endian*/
		multiply_small H;
	} Pair;
} multiply_t;




/*
Will not check array bounds on R, thus must have space to accomodate MAX(ASIZEInBytes,BSIZEInBytes)+1 bytes
*/

EXTERN numsize_t LongSumAsm(reg_t * A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);
EXTERN numsize_t LongSubAsm(reg_t * A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);
EXTERN numsize_t LongSubAsmVariant_1(reg_t * A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);
EXTERN numsize_t LongMulAsm(reg_t * A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);
EXTERN numsize_t LongMulAsmVariant_1(reg_t * A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);
EXTERN reg_t ShortMultiplication(reg_t A, reg_t B, reg_t* out_hiword);
EXTERN int BitScanReverse(reg_t A);

EXTERN reg_t CastingOutNines_asm(numsize_t a, reg_t* A);
reg_t CastingOutNines(reg_t* A, numsize_t a);

/* HERE I AM SETTING UP THINGS TO USE THE ASSEMBLER CastingOutNines_asm routine instead of the C version CastingOutNines
   if you need to disable the assembler version used on various tests you have to change following macro..

   to sligthly optimize asm version i rotated the parameters order...


*/

#define CastModuloRegSize(num, size) CastingOutNines_asm(size, num)

reg_t CastingOutElevens(reg_t* A, numsize_t a, int* out_carry);



/*
Caller Must check
	* R, thus must have space to accomodate MAX(ASIZE,BSIZE)+1 digits

	return value position of most significant digit on result (one of MAX(ASIZE,BSIZE)+1 or MAX(ASIZE,BSIZE))

*/
numsize_t LongSumWithCarryDetection(reg_t* A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);
numsize_t LongSumWithCarryDetectionV2(reg_t* A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);
numsize_t LongSumWithCarryDetectionV3(reg_t* A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R);

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
numsize_t LongMultiplication                             (reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R);
numsize_t LongMultiplicationNoAssembly                   (reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R);
numsize_t LongMultiplicationV2                           (reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R);
numsize_t LongMultiplicationNoAssemblyV2                 (reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R);
numsize_t KaratsubaMultiplicationUsingPortablePrimitive1 (reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R);
numsize_t KaratsubaMultiplicationUsingPortablePrimitive8 (reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R);
numsize_t KaratsubaMultiplicationUsingPortablePrimitive12(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R);
numsize_t KaratsubaMultiplicationUsingPortablePrimitive16(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R);
numsize_t KaratsubaMultiplicationUsingPortablePrimitive24(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R);
numsize_t KaratsubaMultiplicationUsingPortablePrimitive32(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R);
numsize_t KaratsubaMultiplicationUsingAssembly12(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R);

/* A must have m+1 space to support normalization, Q and R must have enough space to hold the result*/
_div_result_t LongDivision(reg_t *A, numsize_t m, reg_t *B, numsize_t n, reg_t * Q, numsize_t * q, reg_t * R, numsize_t * r);
_div_result_t LongDivisionReadable(reg_t * A, numsize_t m, reg_t * B, numsize_t n, reg_t * Q, numsize_t * q, reg_t * R, numsize_t * r);

/*
	Compare number A and B returns -1 if A < B 0 if A == B and 1 if A > B
*/
int CompareWithPossibleLeadingZeroes(reg_t * A, numsize_t ASize, reg_t * B, numsize_t BSize);

_result_t parseFromHex(wchar_t const * const nullTerminatedString, reg_t * outBuffer, numsize_t bufferSizeInWords, numsize_t * outNumberSizeInWords);

int parseFromByteArrayLittleEndian(unsigned char * input, numsize_t sizeOfInput, reg_t * const outBuffer, numsize_t bufferSizeInWords, numsize_t * outNumberSizeInWords);

reg_t* AllocNumOrFail(numsize_t size);

_result_t FillHexString(wchar_t*buffer, unsigned bufferSize, reg_t* number, numsize_t size);

reg_t CastingOutNines(reg_t* A, numsize_t a);
reg_t CastingOutElevens(reg_t* A, numsize_t a, int * out_carry);