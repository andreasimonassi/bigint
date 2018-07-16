#include "BigIntSimple.h"

/*
Caller must check:
* R should have MAX(ASize, BSize) space, will not check bounds
* ASize must be > BSize  or ASize can be == BSize if A > B
* Most significant digit of A is not zero

Return value is number of significant digits in result

*/
reg_t LongSub(reg_t* A, reg_t ASize, reg_t * B, reg_t BSize, reg_t* R)
{

	register reg_t i;

	int borrow = (int)(i = 0);
	reg_t msd = -1; //keep track of last most significant digit

	while (BSize > i)
	{
		R[i] = A[i] - B[i] - borrow;
		if(R[i] != 0) 
			msd = i;
		borrow = R[i]  + borrow > A[i] ? 1 : 0;
		++i;
	}

	while (ASize > i)
	{
		R[i] = A[i] - borrow;
		if (R[i] != 0)
			msd = i;
		borrow = R[i] > A[i] ? 1 : 0;
		++i;
		if (borrow == 0)
			break;
	}

	if (ASize > i)
		msd = ASize - 1;

	while (ASize > i)
	{
		R[i] = A[i];
		++i;
	}

	return msd+1;
}

