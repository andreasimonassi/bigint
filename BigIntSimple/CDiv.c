#include "BigIntSimple.h"
#include <assert.h>
#include <stdlib.h>

#define ENABLE_NORMALIZE

static int fastcmp(reg_t *A, numsize_t ASize, reg_t*B, numsize_t BSize)
{
	numsize_t i = (ASize > BSize ? ASize : BSize);

	while (i > 0)
	{
		i--;
		if (A[i] != B[i])
			break;
	}

	if (i == 0 && A[i] == B[i])
		return 0;

	if (A[i] > B[i])
		return 1;

	return -1;
}

/* reverse array */
void invert(reg_t * A, numsize_t n)
{
	numsize_t l = 0;
	reg_t temp;
	while (l != n)
	{
		n--;
		temp = A[0];
		A[0] = A[n];
		A[n] = temp;
	}
}
/* copy with regard to overlap */
void copy(reg_t *dest, reg_t*source, numsize_t n)
{	
	if (dest > source)
	{
		while (n != 0)
		{
			n--;
			dest[n] = source[n];
		}
	}
	else if(dest < source)
	{
		numsize_t m = 0;
		while (m < n)
		{			
			dest[m] = source[m];
			m++;
		}
	}
}

#ifdef  ENABLE_NORMALIZE
#define SHIFTBITS (sizeof(reg_t) * 8)
numsize_t findmsw(reg_t * A, numsize_t n)
{
	while (n > 0)
	{
		n--;
		if (A[n] != 0)
			return n;		
	}
	return 0;
}

numsize_t shiftl(reg_t * dest, reg_t * source, numsize_t source_l, unsigned n)
{
	/* this can work in place */
	assert(n < SHIFTBITS);
	unsigned  m = SHIFTBITS - n;
	reg_t prev_word = 0;
	reg_t temp;
	numsize_t i = source_l;

	while (i > 0)
	{
		temp = source[i-1];
		dest[i--] = temp >> m | prev_word;
		prev_word = temp << n;
	}

	dest[i] = prev_word;
	return findmsw(dest, source_l + 1) + 1;
}

numsize_t shiftr(reg_t * dest, reg_t * source, numsize_t source_l, unsigned n)
{
	/* this can work in place */
	assert(n < SHIFTBITS);
	reg_t  m = SHIFTBITS - n;
	reg_t prev_word = 0;
	reg_t temp;

	while (source_l > 0)
	{
		temp = source[--source_l];
		dest[source_l] = temp >> n | prev_word;
		prev_word = temp << m;
	}
	return findmsw(dest, source_l )+1;
}
#endif



