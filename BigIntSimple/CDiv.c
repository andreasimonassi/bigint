#include "BigIntSimple.h"
#include <assert.h>
#include <stdlib.h>

#define ENABLE_NORMALIZE

#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
long long cpu_mul_count;
long long cpu_div_count;
long long sub_count;
long long compare_count;

void startReadableDivisionDebug() {
	compare_count = sub_count = cpu_div_count = cpu_mul_count = 0;
}
void printReadableDivisionDebug()
{
	fprintf(stderr, "Total multiplications %lld\n" , cpu_mul_count);
	fprintf(stderr, "Total divisions %lld\n", cpu_div_count);
	fprintf(stderr, "Total subtractions %lld\n", cpu_div_count);
	fprintf(stderr, "Total comparisons %lld\n", compare_count);
}
#endif

/* caution will not check against div by zero */
EXTERN reg_t cpu_divide(reg_t LoWord, reg_t HiWord, reg_t Divisor, reg_t* R);
EXTERN reg_t cpu_multiply(reg_t A, reg_t B, reg_t* high);

static int fastcmp(reg_t* A, numsize_t ASize, reg_t* B, numsize_t BSize)
{
#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
	compare_count++;
#endif 
	numsize_t cmp = ASize - BSize;
	if (cmp != 0)
		return cmp;
	numsize_t i = ASize;

	while (i > 0)
	{
#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
		compare_count++;
#endif 
		i--;
		if (A[i] != B[i])
			break;
	}

	if (i == 0 && A[i] == B[i])
		return 0;

	/* if we were sure that A[i] - B[i] does not overflow an int we could do

	return A[i] - B[i];

	but reg_t could be larger than int so don't do that!

	use branches instead
	*/

	if (A[i] > B[i])
		return 1;

	return -1;
}

