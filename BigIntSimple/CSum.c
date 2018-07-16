#include "BigIntSimple.h"



/*  Will not check array bounds on R, thus must have space to accomodate MAX(ASIZE,BSIZE)+1 unsigned ints
	returns length of Result
*/
reg_t LongSum(reg_t* A, reg_t ASize, reg_t * B, reg_t BSize, reg_t* R)
{
	register reg_t carry;
	register reg_t i;
	reg_t min_a_b = ASize > BSize ? BSize : ASize;

	carry = i = 0;


	while (min_a_b > i)
	{		
		R[i] = A[i] + B[i] + carry;
		carry = R[i] < A[i] + carry ? _R(1) : _R(0);
		++i;
	}

	while (ASize > i)
	{		
		R[i] = A[i] + carry;
		carry = R[i] == 0 ? _R(1) : _R(0);
		++i;
	}

	while (BSize > i)
	{	
		R[i] = B[i] + carry;
		carry = R[i] == 0 ? _R(1) : _R(0);
		++i;
	}

	if (carry)
	{
		R[i] = _R(1);
		return i+1;
	}

	return i ;
}