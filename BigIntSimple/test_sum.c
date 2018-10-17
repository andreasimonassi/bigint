#include "test.h"

#define TEST_ITERATIONS 100000
//#define TEST_ITERATIONS 1

/*testing purposes A_BITS > B_BITS and A_BITS, B_BITS divisible by 64*/
#define A_BITS   65536
#define B_BITS  1024
#define R_BITS  ((A_BITS > B_BITS ? A_BITS : B_BITS)+ sizeof(reg_t)*8 )

#define A_BYTES (A_BITS >> 3)
#define B_BYTES (B_BITS >> 3)
#define R_BYTES (R_BITS >> 3) 

#define A_REG_WORDS (A_BYTES / sizeof(reg_t))
#define B_REG_WORDS (B_BYTES / sizeof(reg_t))
#define R_REG_WORDS (R_BYTES / sizeof(reg_t))

#define HALF_MEGABYTE_NUMBER_WORDS (1024*1024/sizeof(reg_t)/2)

#define TEST_NUMBER_WORDS 500

static reg_t _A[A_REG_WORDS];
static reg_t _B[B_REG_WORDS];
static reg_t _R[R_REG_WORDS];

static reg_t _BIG_A[HALF_MEGABYTE_NUMBER_WORDS];
static reg_t _BIG_B[HALF_MEGABYTE_NUMBER_WORDS];
static reg_t _BIG_R[HALF_MEGABYTE_NUMBER_WORDS + 1];

static uint_fast64_t _rand_seed;

static int shouldBeAllZeroesExceptMSD(reg_t *);

static _result_t shouldBeAllZeroesExceptMSD(reg_t * R)
{
	int i;
	int k = R_REG_WORDS;
	for (i = 0; i < k - 1; i++)
	{
		if (R[i] != _R(0))
		{				
			return _FAIL;
		}
	}
	if (R[k - 1] != _R(1))
	{
		return _FAIL;
	}
	return _OK;
}

static void init_test()
{
	int i;
	
	_A[0] = _R(-1);

	for (i = 1; i < B_REG_WORDS; i++)
	{
		_A[i] = _R(-2);
	}

	for (i = B_REG_WORDS; i < A_REG_WORDS; i++)
	{
		_A[i] = _R(-1);
	}

	for (i = 0; i < B_REG_WORDS; i++)
	{
		_B[i] = _R(1);
	}
}


static void each_op(_result_t(*unit_test)(CLOCK_T* outElapsedTime, struct _operation_implementations*), int boolRepeat,
	_char_t const * const test_description
	)
{

	_rand_seed = 0x4d595df4d0f33173; //deterministic , i want all tests to be reproducible
	srand((unsigned int)_rand_seed);

	for (int i = 0; i < number_of_arithmetics; ++i)
	{
		if (arithmetics[i].addition == NULL)
			continue;

		if (boolRepeat)
		{
			run_test_repeat(unit_test, &(arithmetics[i]), &(arithmetics[i].addition_test_results), test_description);
		}
		else
		{
			run_test_single(unit_test, &(arithmetics[i]), &(arithmetics[i].addition_test_results), test_description);
		}
	}
}


static _result_t test_on_1000_unit(CLOCK_T * delta_t,struct _operation_implementations* impl)
{
	/*the array is in reverse order, less significative on the left...*/
	
	_result_t result = _OK;

	reg_t ndigits;
	*delta_t = precise_clock();
	ndigits = impl->addition(_A, A_REG_WORDS, _B, B_REG_WORDS, _R);
	*delta_t = precise_clock() - *delta_t;

	if (FAILED(shouldBeAllZeroesExceptMSD(_R)))
	{
		LOG_ERROR(STR("The implementation returned unexpected value, view dump"));
		result = _FAIL;

		_fprintf(stderr,STR("DUMP OF A"));
		dumpNumber(_A, STR("A"), A_REG_WORDS);
		_fprintf(stderr,STR( "DUMP OF B"));
		dumpNumber(_B, STR("B"), B_REG_WORDS);
		_fprintf(stderr,STR( "Expected RESULT should have same size of A + 1 and must have all zeroes except the most significant which must be 1"));

	}
	return result;
}