/* reverse array */
static void invert(reg_t* A, numsize_t n)
{
	numsize_t l = 0;
	reg_t temp;
	while (l < n)
	{
		n--;
		temp = A[l];
		A[l] = A[n];
		A[n] = temp;
		l++;
	}
}
/* copy with regard to overlap */
static void copy(reg_t* dest, reg_t* source, numsize_t n)
{
	if (dest > source)
	{
		while (n != 0)
		{
			n--;
			dest[n] = source[n];
		}
	}
	else if (dest < source)
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
static numsize_t findmsw(reg_t* A, numsize_t n)
{
	while (n > 0)
	{
		n--;
		if (A[n] != 0)
			return n;
	}
	return (numsize_t)-1;
}

static numsize_t shiftl(reg_t* dest, reg_t* source, numsize_t source_l, unsigned n)
{
	/* this can work in place */
	if (n == 0)
		return source_l;
	assert(n < SHIFTBITS);
	unsigned  m = SHIFTBITS - n;
	reg_t prev_word = 0;
	reg_t temp;
	numsize_t i = source_l;

	while (i > 0)
	{
		temp = source[i - 1];
		dest[i--] = (temp >> m) | prev_word;
		prev_word = temp << n;
	}

	dest[i] = prev_word;
	return findmsw(dest, source_l + 1) + 1;
}

static  numsize_t shiftr(reg_t* dest, reg_t* source, numsize_t source_l, unsigned n)
{
	/* this can work in place */
	if (n == 0)
		return source_l;
	assert(n < SHIFTBITS);
	reg_t  m = SHIFTBITS - n;
	reg_t prev_word = 0;
	reg_t temp;

	while (source_l > 0)
	{
		temp = source[--source_l];
		dest[source_l] = (temp >> n) | prev_word;
		prev_word = temp << m;
	}
	return findmsw(dest, source_l) + 1;
}
#endif

/*
Function _div_result_t LongDivision:
------------------------------------

return non zero means failure

Note:
Q and R must be large enough to contain result

A and B are allocated such that A[m] and B[n] are indexable... (it means it must have space to accomodate m+1 and n+1 digits)

this allows me not to allocate a copy for A or B because i potentially need 1 more word for normalization

the interface is quite raw you will need to encapsulate this if you want to create an actual API

as a corollary of the previous statements we does not make A and B constant because we need to modify them but after return they will be the same as they were passed in

*/
_div_result_t LongDivision(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* Q, numsize_t* q, reg_t* R, numsize_t* r)
{
	if (n == 0)
		return DIV_BY_ZERO; /*failure*/

	int compare;
#ifdef  ENABLE_NORMALIZE
	int shift = 0; /* keep track of how many shifts we do to have the leftmost bit of divisor (B) be 1*/
#endif
	reg_t Aguess[2]; /* the one or 2 leftmost digits of next divisor that we use to guess Qn */
	reg_t Qn;        /* the Qn, at first it will be a guess and we will reduce it by 1 until we find the right one */
	reg_t* test;	 /* this is used to do test = B * Qn to check whether or not Qn is the correct divisor */
#ifdef _DEBUG
	reg_t* testpin; /* for debugging we check that the above test pointer has not been altered by keeping track of its original value */
	int debug_count; /* will use this to count how many times we do the guessing correction loop. we will assert that the loop is executed maximum 2 times*/
#endif
	reg_t bleftmost; /* leftmost digit of B */

	/*get rid of leading zeroes*/
	m = findmsw(A, m) + 1;
	n = findmsw(B, n) + 1;

	/* handling corner case, divide by zero*/
	if (n == 0)
		return DIV_BY_ZERO; /* failure divide by zero */

	/* corner case 0 divided by x */
	if (m == 0)
	{
		/* zero divided by anything returns 0 with a remainder of 0
		because by definition

		A = qB + R

		if A = 0 only solution is to have both q and R equals 0 (when A, B, q, R are unsigned)

		*/
		*q = 0;
		*r = 0;
		return OK;
	}

	/* corner case, A is less than B */
	compare = fastcmp(A, m, B, n); /* check if A <= B*/
	if (compare < 0) /* A < B */
	{
		*q = 0; /* set length of Q equals to zero so Q is equals to 0 */
		*r = m;
		copy(R, A, m);
		return OK; /* division OK */
	}
	/* corner case, A == B */
	if (compare == 0)
	{
		/* A == B so return Q=1 and remainder is 0 */
		*q = 1;
		*r = 0;
		*Q = 1;
		return OK; /* division OK */
	}

	bleftmost = B[n - 1]; /*leftmost digit of B*/

#ifdef  ENABLE_NORMALIZE
	/* now normalize the division, leftmost digit of B must have its leftmost bit to 1 */
	while (bleftmost < LEFTBIT)
	{
		/*
		that loop have a worst case of 63 iterations on x64 machines, but if
		we consider an uniform distribution of the input numbers, the probability
		of doing more than 7 shifts is ~ 0.78%
		MSB|Events count|SHIFTS |Prob Cum
		63 |9,22E+18    |0      |100.00% <-probability of having 0 or more shifts to do
		62 |4,61E+18    |1      | 50.00% <-probability of having 1 or more shifs to do
		61 |2,31E+18    |2      | 25.00%
		60 |1,15E+18    |3      | 12.50%
		59 |5,76E+17    |4      |  6.25%
		58 |2,88E+17    |5      |  3.13%
		57 |1,44E+17    |6      |  1.56%
		56 |7,21E+16    |7      |  0.78%
		..
		1  |       2    |63     |  (1 out of 2^64)
		0  | that case does not exist because we removed the zero digits at left

		we could use an hardware instruction BSR to find the number of shift though on
		next implementation

		*/
		bleftmost = bleftmost << 1;
		shift++; /*count how many shifts, on x64 processor maximum shifts are 63,
				 50% of times there will be no shifts, because there are as many
				 numbers having left bit to 1 and having left bit to 0*/
	}

	/* here is where I need A and B to be large enough to contain one more word*/
	m = shiftl(A, A, m, shift);
	n = shiftl(B, B, n, shift);
	bleftmost = B[n - 1];

	test = (reg_t*)malloc((((size_t)n + 1) * sizeof(reg_t)));

#ifdef _DEBUG
	testpin = test;/* using testpin for debug reason, test is never edited but at the end the free(test) could crash the program if we changed it ... so using testpin to check if test has been changed. */
#endif

	if (!test)
	{
		/* if you want to be more precise you could return something like NO_MEMORY*/
		return	GENERIC_FAILURE;
	}

#endif

	/* initialize result Q=0 (is zero when its size q is 0 or only contains zeroes) */
	* q = 0;

	/* we should now copy as many digits from A into Remainder (R) */

	numsize_t a_offset = m - n; /* index of A such that A[a_offset] ... A[m] has same number of digits as B */

	if (fastcmp(&A[a_offset], n, B, n) < 0) /*An is still less than B*/
	{
		a_offset--; /* pick one more digit on A*/
	}

	/* initialize first remainder (remainder equals An) */
	*r = m - a_offset;

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
		if (compare >= 0) /* in this case An >= B so Qn have to be >= 1*/
		{
			/* An = select as many digits from A such that Aguess >= Bn*/
			/* we may have 2 cases leftmost of A >= B so we divide 1 number*/

			if (R[*r - 1] > bleftmost)
			{				
				Qn = 1; 
			}
			else if (R[*r - 1] == bleftmost)
			{
				/*
				could be something like 599 : 598 or 599 : 599
				or something like 5980 : 599
				cannot be like 5990 : 599
				*/
				if (*r > n) /* means An lenght > B length*/
					Qn = _R(-1);
				else
					Qn = _R(1);
			}
			else if (R[*r - 1] < bleftmost)
			{
				Aguess[1] = R[*r - 1];
				Aguess[0] = R[*r - 2];
#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
				cpu_div_count++;
#endif
				Qn = cpu_divide(Aguess[0], Aguess[1], bleftmost, &dummy);
			}

			/* now verify if Qn is good guess multiplying it by B*/
			testLen = LongMultiplication(&Qn, 1, B, n, test);			

			/* verify if Test > An */
			compare = fastcmp(test, testLen, R, *r);
			/* if An > Test then Qn is too large */
#ifdef _DEBUG
			debug_count = 0;
#endif
			while (compare > 0)
			{
				/*
				since Qn is too large we decrease it by 1 until we have
				that Qn is the correct one
				*/

				Qn--;

				/*
				the longsub operation is required to allow me to work in place
				i mean... write the result to the first argument... so i don't
				have to allocate more space to store the subtraction result...
				that is generally not required for most LongSub implements...
				...so check that the implementation of the LongSub we
				use here supports that case...
				*/

				testLen = LongSub(test, testLen, B, n, test);

				/* ok compare again to see if now we have found right Qn...
				remember that if division is normalized we can have a maximum of 2
				iteration for this loop.
				*/
				compare = fastcmp(test, testLen, R, *r);
#ifdef _DEBUG

				/* our assumption is that this loop must execute maximum 2 times
				   ... let's assert this and find if it's true at debug time
				*/
				debug_count++;
				assert(debug_count < 3);
#endif
			}
		}
		/*
		else : Qn is 0 because An < B
		*/

		/* ok now Qn contain the correct Qn or eventually 0*/
		/* we have to store Qn to the higher index of Q but we don't
		know how long Q is going to be, so we store this in reverse order
		, before to return we will reverse the array, on next implementation
		we will compute the destination index before to start, but that is marginal improvement,
		the outer loop dominates;
		*/
		Q[*q] = Qn;
		(*q)++;

		/* now we compute remainder, as usual we do subtraction in place */
		if (testLen > 0)
			* r = LongSub(R, *r, test, testLen, R);

		/* let's see if there are more digits on A */

		if (a_offset == 0)
			break; /* no more digits on A */

		/* more digits on A ... shift remainder and append next digit of A */
		copy(&R[1], &R[0], *r); /* shift significative digits left by one position to the left*/
		a_offset--; /* next digit of A */
		*r = *r + 1; /* to add a digit to R increase its count by 1, only if its left digt is not going to be zero*/	
		R[0] = A[a_offset]; /* append next digit of A to rightmost (least significant) of R */
		while (*r > 0 && R[*r-1] == 0) /* trim leading zeroes */
			*r = *r - 1;
	} while (1);

#ifdef _DEBUG
	/*
	we assert that test has not been changed by the division code otherwise the free will fail
	*/
	assert(test == testpin);
#endif
	free(test);
#ifdef  ENABLE_NORMALIZE
	if (shift != 0)
	{
		/*
		restore the remainder,
		A and B were modified in place so we must restore them before return
		since the caller does not expect that we modified the input arrays...
		*/
		shiftr(R, R, *r, shift);
		shiftr(A, A, m, shift);
		shiftr(B, B, n, shift);
	}
#endif

	/*
	at this point Q array is reversed.. we need to reverse it before to return
	*/
	invert(Q, *q);
	return OK;
}

