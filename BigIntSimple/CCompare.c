#include "BigIntSimple.h"


int CompareWithPossibleLeadingZeroes(reg_t *A, reg_t ASize, reg_t*B, reg_t BSize)
{
	reg_t i = (ASize > BSize ? ASize : BSize) ;

	while (i > BSize) // ignore leading zeroes
	{
		i--;
		if (A[i] != 0)
			return 1;
	}

	while (i > ASize) // ignore leading zeroes
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

	if (i == 0)
		return 0;

	if (A[i] > B[i])
		return 1;

	return -1;
}





