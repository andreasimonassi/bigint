#include "BigIntSimple.h"
#include <assert.h>

#ifndef _WIN32
	#warning "Please define STACK_ALLOCATOR macros to use stack allocation, falling back to use malloc"
	#define STACK_ALLOC_IF_GREATER_THAN 4*1024
	#define STACK_ALLOCATOR(sz) malloc(sz)
	#define STACK_DEALLOCATOR(ptr) free(ptr)
#else
	#include <malloc.h>
	#define STACK_ALLOC_IF_SMALLER_THAN 4*1024
	#define STACK_ALLOCATOR(sz) _malloca(sz)
    #define STACK_DEALLOCATOR(ptr)
#endif



typedef numsize_t(*operation) (reg_t* A, numsize_t ASize, reg_t* B, numsize_t BSize, reg_t* R);

typedef reg_t (*_mul_func)(reg_t A, reg_t B,  reg_t* Hiword) ;
EXTERN reg_t cpu_multiply(reg_t A, reg_t B, reg_t * high);
#ifdef NO_DWORD_INTS
static reg_t c_multiply(reg_t A, reg_t B, reg_t * high)
{
	
	multiply_t a;
	multiply_t b;
	multiply_t c;
	multiply_t d;
	multiply_t e;
	multiply_t f;
	
	/* imagine this 
	
			                                  [A.H]         [A.L] * 
				                              [B.H]         [B.L] =
-------------------------------------------------------------------
	                                     [A.L*B.L HW]  [A.L*B.L LW] + (mult1)
	                       [A.H*B.L HW]  [A.H*B.L LW]               + (mult2)
			               [A.L*B.H HW]  [A.L*B.H LW]               + (mult3)
              [A.H*B.H HW] [A.H*B.H LW]                             + (mult4)
-------------------------------------------------------------------
              [high HW]    [high LW]     [Return HW ]  [Return LW ]

	*/


	a.dword = A;
	b.dword = B;

	c.dword = a.Pair.L;
	d.dword = b.Pair.L;


	/* mult1 */
	c.dword = c.dword * d.dword;
	e.dword = c.dword;

	

	/* mult2*/
	c.dword = a.Pair.H;
	d.dword = b.Pair.L;
	c.dword = c.dword * d.dword;
	e.Pair.H += c.Pair.L;
	f.dword = 0;
	if (e.Pair.H < c.Pair.L)
		f.Pair.L = 1; 
	f.Pair.L += c.Pair.H;

	/* mult 3*/
	c.dword = a.Pair.L;
	d.dword = b.Pair.H;
	c.dword = c.dword * d.dword;
	e.Pair.H += c.Pair.L;
	
	if (e.Pair.H < c.Pair.L)
		f.Pair.L += 1; 
	f.Pair.L += c.Pair.H;
	if (f.Pair.L < c.Pair.H)
		f.Pair.H += 1;


	/* mult 4*/
	c.dword = a.Pair.H;
	d.dword = b.Pair.H;
	c.dword = c.dword * d.dword;
	f.Pair.L += c.Pair.L;
	if (f.Pair.L < c.Pair.L)
		f.Pair.H += 1;
	f.Pair.H += c.Pair.H;

	/* done */
	*high = f.dword;
	return e.dword;

}
#else
static reg_t c_multiply(reg_t A, reg_t B, reg_t * high)
{
	multiply_t a;
	a.dword = *A;
	a.dword *= *B;
	*high = a.pair.H;
	return a.pair.L;
}
#endif

reg_t ShortMultiplication(reg_t A, reg_t B, reg_t* out_hiword)
{
	/* if you remove the assembler code you need to redefine this routing by using any other portable option such as 
	
	return c_multiply(A, B, out_hiword);

	*/
	return cpu_multiply(A, B, out_hiword); 
}

