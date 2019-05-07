#include "test.h"

/*#define TEST_ITERATIONS 1*/

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

#define HALF_BIG_NUMBER (1024*1024/sizeof(reg_t)/2)

#define TEST_NUMBER_WORDS 500


static reg_t _A[A_REG_WORDS];
static reg_t _B[B_REG_WORDS];
static reg_t _R[R_REG_WORDS];

static reg_t _HALF_MEG_A[HALF_BIG_NUMBER];
static reg_t _HALF_MEG_B[HALF_BIG_NUMBER];
static reg_t _HALF_MEG_RESULT[HALF_BIG_NUMBER + 1];

static reg_t _TWO_WORDS_A[2];
static reg_t _TWO_WORDS_B[2];





static uint_fast64_t _rand_seed;

static int shouldBeAllZeroesExceptMSD(reg_t *);

static _result_t shouldBeAllZeroesExceptMSD(reg_t * R)
{
	numsize_t i;
	numsize_t k = R_REG_WORDS;
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
	numsize_t i;
	
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


static void each_op(_result_t(*unit_test)(CLOCK_T* outElapsedTime, 
	struct _operation_implementations*), int boolRepeat,
	_char_t const * const test_description
	)
{

	

	for (int i = 0; i < number_of_arithmetics; ++i)
	{
		if (arithmetics[i].addition == NULL)
			continue;

		_rand_seed = 0x4d595df4d0f33173; /*/deterministic , i want all tests to be reproducible*/
		srand((unsigned int)_rand_seed);

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

	numsize_t ndigits;
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

/*SET STACK SIZE TO SOMETHING BIG TO HANDLE THIS USE CASE*/
static _result_t test_speed_1_MB_unit(CLOCK_T * delta_t, struct _operation_implementations* impl)
{	
	reg_t * A = _HALF_MEG_A;
	reg_t * B = _HALF_MEG_B;
	reg_t * R = _HALF_MEG_RESULT;

	randNum(&_rand_seed, A, HALF_BIG_NUMBER);
	randNum(&_rand_seed, B, HALF_BIG_NUMBER);

	*delta_t = precise_clock();
	impl->addition(A, HALF_BIG_NUMBER, B, HALF_BIG_NUMBER, R);	
	*delta_t = precise_clock() - *delta_t;

	return _OK;
}

/*SET STACK SIZE TO SOMETHING BIG TO HANDLE THIS USE CASE*/
static _result_t test_speed_512KB_Plus_2Words_unit(CLOCK_T * delta_t, struct _operation_implementations* impl)
{
	reg_t * A = _HALF_MEG_A;
	reg_t * B = _TWO_WORDS_B;
	reg_t * R = _HALF_MEG_RESULT;

	randNum(&_rand_seed, A, HALF_BIG_NUMBER);
	randNum(&_rand_seed, B, 2);

	*delta_t = precise_clock();
	impl->addition(A, HALF_BIG_NUMBER, B, 2, R);
	*delta_t = precise_clock() - *delta_t;

	return _OK;
}

static _result_t test_last_carry(CLOCK_T * delta_t, struct _operation_implementations * impl)
{
	_result_t result = _OK;
	reg_t A[] = {_R(-1)};

	reg_t B[] = {_R(1)};

	reg_t RExpected[] = {_R(0),_R(1)};
	reg_t Actual[] = { _R(0),_R(0) };

	numsize_t ActualLen;

	*delta_t = precise_clock();
	ActualLen = impl->addition(A, 1, B, 1, Actual);
	*delta_t = precise_clock() - *delta_t;

	if (ActualLen != 2 )
	{
		result = _FAIL;
	}
	else
	{
		int c = CompareWithPossibleLeadingZeroes(Actual, 2, RExpected, 2);
		if (c != 0)
		{
			result = _FAIL;
		}
	}
	if (FAILED(result))
	{
		
		dumpNumber(A, STR("A"), 1);
		dumpNumber(B, STR("B"), 1);
		dumpNumber(RExpected, STR("ExpectedResult"), 2);
		dumpNumber(Actual, STR("ActualResult"), ActualLen);
		LOG_INFO(STR("Last carry test failed, see dump"));
	}

	return result;
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
	numsize_t ASize = rand() % TEST_NUMBER_WORDS;
	numsize_t BSize = rand() % TEST_NUMBER_WORDS;
	
	numsize_t R1Len;
	numsize_t R2Len;

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

static _result_t  test_associative_prop_unit(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;
	/*the array is in reverse order, less significative on the left...*/
	reg_t A[TEST_NUMBER_WORDS];
	reg_t B[TEST_NUMBER_WORDS];
	reg_t C[TEST_NUMBER_WORDS];
	reg_t R1[TEST_NUMBER_WORDS + 2];
	reg_t R2[TEST_NUMBER_WORDS + 2];
	reg_t R_temp[TEST_NUMBER_WORDS + 2];

	/* Initialization */
	numsize_t ASize = rand() % TEST_NUMBER_WORDS;
	numsize_t BSize = rand() % TEST_NUMBER_WORDS;
	numsize_t CSize = rand() % TEST_NUMBER_WORDS;
	
	numsize_t R1Len;
	numsize_t R2Len;
	numsize_t R_tempLen;

	randNum(&_rand_seed, A, ASize);
	randNum(&_rand_seed, B, BSize);
	randNum(&_rand_seed, C, CSize);

	*delta_t = precise_clock();
	R_tempLen = impl->addition(A, ASize, B, BSize, R_temp);
	R1Len = impl->addition(R_temp, R_tempLen, C, CSize, R1);

	R_tempLen = impl->addition(B, BSize, C, CSize, R_temp);
	R2Len = impl->addition(R_temp, R_tempLen, A, ASize, R2);

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
		dumpNumber(R1, STR("ActualResultOfA_plus_B_plus_C"), R1Len);
		dumpNumber(R2, STR("ActualResultOfB_plus_C_plus_A"), R2Len);
		dumpNumber(A, STR("A"), ASize);
		dumpNumber(B, STR("B"), BSize);
		dumpNumber(C, STR("C"), CSize);
		LOG_INFO(STR("Associative prop test failed, see dump"));
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
	numsize_t ASize = 0;
	numsize_t BSize = rand() % TEST_NUMBER_WORDS;
	
	numsize_t R1Len;
	numsize_t R2Len;

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
	
	
	each_op(test_zero_is_neutral_element_of_sum, 1, STR("SUM: Testing zero should be neutral element of sum"));

	each_op(test_on_1000_unit, 1, STR("SUM: Testing that numbers like ff,fe,ff + 1,1 = 1,0,0,0"));
	each_op(test_speed_1_MB_unit, 1, STR("SUM: Testing 1MB (512KB+512KB) of data segment allocated numbers"));
	each_op(test_speed_512KB_Plus_2Words_unit, 1, STR("SUM: Testing 512KB+2words of data segment allocated numbers"));	
	
	each_op(test_commutative_prop_unit, 1, STR("SUM: Test Commutative Property"));
	each_op(test_associative_prop_unit, 1, STR("SUM: Test Associative Property"));

	each_op(test_last_carry, 0, STR("SUM: Testing last carry (1 + allonebits equals to 1 followed by all zero bits)"));
	


}


