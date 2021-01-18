#include "BigIntSimple.h"



/*  
  Will not check array bounds on R, thus must have space to accomodate MAX(ASIZE,BSIZE)+1 unsigned ints
  returns length of Result
*/
numsize_t LongSumWithCarryDetection(reg_t* A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R)
{
	register reg_t carry;
	register numsize_t i;
	reg_t min_a_b = ASize > BSize ? BSize : ASize;
	/* temp allow me to work in place*/
	reg_t temp;
	carry = i = 0;


	while (min_a_b > i)
	{		
		temp = A[i] + B[i] + carry;
		carry = (temp < A[i] | temp < B[i]); /*careful, check what your compiler produce, btw tests may fail if carry != 1*/
		R[i] = temp;
		++i;
	}

	while (ASize > i && carry == 1)
	{		
		temp = A[i] + carry;
		carry = temp == _R(0) ? _R(1) : _R(0);
		R[i] = temp;
		++i;
	}

	while (ASize > i)
	{
		R[i] = A[i];
		++i;
	}

	while (BSize > i && carry == 1)
	{	
		temp = B[i] + carry;
		carry = temp == _R(0) ? _R(1) : _R(0);
		R[i] = temp;
		++i;
	}

	while (BSize > i)
	{
		R[i] = B[i];		
		++i;
	}

	if (carry)
	{
		R[i] = _R(1);
		++i;
	}

	return  (numsize_t)i;
}

/* this is slower than reference */
/* try to do a sum less but a branch more */
numsize_t LongSumWithCarryDetectionV2(reg_t* A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R)
{
	register reg_t carry, pcarry;
	register numsize_t i;
	reg_t min_a_b = ASize > BSize ? BSize : ASize;

	carry = i = 0;
	pcarry = 0;

	while (min_a_b > i)
	{
		R[i] = A[i] + B[i];		
		pcarry = carry;
		carry = R[i] < A[i] ? 1 : 0;
		R[i] += pcarry;
		if (R[i] == 0)
			carry = 1;
		
		++i;
	}

	while (ASize > i && carry == 1)
	{
		R[i] = A[i] + carry;
		carry = R[i] == _R(0) ? 1 : 0;
		++i;
	}

	while (ASize > i)
	{
		R[i] = A[i];
		++i;
	}

	while (BSize > i && carry == 1)
	{
		R[i] = B[i] + carry;
		carry = R[i] == _R(0) ? 1 : 0;
		++i;
	}

	while (BSize > i)
	{
		R[i] = B[i];
		++i;
	}

	if (carry)
	{
		R[i] = 1;
		++i;
	}

	return  (numsize_t)i;
}

/* this is slower than reference */
/* try to use a single byte carry instead of a register sized */
numsize_t LongSumWithCarryDetectionV3(reg_t* A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R)
{
	register char carry;
	register numsize_t i;
	reg_t min_a_b = ASize > BSize ? BSize : ASize;

	carry = (char)(i = 0);


	while (min_a_b > i)
	{
		R[i] = A[i] + B[i] + carry;
		carry = R[i] < A[i] + carry ? _R(1) : _R(0);
		++i;
	}

	while (ASize > i && carry == 1)
	{
		R[i] = A[i] + carry;
		carry = R[i] == _R(0) ? _R(1) : _R(0);
		++i;
	}

	while (ASize > i)
	{
		R[i] = A[i];
		++i;
	}

	while (BSize > i && carry == 1)
	{
		R[i] = B[i] + carry;
		carry = R[i] == _R(0) ? _R(1) : _R(0);
		++i;
	}

	while (BSize > i)
	{
		R[i] = B[i];
		++i;
	}

	if (carry)
	{
		R[i] = _R(1);
		++i;
	}

	return  (numsize_t)i;
}
