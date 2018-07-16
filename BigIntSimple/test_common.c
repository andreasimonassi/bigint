#include "test.h"

void repeat(unsigned int N, void(*func)())
{
	for (int i = 0; i < N; ++i)
	{
		func();
	}
}

void dumpNumber(reg_t * A, reg_t ASize)
{
	printf("\nreg_t A [] = {\n");
	for (reg_t i = 0; i < ASize; ++i)
	{
		if (i != 0)
		{
			printf(",");
			if (i % 6 == 0)
				printf("\n");
		}
		printf("0x%x", A[i]);		
		
	}
	printf("};\n");
}

void initTest()
{
	srand(time(NULL));
}

reg_t * randNum(reg_t size)
{

		size *= sizeof(reg_t);
		reg_t i;
		unsigned char * B;
		B = malloc(size);
		if (!B)
		{
			perror("CAN'T ALLOCATE MEMORY");
			exit(EXIT_FAILURE);
		}
		for (i = 0; i < size; i++)
		{
			B[i] = rand();
		}

		return (reg_t*)B;
}