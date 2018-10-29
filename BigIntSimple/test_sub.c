#include "test.h"

#define TEST_ITERATIONS 100000
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

static void each_op(_result_t(*unit_test)(CLOCK_T* outAlgorithmElapsedTime, struct _operation_implementations*), int boolRepeat,
	 _char_t const * const test_description
)
{
	for (int i = 0; i < number_of_arithmetics; ++i)
	{
		if (arithmetics[i].subtraction == NULL)
			continue;

		if (boolRepeat)
		{
			run_test_repeat(unit_test, &(arithmetics[i]), &(arithmetics[i].subtraction_test_results), test_description);
		}
		else
		{
			run_test_single(unit_test, &(arithmetics[i]), &(arithmetics[i].subtraction_test_results), test_description);
		}
	}
}




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


static _result_t testWellKnownSubtraction(CLOCK_T* delta_t, struct _operation_implementations* impl)
{	
	_result_t result = _OK;
	/*the array is in reverse order, less significative on the left...*/
	reg_t * A;
	reg_t * B;
	reg_t * R;
	reg_t * _exp;

	/* Initialization */

	R = malloc(R_BYTES);
	MY_ASSERT(R, NOMEM);
	A = initA();
	B = initB();
	_exp = expected();
	
	/* Testing C version */
	*delta_t = precise_clock();
	reg_t n = impl->subtraction(A, A_REG_WORDS, B, B_REG_WORDS, R);
	*delta_t = precise_clock() - *delta_t;

	if (n != expected_digits)
	{
		LOG_ERROR(STR("Unexpected number of digits, see dump"));
		result = _FAIL;
	}
	else if (CompareWithPossibleLeadingZeroes(R, A_REG_WORDS, _exp, A_REG_WORDS) != 0)
	{
		LOG_ERROR(STR("A - B result was unexpected, see dump"));
		result = _FAIL;
	}
	if (FAILED(result))
	{
		_fprintf(stderr, STR("DUMP : A - B  = R"));
		dumpNumber(A, STR("A"), A_REG_WORDS);
		dumpNumber(B, STR("B"), B_REG_WORDS);
		dumpNumber(_exp, STR("ExpectedResult"), expected_digits);
		dumpNumber(R, STR("ActualResult"), n);
	}

	/* Cleanup */

	free(A);
	free(B);
	free(R);
	free(_exp);

	return result;
	
}

static _result_t testSubtractionIsInverseOfSum(CLOCK_T* delta, struct _operation_implementations* impl)
{

}


void testSub()
{
	each_op(testWellKnownSubtraction, 1,STR("Testing subtraction with wellknown values"));
}