//SET STACK SIZE TO SOMETHING BIG TO HANDLE THIS USE CASE
static _result_t test_speed_1_MB_unit(CLOCK_T * delta_t, struct _operation_implementations* impl)
{	
	reg_t * A = _BIG_A;
	reg_t * B = _BIG_B;
	reg_t * R = _BIG_R;

	randNum(&_rand_seed, A, HALF_MEGABYTE_NUMBER_WORDS);
	randNum(&_rand_seed, B, HALF_MEGABYTE_NUMBER_WORDS);

	*delta_t = precise_clock();
	impl->addition(A, HALF_MEGABYTE_NUMBER_WORDS, B, HALF_MEGABYTE_NUMBER_WORDS, R);	
	*delta_t = precise_clock() - *delta_t;

	return _OK;
}

static _result_t  test_commutative_prop_unit(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;
	/*the array is in reverse order, less significative on the left...*/
	reg_t A[TEST_NUMBER_WORDS];
	reg_t B[TEST_NUMBER_WORDS];
	reg_t R1[TEST_NUMBER_WORDS+1];
	reg_t R2[TEST_NUMBER_WORDS+1];

	/* Initialization */
	reg_t ASize = rand() % TEST_NUMBER_WORDS;
	reg_t BSize = rand() % TEST_NUMBER_WORDS;
	//reg_t RSize = 1 + (ASize > BSize ? ASize : BSize);
	reg_t R1Len;
	reg_t R2Len;

	randNum(&_rand_seed, A, ASize);
	randNum(&_rand_seed, B, BSize);
	
	*delta_t = precise_clock();
	R1Len = impl->addition(A, ASize, B, BSize, R1);
	R2Len = impl->addition(B, BSize, A, ASize, R2);
	*delta_t = precise_clock() - *delta_t;
	if (R1Len != R2Len)
	{
		result = _FAIL;
	}
	else
	{
		int c = CompareWithPossibleLeadingZeroes(R1, R1Len, R2, R2Len);
		if (c != 0)
		{
			result = _FAIL;
		}
	}
	
	/* Cleanup */

	if (FAILED(result))
	{
		dumpNumber(R1, STR( "ActualResultOfA_plus_B"), R1Len);
		dumpNumber(R2, STR("ActualResultOfB_plus_A"), R2Len);
		dumpNumber(A, STR("A"), ASize);
		dumpNumber(B, STR("B"), BSize);
		LOG_INFO(STR("Commutative prop test failed, see dump"));
	}
	
	
	return result;

}



_result_t test_zero_is_neutral_element_of_sum(CLOCK_T * delta_t, struct _operation_implementations* impl)
{
	
	_result_t result = _OK;
	/*the array is in reverse order, less significative on the left...*/

	reg_t * A;
	reg_t B[TEST_NUMBER_WORDS];
	reg_t R1[TEST_NUMBER_WORDS +1];
	reg_t R2[TEST_NUMBER_WORDS +1];

	/* Initialization */
	reg_t ASize = 0;
	reg_t BSize = rand() % TEST_NUMBER_WORDS;
	//reg_t RSize = 1 + (ASize > BSize ? ASize : BSize);

	reg_t R1Len;
	reg_t R2Len;

	int c;

	A = NULL;
	randNum(&_rand_seed, B, BSize);	
	
	*delta_t = precise_clock();
	R1Len = impl->addition(A, ASize, B, BSize, R1); /*0+B*/
	*delta_t = precise_clock() - *delta_t;
	R2Len = impl->addition(B, BSize, A, ASize, R2); /*B+0*/

	if(R1Len != R2Len)
	{		
		result = _FAIL;
	}
	else if ((c = CompareWithPossibleLeadingZeroes(R1, R1Len, B, BSize)) != 0) /*0+B=B*/
	{
		result = _FAIL;
	}
	else if ((c = CompareWithPossibleLeadingZeroes(R2, R2Len, B, BSize)) != 0) /*B+0=B*/
	{
		result = _FAIL;
	}

	if (FAILED(result))
	{
		
		dumpNumber(A, STR("A"), ASize);
		dumpNumber(B, STR("B"), BSize);
		dumpNumber(R1, STR("ActualResultOfA_plus_B"), R1Len);
		dumpNumber(R2, STR("ActualResultOfB_plus_A"), R2Len);
		LOG_INFO(STR("B+0 should be = to 0+B and should be equals to B"));
	}

	/* Cleanup */
	return result;
}


void testSum()
{
	
	init_test();
	
	each_op(test_commutative_prop_unit, 1, STR("Test Commutative Property"));
	each_op(test_on_1000_unit, 1, STR("Testing that numbers like ff,fe,ff + 1,1 = 1,0,0,0"));
	each_op(test_speed_1_MB_unit, 1, STR("Testing 1MB of stack allocated numbers"));
	
	each_op(test_zero_is_neutral_element_of_sum, 1, STR("Testing zero should be neutral element of sum"));
	
}


