#include "BigIntSimple.h"

/*
Caller must check:
* R should have MAX(ASize, BSize) space, will not check bounds
* ASize must be > BSize  or ASize can be == BSize if A > B
* Most significant digit of A is not zero

Return value is number of significant digits in result

*/
numsize_t LongSub(reg_t* A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R)
{

	register numsize_t i;

	int borrow = (int)(i = 0);

	while (BSize > i)
	{
#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
		sub_count++;
#endif 
		/*
		commented code here below was buggy i left the code here for studying purposes on "how not to do borrow detection"
		since it misses a corner case: 11111 - 1112 is going to fail with this kind of borrow detection
		reg_t a = A[i]; 
		R[i] = a - B[i] - borrow;
		borrow = R[i] + borrow > a ? 1 : 0;
		++i;
		*/

		register reg_t a = A[i]; /* that allow me to work in place instead of requiring R to be a different array than A*/
		register reg_t b = B[i];
		R[i] = a - b - borrow;	
		borrow = (R[i] > a) | (borrow & (b==a)); /* that's an hack, only tests will say if it is going to work, we expect bool values to be 1 or 0*/
		++i;
	}

	while (ASize > i)
	{
#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
		sub_count++;
#endif 
		reg_t a = A[i]; /* that allow me to work in place instead of requiring R to be a different array than A*/
		R[i] = a - borrow;
		
		borrow = R[i] > a ? 1 : 0;
		++i;
		if (borrow == 0)
			break;
	}	

	/* copy remaining words, unless R != A*/
	
		while (ASize > i)
		{
#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
			sub_count++;
#endif 
			R[i] = A[i];
			++i;
		}
	

	while (i > 0 && R[i - 1] == 0)
	{
#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
		sub_count++;
#endif 
		i--;
	}

	return i;
}