#define GetLeftmostOf(X, XSize) ((X)[(XSize)-1])

static reg_t RunHwDivision(reg_t ALeftMost, reg_t ASecond, reg_t BLeftmost)
{	
	
#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
		cpu_div_count++;
#endif
		return cpu_divide(ASecond, ALeftMost, BLeftmost, &BLeftmost);
	
}


static numsize_t simpleMultiplication(reg_t* A, numsize_t ASize, reg_t B, reg_t* R)
{
	reg_t low;
	reg_t high = 0;

	R[0] = 0;
	for (numsize_t i = 0; i < ASize; ++i)
	{
		low = cpu_multiply(A[i], B, &high);

#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
		cpu_mul_count++;
#endif

		R[i] += low;

		if (R[i] < low) //carry detection
			high++; //have carry

		R[i + 1] = high;
	}
	if (R[ASize] != 0)
		return ASize + 1;
	return ASize;
}


static void LongDivisionReadableCore(reg_t* temp, reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* Q, numsize_t q, reg_t* R, numsize_t* r)
{
	/**r = 0;
	return;*/

	reg_t  bleftmost = B[n - 1]; /* leftmost digit of B */
#ifdef _DEBUG
	int debug_count; /* will use this to count how many times we do the guessing correction loop. we will assert that the loop is executed maximum 2 times*/
#endif

	/* An = Select as many digits from A such that An >= B; */
	reg_t* An = temp + n + 1; /* using preallocated memory to keep copy of A*/
	numsize_t AnSize = n;

	copy(An, A, m); /* copy A into An*/


	if (fastcmp(An+m-AnSize, AnSize,B, n) <0)
	{
		AnSize++;
	}

	An += m - AnSize;
	while (q > 0)
	{
		reg_t Qn;
		numsize_t tempSize;

		if (An[AnSize - 1] == bleftmost)
		{		
			if (AnSize < n)
			{
				Qn = 0;
				tempSize = 0;
			}
			else if (AnSize == n)
			{
				Qn = 1;
				copy(temp, B, n);
				tempSize = n;
			}
			else //AnSize bigger than n, no other case exist			
			{
				Qn = _R(-1);
				copy(temp+1, B, n);
				temp[0] = 0;
				tempSize = n +1;
				LongSub(temp, tempSize, B, n, temp);	
				
			}
		}
		else if (AnSize == 1 )
		{
			if (An[AnSize - 1] < bleftmost)
			{
				Qn = 0;
				tempSize = 0;
			}
			else
			{
				Qn = 1;
				copy(temp, B, n);
				tempSize = n;
			}
		}
		else
		{
			if (An[AnSize - 1] > bleftmost)

			{
				Qn = 1;
				copy(temp, B, n);
				tempSize = n;
			}
			else
			{
				Qn = RunHwDivision(An[AnSize - 1], An[AnSize - 2], bleftmost);
				tempSize = simpleMultiplication(B, n, Qn, temp); /* temp = Qn * B   */
			}
		}
		
#ifdef _DEBUG
		debug_count = 0;
#endif
		while (fastcmp(temp, tempSize, An, AnSize) > 0)/* while (Temp > An),  maximum 2 iterations here*/
		{
			Qn--;
			tempSize = LongSub(temp, tempSize, B, n, temp);/*Temp -= B;*/
#ifdef _DEBUG

	/* our assumption is that this loop must execute maximum 2 times
	   ... let's assert this and find if it's true at debug time
	*/
			debug_count++;
			assert(debug_count < 3);
#endif
		}

		AnSize = LongSub(An, AnSize, temp, tempSize, An); /*Remainder = An = An - Test; */
		Q[--q] = Qn; /* Append Qn to Q; */

		if (q > 0)
		{
			An--;
			AnSize++;
		}
	}
	copy(R, An, AnSize);
	*r = AnSize;
}