static numsize_t LongMultiplicationPortable(reg_t *A, numsize_t m, reg_t *B, numsize_t n, reg_t * R, _mul_func mfunc)
{
	numsize_t outBuffSize = m + n;
	reg_t hiword;
	reg_t loword;
	int carry;
	for (numsize_t k = 0; k < outBuffSize; ++k)
		R[k] = 0; /*reset output array*/


	for (numsize_t j = 0; j < m; j++)/* read  left number*/
	{
		if (A[j] == 0)
			continue; /* relatively low-cost optimization for multiply by 0, it should be measured... because the
					  probability of a zero digit it is likely to be low in most cases
					  */

		for (numsize_t i = 0; i < n; i++) /* read right number*/
		{
			unsigned k = i + j;

			if (B[i] == 0)
				continue;

#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
			cpu_mul_count++;
#endif 

			loword = mfunc(A[j], B[i], &hiword);

			R[k] += loword;
			carry = R[k] < loword ;

			++k;

			R[k] += hiword + carry;
			carry = (R[k] < hiword) | (carry & (R[k] == hiword));

			while (carry)
			{
				R[++k] += 1;     /*sum the previous carry to Result[i+j+k]	*/
				carry = R[k] == _R(0);
			}			
		}
	}

	/* in the unlikely case most significant digit is zero decrease size count */
	while (outBuffSize > 0 && R[outBuffSize-1] == 0)
		outBuffSize--;

	return outBuffSize;
}


static numsize_t LongMultiplicationPortableV2(reg_t *A, numsize_t m, reg_t *B, numsize_t n, reg_t * R, _mul_func mfunc)
{
	numsize_t outBuffSize = m + n, k;
	reg_t hiword, loword;
	int carry;
	
	for (k = 0; k < outBuffSize; ++k)
		R[k] = 0; /*reset output array*/
	
	for (numsize_t j = 0; j < m; j++)/* read  left number*/			
		for (numsize_t i = 0; i < n; i++) /* read right number*/
		{
			k = i + j;

			loword = mfunc(A[j], B[i], &hiword);

			R[k] += loword;
			carry = R[k] < loword;	/* detect the carry */
			++k; /* next digit */
			R[k] += hiword + carry;
			carry = (R[k] < hiword) | (carry & (R[k] == hiword)); /* ugly carry detection */

			while (carry) /* that is unlikely to loop too much */
			{
				R[++k] += 1;     /*sum the previous carry to Result[i+j+k]	*/
				carry = R[k] == _R(0);
			}
		}	
		
	while (outBuffSize > 0 && R[outBuffSize - 1] == 0)
		outBuffSize--;

	return outBuffSize;
}

/* these are the public interface implementation */
numsize_t LongMultiplication(reg_t *A, numsize_t m, reg_t *B, numsize_t n, reg_t * R)
{
	return LongMultiplicationPortable(A, m, B, n, R, cpu_multiply);
}

numsize_t LongMultiplicationNoAssembly(reg_t *A, numsize_t m, reg_t *B, numsize_t n, reg_t * R)
{
	return LongMultiplicationPortable(A, m, B, n, R, c_multiply);
}

numsize_t LongMultiplicationV2(reg_t *A, numsize_t m, reg_t *B, numsize_t n, reg_t * R)
{
	return LongMultiplicationPortableV2(A, m, B, n, R, cpu_multiply);
}

numsize_t LongMultiplicationNoAssemblyV2(reg_t *A, numsize_t m, reg_t *B, numsize_t n, reg_t * R)
{
	return LongMultiplicationPortableV2(A, m, B, n, R, c_multiply);
}

/*
KaratsubaRecursive: beware:
	te simpleSum and simpleSub are required to allow operation in place (i.e the result could be any of A or B)
	
	please ensure simple
*/

