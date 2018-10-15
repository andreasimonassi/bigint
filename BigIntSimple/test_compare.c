#include "test.h"

void zero_equals_zero_with_nulls()
{
	_fprintf(stderr, STR("testing that 0 equals 0 with nulls: "));
	int c = CompareWithPossibleLeadingZeroes(NULL, 0, NULL, 0);
	MY_ASSERT(c == 0, STR("zero_equals_zero_with_nulls"));
	_fprintf(stderr,STR ("PASSED\n"));
}

void zero_equals_zero_with_leading_zeroes()
{
	_fprintf(stderr, STR("testing that 0 equals 0 leading zeroes: \n"));
	int size = 3 ;
	reg_t i;
	reg_t * B;
	int c;

	B = malloc(size * sizeof(reg_t));
	
	MY_ASSERT(B, STR("zero_equals_zero_with_leading_zeroes\n"));
	
	for (i = 0; i < size; i++)
	{
		B[i] = 0;
	}

	
	c = CompareWithPossibleLeadingZeroes(B, 0, B, 0);
	MY_ASSERT(c == 0, STR(" [] = []"));
	c = CompareWithPossibleLeadingZeroes(B, 0, B, 1);
	MY_ASSERT(c == 0, STR(" [] = [0]"));
	c = CompareWithPossibleLeadingZeroes(B, 0, B, 2);
	MY_ASSERT(c == 0, STR(" [] = [0][0]"));

	c = CompareWithPossibleLeadingZeroes(B, 1, B, 0);
	MY_ASSERT(c == 0, STR("[0] = []"));
	c = CompareWithPossibleLeadingZeroes(B, 1, B, 1);
	MY_ASSERT(c == 0, STR("[0] = [0]"));
	c = CompareWithPossibleLeadingZeroes(B, 1, B, 2);
	MY_ASSERT(c == 0, STR("[0] = [0][0]"));

	c = CompareWithPossibleLeadingZeroes(B, 2, B, 0);
	MY_ASSERT(c == 0, STR("[0][0] = []"));
	c = CompareWithPossibleLeadingZeroes(B, 2, B, 1);
	MY_ASSERT(c == 0, STR("[0][0] = [0]"));
	c = CompareWithPossibleLeadingZeroes(B, 2, B, 2);
	MY_ASSERT(c == 0, STR("[0][0] = [0][0]"));

	free(B);

	_fprintf(stderr, STR( "PASSED\n"));
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