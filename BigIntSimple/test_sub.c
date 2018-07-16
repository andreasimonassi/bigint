#include "test.h"

#define TEST_ITERATIONS 1000000
//#define TEST_ITERATIONS 1

/*testing purposes A_BITS > B_BITS and A_BITS, B_BITS divisible by 64*/
#define A_BITS   65536
#define B_BITS   65472
#define R_BITS   ((A_BITS > B_BITS ? A_BITS : B_BITS) )

#define A_BYTES (A_BITS >> 3)
#define B_BYTES (B_BITS >> 3)
#define R_BYTES (R_BITS >> 3) 

#define A_REG_WORDS (A_BYTES / sizeof(reg_t))
#define B_REG_WORDS (B_BYTES / sizeof(reg_t))
#define R_REG_WORDS (R_BYTES / sizeof(reg_t))

static reg_t * initA();
static reg_t * initB();
static reg_t * expected();
static int expected_digits = 1;
static int sub_c_version(reg_t *A, reg_t * B, reg_t * R);
//static void sub_asm_x64_version(reg_t *A, reg_t * B, reg_t * R);

static int sub_c_version(reg_t*A, reg_t*B, reg_t*R)
{
	
	clock_t begin, end;
	int ndigits;
	double time_spent;
	int i;
	printf("Testing C Compiled Speed for subtraction: ");
	begin = clock();
	for (i = 0; i<TEST_ITERATIONS; ++i)
		ndigits = LongSub(A, A_REG_WORDS, B, B_REG_WORDS, R);
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	double d = TEST_ITERATIONS *A_REG_WORDS*B_REG_WORDS / time_spent;
	printf(" %e reg_words processed per second\n", d);
	return ndigits;
}

//static void sub_asm_x64_version(reg_t*A, reg_t*B, reg_t*R)
//{
//	clock_t begin, end;
//	int ndigits;
//	double time_spent;
//	int i;
//
//	begin = clock();
//	for (i = 0; i<TEST_ITERATIONS; ++i)
//		ndigits = LongSumAsm(A, A_REG_WORDS, B, B_REG_WORDS, R);
//	end = clock();
//	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
//
//	printf("Assembler version : %f sums per second\n", i / time_spent);
//}


static reg_t * initA()
{

	int i;
	reg_t * A;

	A = malloc(A_BYTES);
	if (!A)
		perror("CAN'T ALLOCATE MEMORY");
	for (i = 0; i < A_REG_WORDS-1; i++)
	{
		A[i] = _R(0);
	}	
	A[A_REG_WORDS - 1] = 1; //msd

	return A;
}

static reg_t * expected()
{
	int i;
	reg_t * A;

	A = malloc(A_BYTES);
	if (!A)
		perror("CAN'T ALLOCATE MEMORY");
	A[0] = _R(1);
	for (i = 1; i < B_REG_WORDS; i++)
	{
		A[i] = _R(0);
	}

	for (i = B_REG_WORDS; i < A_REG_WORDS - 1; i++)
	{
		A[i] = _R(-1);
	}
	A[A_REG_WORDS - 1] = 0;

	return A;
}

static reg_t * initB()
{
	int i;
	reg_t * B;
	B = malloc(B_BYTES);
	if (!B)
		perror("CAN'T ALLOCATE MEMORY");
	for (i = 0; i < B_REG_WORDS; i++)
	{
		B[i] = _R(-1);
	}
	return B;
}


void testSub()
{	
	/*the array is in reverse order, less significative on the left...*/
	reg_t * A;
	reg_t * B;
	reg_t * R;
	reg_t * _exp;
	double time_spent;

	/* Initialization */

	R = malloc(R_BYTES);
	if (!R)
		perror("CAN'T ALLOCATE MEMORY");
	A = initA();
	B = initB();
	_exp = expected();

	/* Print out environment */
	printf("Test SUBTRACTION \n");

	/* Testing C version */
	int n = sub_c_version(A, B, R);
	MY_ASSERT(n == expected_digits);
	

	MY_ASSERT(CompareWithPossibleLeadingZeroes(R, A_REG_WORDS, _exp, A_REG_WORDS) == 0);

	/* Testing Assembler x64 version */
	//asm_x64_version(A, B, R);
	//shouldBeAllZeroesExceptMSD(R);

	/* Cleanup */

	free(A);
	free(B);
	free(R);
	free(_exp);

	printf("Test subtraction passed\n");
	
}