numsize_t KaratsubaRecursive(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R, 
	operation simpleMul,
	operation simpleSum,
	operation simpleSub,
	numsize_t simpleMulThreshold)
{
	
	assert(simpleMulThreshold > 0);

	numsize_t min = m;
	numsize_t max = n;
	if (m > n)
	{
		min = n;
		max = m;
	}

	/* the length of one of the operand is zero the result is zero */
	if (min == 0)
		return 0;

	/* the length of one of the operands is 1 so complexity is going to be N even for simpleMul */
	if (min <= simpleMulThreshold)
		return simpleMul(A, m, B, n, R);

	/*
		we are going to split numbers so that the right group is longer than the left group, for example

		A = 99999
		B = 9999

		longest number is 5 long

		(5+1)/2 = 3 so right group will be long 3

		A=99|999
		B= 9|999
	*/

	numsize_t split_point = (max + 1) >> 1;


	/*
	when one of the 2 numbers is smaller than the minimum group we move the split point so that the split point is equals to the minimum group

	example

	A= 99|999
	B=   | 99

	we move split point such that

	A= 999|99  (note that 4 digits will still accomodate 999+99, see allocation much below)
	B=    |99


	*/



	if (split_point > min)
		split_point = min;

	/* now split A into a|b an B into c|d */
	reg_t* a = A + split_point;
	reg_t* b = A;
	reg_t* c = B + split_point;
	reg_t* d = B;

	numsize_t len_a = m - split_point,		
		len_c = n - split_point;

	/*
		allocate space for later usage, we allocate in adbundance, we'll try to refine later
	*/
#ifdef _DEBUG
	numsize_t alloc_size_a = max - split_point + 2;
	numsize_t alloc_size_b = max - split_point + 2;
	numsize_t alloc_size_z1 = ((max - split_point + 2) << 1);
#endif
	numsize_t allocsize = ((max - split_point + 2)
		+ (max - split_point + 2) +
		((max - split_point + 2) << 1)
		)
		* sizeof(reg_t);
	reg_t* a_plus_b;
	
	if (allocsize >= STACK_ALLOC_IF_SMALLER_THAN)
		a_plus_b = (reg_t*)malloc(
			allocsize
		);
	else
		a_plus_b = (reg_t*)STACK_ALLOCATOR(allocsize);

	reg_t* c_plus_d = a_plus_b + (max - split_point + 2) ;
	reg_t* z1 = c_plus_d + (max - split_point + 2);


	if (a_plus_b == NULL )
	{		
		/* revert to simple mul which will likely not allocate other memory */
		return simpleMul(A, m, B, n, R);
	}

	/* 
		doing a*c, result go to R[split_point*2]

	*/
	numsize_t len_z0 = KaratsubaRecursive(a, len_a, c, len_c, R + (split_point << 1), simpleMul, simpleSum, simpleSub, simpleMulThreshold);
	/*
		doing b*d result go to R[0]
	*/
	numsize_t len_z2 = KaratsubaRecursive(b, split_point, d, split_point, R, simpleMul, simpleSum, simpleSub, simpleMulThreshold);

	/*
		now we have this:

		xxxx = a*c; we stored those digits in R[split_point<<1]
		yyyy = b*d; we store those in R[0]

		where xxxx are digits resulting from a*c , and yyyy digits from b*d

		length of yyyy is split_point * 2
		length of xxxx is less or equals than split_point * 2

		R = [x]xxx|yyyy	
	*/

	/*
		now we will do (a+b)x(c+d), we divide in 2 parts
		
		1) First do the inner sums 
		2) Multiply the result
	*/

	/*
		1) First inner sums	
	*/
#if _DEBUG
	assert(len_a + 1 <= alloc_size_a);
	assert(split_point + 1 <= alloc_size_a);
	assert(len_c + 1 <= alloc_size_b);
	assert(split_point + 1 <= alloc_size_b);
#endif
	numsize_t len_a_plus_b = simpleSum(a, len_a, b, split_point, a_plus_b);
	numsize_t len_c_plus_d = simpleSum(c, len_c, d, split_point, c_plus_d);



	/*
		2) now multiply together
	*/
#if _DEBUG
	assert(len_a_plus_b + len_c_plus_d <= alloc_size_z1);
#endif
	numsize_t z1_len = KaratsubaRecursive(a_plus_b, len_a_plus_b, c_plus_d, len_c_plus_d, z1, simpleMul, simpleSum, simpleSub, simpleMulThreshold);



	/*
		now we will do z1 - yyyy, remember that simpleSub can subtract in place

		we will reuse z1_len...
	*/
	z1_len = simpleSub(z1, z1_len, R, len_z2, z1);

	/*
		now we will do z1 - xxxx, remember that simpleSub can subtract in place

		we will reuse z1_len...
	*/
	z1_len = simpleSub(z1, z1_len, R + (split_point << 1), len_z0, z1);


	/* 
		ok now we have z0, z1, z2
		
		we have to arrange them.

		z0 = most significant, z1 middle, z2 least significant.

		z0 is already in its place in R, and also z2 is already in place.

		in fact, as I show before, R = xx|yy where xx are digits in z0 and yy are digits in z2

		we have to add z1 shifted left of split_point positions...

		so now we'll run simple sum 	

		we will reuse z1_len to save a bit , only concession to optimization on this first implementation

		simpleSum must allow us to sum in place.
	*/	

#if _DEBUG
	/*
		we should not use more than m+n elements of output array R	
	*/
	assert(len_z0 + split_point + split_point  <= m+n);
	assert(z1_len + split_point  <= m + n);
#endif
	/*
		if len_z2 = 0 means we have a zero , which yet occupy 1 digit 
	*/
	z1_len = simpleSum(R + split_point, len_z0 + split_point, z1, z1_len, R+split_point);

	if(allocsize >= STACK_ALLOC_IF_SMALLER_THAN)
		free(a_plus_b);
	else
		STACK_DEALLOCATOR(a_plus_b);

	return z1_len+split_point;
}


