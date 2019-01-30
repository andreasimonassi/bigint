#include "BigIntSimple.h"
#include <assert.h>

static numsize_t LongMultiplication_A(reg_t* A, numsize_t m, reg_t * B, numsize_t n, reg_t* R);
static numsize_t LongMultiplication_B(multiply_small* A, numsize_t m, multiply_small * B, numsize_t n, multiply_small* R);

numsize_t LongMultiplication(reg_t *A, numsize_t m, reg_t *B, numsize_t n, reg_t * R)
{
	assert(sizeof(multiply_big) == 2 * sizeof(multiply_small));
	assert(sizeof(multiply_t) == sizeof(reg_t) || sizeof(multiply_t) == 2 * sizeof(reg_t));
	/*
	keep the below assertion until impement the case for which m size*sizof(reg_t) or n size
	* sizeof(reg_t) are not multiple of sizeof(multiply_t)
	a
	*/
	assert(sizeof(multiply_t) == sizeof(reg_t));

	/*the below line generates a warning, saying that it is a constant expression
	... well i know it is constant but i do not know how to do this using the preprocessor
	... so I hope in compiler inlining optimisation
	*/
#pragma warning(disable:4127)  
	if (sizeof(multiply_big) == 2* sizeof(reg_t))
#pragma warning(default:4127)
	{
		return LongMultiplication_A(A, m, B, n, R);
	}
#pragma warning(disable:4127)  
	else if ((sizeof(multiply_big) == sizeof(reg_t)))
#pragma warning(default:4127)
	{
		numsize_t r = LongMultiplication_B((multiply_small*)A, m<<1, (multiply_small*)B, n<<1, (multiply_small*)R);
		if ((r & 1) == 1)
			r++;
		return r >> 1;
	}

	
}

numsize_t LongMultiplication_A(reg_t* A, numsize_t m, reg_t * B, numsize_t n, reg_t* R)
{
	multiply_small r;
	multiply_t b; //number B[n]
	
	numsize_t outBuffSize = m + n;

	for (numsize_t k = 0; k < outBuffSize; ++k)
		R[k] = 0; //reset output array
	outBuffSize = 0;
	for (numsize_t j = 0; j < m; j++)// read  left number
	{
		if (A[j] == 0)
			continue; // relatively low-cost optimization for multiply by 0, it should be measured..

		for (numsize_t i = 0; i < n; i++) // read right number
		{
			int k = i + j;
			if (B[i] == 0)
				continue;

			b.dword = B[i];
			b.dword = b.dword * A[j];

			r = b.Pair.H;
			b.dword = (multiply_big)R[k] + b.Pair.L;
			R[k] = b.Pair.L;

			++k;
			b.dword = (multiply_big)R[k] + r + b.Pair.H;
			R[k] = b.Pair.L;
			r = b.Pair.H;
			while (r)
			{
				b.dword = r + (multiply_big)R[++k];       //sum the previous carry to Result[i+j+k]	
				R[k] = b.Pair.L;
				if (b.Pair.L > 0)
					outBuffSize = k;
				r = b.Pair.H; //if there is still a carry move it to low word.
			}
		}
	}

	return outBuffSize;
}


#error QUA SOTTO LA MOLTIPLICAZIONE NON RESTITUISCE CORRETTAMENTE IL NUMERO DI WORDS NEL RISULTATO
/*
algorithm is exactly the same but work on half sized operators with doubled length (m and n)
*/
numsize_t LongMultiplication_B(multiply_small* A, numsize_t m, multiply_small * B, numsize_t n, multiply_small* R)
{
	multiply_small r;
	multiply_t b; //number B[n]

	numsize_t outBuffSize = m + n;

	for (numsize_t k = 0; k < outBuffSize; ++k)
		R[k] = 0; //reset output array
	outBuffSize = 0;
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
}

