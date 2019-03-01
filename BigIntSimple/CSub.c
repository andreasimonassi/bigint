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
	numsize_t msd = _R(-1); /*keep track of last most significant digit*/

	while (BSize > i)
	{
		reg_t a = A[i]; /* that allow me to work in place instead of requiring R to be a different array than A*/
		R[i] = a - B[i] - borrow;
		if (R[i] != 0)
			msd = i;
		borrow = R[i] + borrow > a ? 1 : 0;
		++i;
	}

	while (ASize > i)
	{
		reg_t a = A[i]; /* that allow me to work in place instead of requiring R to be a different array than A*/
		R[i] = a - borrow;
		if (R[i] != 0)
			msd = i;
		borrow = R[i] > a ? 1 : 0;
		++i;
		if (borrow == 0)
			break;
	}

	if (ASize > i)
		msd = ASize - 1;

	/* copy remaining words, unless R != A*/
	if (R != A)
	{
		while (ASize > i)
		{
			R[i] = A[i];
			++i;
		}
	}

	return msd+1;
}