/*
KaratsubaRecursive: beware:
	te simpleSum and simpleSub are required to allow operation in place (i.e the result could be any of A or B)

	please ensure simple
*/




numsize_t KaratsubaMultiplicationUsingPortablePrimitive16(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R)
{
	return KaratsubaRecursive(A, m, B, n, R,
		LongMultiplicationV2,
		LongSumWithCarryDetection,
		LongSub, 16
	);
}

numsize_t KaratsubaMultiplicationUsingPortablePrimitive32(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R)
{
	return KaratsubaRecursive(A, m, B, n, R,
		LongMultiplicationV2,
		LongSumWithCarryDetection,
		LongSub, 32
	);
}

numsize_t KaratsubaMultiplicationUsingPortablePrimitive8(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R)
{
	return KaratsubaRecursive(A, m, B, n, R,
		LongMultiplicationV2,
		LongSumWithCarryDetection,
		LongSub, 8
	);
}


numsize_t KaratsubaMultiplicationUsingPortablePrimitive1(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R)
{
	return KaratsubaRecursive(A, m, B, n, R,
		LongMultiplicationV2,
		LongSumWithCarryDetection,
		LongSub, 1
	);
}



numsize_t KaratsubaMultiplicationUsingPortablePrimitive12(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R)
{
	return KaratsubaRecursive(A, m, B, n, R,
		LongMultiplicationV2,
		LongSumWithCarryDetection,
		LongSub, 12
	);
}


numsize_t KaratsubaMultiplicationUsingAssembly12(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R)
{
	return KaratsubaRecursive(A, m, B, n, R,
		LongMulAsmVariant_1,
		LongSumWithCarryDetection,
		LongSub, 12
	);
}
numsize_t KaratsubaMultiplicationUsingPortablePrimitive24(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R)
{
	return KaratsubaRecursive(A, m, B, n, R,
		LongMultiplicationV2,
		LongSumWithCarryDetection,
		LongSub, 24
	);
}


//numsize_t KaratsubaMultiplicationUsingNonPortablePrimitive(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R)
//{
//	return KaratsubaRecursive(A, m, B, n, R,
//		LongMulAsmVariant_1,
//		LongSumAsm,
//		LongSubAsmVariant_1
//	);
//}

