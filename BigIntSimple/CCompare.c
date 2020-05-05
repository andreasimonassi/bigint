#include "BigIntSimple.h"


int CompareWithPossibleLeadingZeroes(reg_t *A, numsize_t ASize, reg_t*B, numsize_t BSize)
{
	numsize_t i = (ASize > BSize ? ASize : BSize) ;
	if (i == 0)
		return 0;

	while (i > BSize) /* ignore leading zeroes*/
	{
		i--;
		if (A[i] != 0)
			return 1;
	}

	while (i > ASize) /* ignore leading zeroes*/
	{
		i--;
		if (B[i] != 0)
			return -1;
	}

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

int CompareWithNoLeadingZeroes(reg_t *A, numsize_t ASize, reg_t*B, numsize_t BSize)
{
	numsize_t i = (ASize > BSize ? ASize : BSize);
	if (i == 0)
		return 0;
	
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