static unsigned int NormalizeShiftLeft(reg_t* A, numsize_t* m, reg_t* B, numsize_t* n)
{
	unsigned int shift = 0;
	shift = 8 * sizeof(reg_t) - BitScanReverse(B[(*n) - 1]) - 1;

	/* here is where I need A and B to be large enough to contain one more word*/
	*m = shiftl(A, A, *m, shift);
	*n = shiftl(B, B, *n, shift);
	return shift;
}

static void NormalizeShiftRight(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* R, numsize_t* r, int shift)
{
	shiftr(R, R, *r, shift);
	shiftr(A, A, m, shift);
	shiftr(B, B, n, shift);
}

static int RemoveLeadingZeroes(reg_t* A, numsize_t* _m, reg_t* B, numsize_t* _n)
{
	while (*_m > 0 && A[(*_m) - 1] == 0)
		(*_m)--;
	while (*_n > 0 && B[(*_n) - 1] == 0)
		(*_n)--;

	if ((*_n) == 0)
		return DIV_BY_ZERO;
	return OK;
}



_div_result_t LongDivisionReadable(reg_t* A, numsize_t m, reg_t* B, numsize_t n, reg_t* Q, numsize_t* q, reg_t* R, numsize_t* r)
{
	/* This function will process the input such that the LongDivisionReadableCore will work on
	  * good numbers (A > B)
	  * without the need for input checking
	  * without to allocate temporary memory
	  * without to compute the lenght of the result
	*/

	if (RemoveLeadingZeroes(A, &m, B, &n) == DIV_BY_ZERO)
		return DIV_BY_ZERO;
	int compare;
	reg_t* tempSpace;

	if (m == 0) /* corner case 0 / X */
	{
		*q = 0;
		*r = 0;
		return OK;
	}
	/* corner case, A is less than B */

	compare = fastcmp(A, m, B, n); /* check if A <= B*/

	if (compare < 0) /* A < B */
	{
		*q = 0; /* set length of Q equals to zero so Q is equals to 0 */
		*r = m;
		copy(R, A, m);
		return OK; /* division OK */
	}

	/* corner case, A == B */
	if (compare == 0)
	{
		/* A == B so return Q=1 and remainder is 0 */
		*q = 1;
		*r = 0;
		*Q = 1;
		return OK; /* division OK */
	}

	int shift = NormalizeShiftLeft(A, &m, B, &n);

	tempSpace = (reg_t*)malloc((((size_t)n + m + 2)) * sizeof(reg_t));

	if (!tempSpace)
	{
		/* if you want to be more precise you could return something like NO_MEMORY*/
		return	GENERIC_FAILURE;
	}

	/* computing lenght of Q */
	*q = m - n;

	if (fastcmp(A + m - n, n, B, n) >= 0)
		(*q)++;
	
	/* end computing length of Q */

	LongDivisionReadableCore(tempSpace, A, m, B, n, Q, *q, R, r);

	free(tempSpace);

	NormalizeShiftRight(A, m, B, n, R, r, shift);
	return OK;
}