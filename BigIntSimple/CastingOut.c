#include "BigIntSimple.h"

reg_t CastingOutNines(reg_t* A, numsize_t a) {
	numsize_t i = 0;
	reg_t acc = 0;
	for (; i < a; ++i)
	{
		acc += A[i];
		if (A[i] > acc)
			acc++;
	}
	if (acc == _R(-1))
	{
		acc = 0;
	}
	return acc;
}

reg_t CastingOutElevens(reg_t* A, numsize_t a, int * out_carry)
{
	numsize_t i;
	*out_carry = 0;
	reg_t acc = 0;
	reg_t temp;
	for (i=0; i < a; i+=2)	/*even positions*/
	{				
		temp = acc;
		acc = acc + A[i];
		if (*out_carry == 0 && acc < temp)
		{
			if (acc == 0)
				*out_carry = 1;
			else
				acc = acc - 1;
		}
		else if(*out_carry != 0 && acc!=0)
		{
			acc = acc - 1;/*acc=0 and carry=! cannot happen*/
			*out_carry = 0;
		}
	}
	
	for (i=1; i < a; i += 2)	/*odd positions*/
	{
		temp = acc;
		acc = acc - A[i];
		if (*out_carry == 0 && acc > temp)
		{			
			if (acc == _R(-1))
			{
				*out_carry = 1;				
			}
			acc = acc + 1;
		}
		else if(*out_carry !=0 && acc != 0)
		{			
			*out_carry = 0;
		}
	}	

	return acc;
}