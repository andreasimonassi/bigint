#include "BigIntSimple.h"
#include <assert.h>

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

static numsize_t LongMultiplicationPortable(reg_t *A, numsize_t m, reg_t *B, numsize_t n, reg_t * R, _mul_func mfunc)
{
	numsize_t outBuffSize = m + n;
	reg_t hiword;
	reg_t loword;
	int carry;
	for (numsize_t k = 0; k < outBuffSize; ++k)
		R[k] = 0; /*reset output array*/

	outBuffSize = 0;

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

			loword = mfunc(A[j], B[i], &hiword);

			R[k] += loword;
			carry = R[k] < loword ? 1 : 0;

			if (k + 1 > outBuffSize && R[k] > 0)
				outBuffSize = k + 1;

			++k;

			R[k] += hiword + carry;
			carry = R[k] < hiword + carry ? _R(1) : _R(0);

			while (carry)
			{
				R[++k] += 1;     /*sum the previous carry to Result[i+j+k]	*/
				carry = R[k] == _R(0) ? _R(1) : _R(0);
			}

			if (k + 1 > outBuffSize && R[k] > 0)
				outBuffSize = k + 1;
		}
	}

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
