#include "test.h"

void zero_equals_zero_with_nulls()
{
	printf("testing that 0 equals 0 with nulls: ");
	int c = CompareWithPossibleLeadingZeroes(NULL, 0, NULL, 0);
	MY_ASSERT(c == 0);
	printf("PASSED\n");
}

void zero_equals_zero_with_leading_zeroes()
{
	printf("testing that 0 equals 0 elading zeroes: ");
	int size = 2;
	size *= sizeof(reg_t);
	reg_t i;
	unsigned char * B;
	int c;

	B = malloc(size);
	
	MY_ASSERT(B);
	
	for (i = 0; i < size; i++)
	{
		B[i] = 0;
	}

	
	c = CompareWithPossibleLeadingZeroes(B, 0, B, 0);
	MY_ASSERT(c == 0);
	c = CompareWithPossibleLeadingZeroes(B, 0, B, 1);
	MY_ASSERT(c == 0);
	c = CompareWithPossibleLeadingZeroes(B, 0, B, 2);
	MY_ASSERT(c == 0);

	c = CompareWithPossibleLeadingZeroes(B, 1, B, 0);
	MY_ASSERT(c == 0);
	c = CompareWithPossibleLeadingZeroes(B, 1, B, 1);
	MY_ASSERT(c == 0);
	c = CompareWithPossibleLeadingZeroes(B, 1, B, 2);
	MY_ASSERT(c == 0);

	c = CompareWithPossibleLeadingZeroes(B, 2, B, 0);
	MY_ASSERT(c == 0);
	c = CompareWithPossibleLeadingZeroes(B, 2, B, 1);
	MY_ASSERT(c == 0);
	c = CompareWithPossibleLeadingZeroes(B, 2, B, 2);
	MY_ASSERT(c == 0);

	free(B);

	printf("PASSED\n");
}



void zero_equals_zero()
{	
	zero_equals_zero_with_nulls();
	zero_equals_zero_with_leading_zeroes();	
}


void testCompare()
{
	zero_equals_zero();
}