/*
return non zero means failure

Note:
Q and R must be large enough to contain result

A and B are allocated such that A[m] and B[n] are indexable... (it means it must have space to accomodate m+1 and n+1 digits)

this allows me not to allocate 2 times A or B because i need 1 more word for normalization

the interface is quite raw you will need to encapsulate this if you want to create an actual API
*/
_div_result_t LongDivision(reg_t *A, numsize_t m, reg_t *B, numsize_t n, reg_t * Q, numsize_t * q, reg_t * R, numsize_t * r)
{
	
	if (n == 0)
		return DIV_BY_ZERO; /*failure*/

	int compare;
#ifdef  ENABLE_NORMALIZE
	int shift=0;
#endif	
	reg_t Aguess[2];
	reg_t Qn;
	reg_t * test;	
#ifdef DEBUG
	reg_t *testpin;
#endif
	reg_t bleftmost;

	/*get rid of leading zeroes*/
	m = findmsw(A, m)+1;
	n = findmsw(B, n)+1;

	if (n == 0) 
		return DIV_BY_ZERO; /* failure divide by zero */

	if (m == 0) /* division of type 0/x */
	{		
		*q = 0;
		*r = 0;
		return DIV_BY_ZERO;
	}	

	compare = fastcmp(A, m, B, n); /* check if A <= B*/
	if (compare < 0) /* A < B */
	{
		*q = 0; /* set length of Q equals to zero so Q is equals to 0 */
		*r = n;
		copy(R, B, n);
		return DIV_BY_ZERO; /* division OK */
	}
	else if (compare == 0)
	{
		/* A == B so return Q=1 and remainder is 0 */
		*q = 1; 
		*r = 0;
		*Q = 1;
		return DIV_BY_ZERO; /* division OK */
	}

	bleftmost = B[n - 1];
	

#ifdef  ENABLE_NORMALIZE
	/* now normalize the division, leftmost digit of B must have its leftmost bit to 1 */
	while (bleftmost < LEFTBIT)
	{
		bleftmost = bleftmost << 1;
		shift++;
	}
	
	/* here I need A and B to be large enough to contain one more word*/
	m = shiftl(A, A, m, shift);
	n = shiftl(B, B, n, shift);

	 test = (reg_t*) malloc((n + 1) * sizeof(reg_t)); 
#ifdef DEBUG
	 testpin = /* using testpin for debug reason, test is never edited but at the end the free(test) gives error... so using testpin to check if test has been changed. */
#endif


	if (!test)
	{
		return	FAILURE;
	}

#endif 
	
	/* initialize result Q=0 (is zero when its size q is 0 or only contains zeroes) */
	*q = 0;

	/* we should now copy as many digits from A into Remainder (R) */

	numsize_t a_offset = m -  n; /* index of A such that A[a_offset] ... A[m] has same number of digits as B */	
	
	if (fastcmp(&A[a_offset], n, B, n) < 0) /*An is still less than B*/
	{
		a_offset--; /* pick one more digit on A*/
	}

	/* initialize first remainder (remainder equals An) */
	*r = m - a_offset ; 

	/* copy digits from A[offset]...A[m] to first remainder */
	copy(R, &A[a_offset], *r);

	do
	{
		/* now guessing next divisor */
		
		reg_t dummy;
		numsize_t testLen;

		Qn = 0; /* Qn = 0*/
		testLen = 0; /* Qn*B=0 (I set length of Qn*B to 0 so that Qn*B=0) */

		/* if An < B then Qn is zero, else find Qn */
		compare = fastcmp(R, *r, B, n);
		if (compare >= 0)
		{
			/* An = select as many digits from A such that Aguess >= Bn*/
			/* we may have 2 cases leftmost of A >= B so we divide 1 number*/
			
			if (R[*r-1] < bleftmost)
			{
				Aguess[1] = R[*r-1];
				Aguess[0] = R[*r-2];
			}
			else
			{
				Aguess[1] = 0;
				Aguess[0] = R[*r-1];
			}
			/* Guess Qn */
			Qn = cpu_divide(Aguess[0], Aguess[1], bleftmost, &dummy);

			/* now verify if Qn is good guess multiplying it by B*/
			testLen = LongMultiplication(&Qn, 1, B, n, test);

			/* verify if Test > An */
			compare = fastcmp(test, testLen, R, *r);
			/* if An > Test then Qn is too large */
			while (compare > 1)
			{
				/* 
				since Qn is too large we decrease it by 1 until we have
				that Qn is the correct one 
				*/

				Qn--;

				/* the longsub operation is required to allow me to work in place 
				i mean... write the result to the first argument... so i don't
				have to allocate more space to store the subtraction result...
				that is generally not required... so check that the implementation
				of LongSub you use supports that case...
				*/
				testLen = LongSub(test, testLen, B, n, test); 

				/* ok compare again to see if now we have found right Qn...
				remember that if division is normalized we can have a maximum of 2 
				iteration for this loop.
				*/
				compare = fastcmp(test, testLen, R, *r);
			}
		}

		/* ok now Qn contain the correct Qn or eventually 0*/
		/* we have to store Qn to the higher index of Q but we don't
		know how long Q is going to be, so we store this in reverse order
		, before to return we will reverse the array, on next implementation iteration
		we will compute the destination index before to start;
		*/
		Q[*q] = Qn;
		(*q)++;

		/* now we compute remainder, as usual we do subtraction in place */
		if(testLen > 0)
			*r = LongSub(R, *r, test, testLen, R);

		/* let's see if there are more digits on A */
		
		if (a_offset == 0)
			break; /* no more digits on A */
		
		/* more digits on A ... shift remainder and append next digit of A */
		copy(&R[1], &R[0], *r); /* shift significative digits left by one position to the left*/
		a_offset--; /* next digit of A */
		*r = *r + 1; /* to add a digit to R increase its count by 1*/
		R[0] = A[a_offset]; /* append next digit of A to rightmost (least significant) of R */


	} while (1);

#ifdef DEBUG
	assert(test == testpin);	
#endif
	free(test);
#ifdef  ENABLE_NORMALIZE
	if (shift != 0)
	{		
		shiftr(R, R, *r, shift);
		shiftr(A, A, m, shift);
		shiftr(B, B, n, shift);
	}
#endif

	/* at this point Q array is reversed.. we need to reverse it before to return */
	invert(Q, *q);
	return 0;
}