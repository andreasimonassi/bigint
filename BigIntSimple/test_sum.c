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

#define HALF_MEG_NUMBER (1024*1024/sizeof(reg_t)/2)

#define TEST_NUMBER_WORDS 200


static reg_t _A[A_REG_WORDS]; /* PLEASE DO NOT USE THIS IS RESERVED FOR A SPECIAL TEST */
static reg_t _B[B_REG_WORDS]; /* PLEASE DO NOT USE THIS IS RESERVED FOR A SPECIAL TEST */
static reg_t _R[R_REG_WORDS]; /* PLEASE DO NOT USE THIS IS RESERVED FOR A SPECIAL TEST */

static reg_t _HALF_MEG_A[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_B[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_RESULT[HALF_MEG_NUMBER + 1];

static reg_t _TWO_WORDS_A[2];
static reg_t _TWO_WORDS_B[2];





static uint_fast64_t _rand_seed;



static int shouldBeAllZeroesExceptMSD(reg_t *);




static _result_t CastingOutNineTestForSum(reg_t* A, numsize_t asize, reg_t* B, numsize_t bsize, reg_t* Result, numsize_t resultsize)
{
	reg_t mod_a;
	reg_t mod_b;
	reg_t mod_c;
	

	mod_a = CastModuloRegSize(A, asize);
	mod_b = CastModuloRegSize(B, bsize);
	mod_a += mod_b;
	if (mod_a < mod_b)
		mod_a++;
	if (mod_a == _R(-1))
		mod_a = 0;

	mod_c = CastModuloRegSize(Result, resultsize);

	if (mod_a != mod_c)
	{
		LOG_ERROR(STR("(A + B) MOD base_less_1 should be equals to (A mod base_less_1) + (b mod base_less_1) view dump"));

		_fprintf(stderr, STR("DUMP OF A"));
		dumpNumber(A, STR("A"), asize);
		_fprintf(stderr, STR("DUMP OF B"));
		dumpNumber(B, STR("B"), bsize);
		_fprintf(stderr, STR("DUMP OF Result"));
		dumpNumber(Result, STR("Result"), resultsize);
		return _FAIL;
	}
	return _OK;
}

static _result_t CastingOutElevensTestForSum(reg_t* A, numsize_t asize, reg_t* B, numsize_t bsize, reg_t* Result, numsize_t resultsize)
{
	
	reg_t mod_a;
	int carry_a;
	reg_t mod_b;
	int carry_b;
	reg_t mod_c;
	int carry_c;

	mod_a = CastingOutElevens(A, asize, &carry_a);
	mod_b = CastingOutElevens(B, bsize, &carry_b);
	mod_c = CastingOutElevens(Result, resultsize, &carry_c);


	if (carry_a + carry_b == 2)
	{
		carry_a = 0;
		mod_a = _R(-1);
	}
	else {
		mod_a += mod_b;
		if (mod_a < mod_b)
		{
			if (mod_a == 0)
			{
				carry_a = 1;
			}
			else
				mod_a--;
		}
	}
	if (mod_a != mod_c || carry_a != carry_c)
	{
		LOG_ERROR(STR("(A + B) MOD base_plus_1 should be equals to (A mod base_plus_1) + (b mod base_plus_1) view dump"));		

		_fprintf(stderr, STR("DUMP OF A"));
		dumpNumber(A, STR("A"), asize);
		_fprintf(stderr, STR("DUMP OF B"));
		dumpNumber(B, STR("B"), bsize);
		_fprintf(stderr, STR("DUMP OF Result"));
		dumpNumber(Result, STR("Result"), resultsize);

		return _FAIL;
	}

	return _OK;

}
static _result_t CheckSum
(reg_t* A, numsize_t asize, reg_t* B, numsize_t bsize, reg_t* Result, numsize_t resultsize)
{
	_result_t r1;
	r1 = CastingOutNineTestForSum(A, asize, B, bsize, Result, resultsize);
	/*if (OK(r1))
	{
		r1 = CastingOutElevensTestForSum(A, asize, B, bsize, Result, resultsize);
	}*/
	return r1;
}


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

static void each_op(_result_t(*unit_test)(CLOCK_T* outAlgorithmElapsedTime, _operationdescriptor*, void* userData), int boolRepeat,
	_char_t const* const test_description, void* userData, unsigned repeat, double operand1_size, double operand2_size)
{

	_operationdescriptor* descriptor = arithmetic->sum;
	int i;
	for(i=0;i<arithmetic->sumcount;++i)
	{
		_rand_seed = 0x4d595df4d0f33173; /*/deterministic , i want all tests to be reproducible*/
		srand((unsigned int)_rand_seed);

		if (boolRepeat)
		{
			run_test_repeat(unit_test, descriptor,  test_description, userData, repeat, operand1_size, operand2_size);
		}
		else
		{
			run_test_single(unit_test, descriptor, test_description, userData, operand1_size, operand2_size);
		}

		descriptor++;
	}
}


static _result_t test_on_1000_unit(CLOCK_T * delta_t, _operationdescriptor* impl, void * userData)
{
	UNUSED(userData);
	/*the array is in reverse order, less significative on the left...*/
	
	_result_t result = _OK;

	numsize_t ndigits;
	*delta_t = precise_clock();
	ndigits = impl->operation.operation(_A, A_REG_WORDS, _B, B_REG_WORDS, _R);
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
		return _FAIL;
	}
	return CheckSum(_A, A_REG_WORDS, _B, B_REG_WORDS, _R, ndigits);
}

/*SET STACK SIZE TO SOMETHING BIG TO HANDLE THIS USE CASE*/
static _result_t test_speed_1_MB_unit(CLOCK_T * delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	reg_t * A = _HALF_MEG_A;
	reg_t * B = _HALF_MEG_B;
	reg_t * R = _HALF_MEG_RESULT;
	numsize_t ndigits;

	randNum(&_rand_seed, A, HALF_MEG_NUMBER);
	randNum(&_rand_seed, B, HALF_MEG_NUMBER);

	*delta_t = precise_clock();
	ndigits = impl->operation.operation(A, HALF_MEG_NUMBER, B, HALF_MEG_NUMBER, R);
	*delta_t = precise_clock() - *delta_t;

	return CheckSum(A, HALF_MEG_NUMBER, B, HALF_MEG_NUMBER, R, ndigits);	
}

/*SET STACK SIZE TO SOMETHING BIG TO HANDLE THIS USE CASE*/
static _result_t test_speed_512KB_Plus_2Words_unit(CLOCK_T * delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	reg_t * A = _HALF_MEG_A;
	reg_t * B = _TWO_WORDS_B;
	reg_t * R = _HALF_MEG_RESULT;
	numsize_t ndigits;

	randNum(&_rand_seed, A, HALF_MEG_NUMBER);
	randNum(&_rand_seed, B, 2);

	*delta_t = precise_clock();
	ndigits = impl->operation.operation(A, HALF_MEG_NUMBER, B, 2, R);
	*delta_t = precise_clock() - *delta_t;



	return CheckSum(A, HALF_MEG_NUMBER, B, 2, R, ndigits);
}

static _result_t test_last_carry(CLOCK_T * delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;
	reg_t A[] = {_R(-1)};

	reg_t B[] = {_R(1)};

	reg_t RExpected[] = {_R(0),_R(1)};
	reg_t Actual[] = { _R(0),_R(0) };

	numsize_t ActualLen;

	*delta_t = precise_clock();
	ActualLen = impl->operation.operation(A, 1, B, 1, Actual);
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
		return result;
	}


	return CheckSum(A, 1, B, 1, Actual, ActualLen);
}


static _result_t  test_commutative_prop_unit(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
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
	R1Len = impl->operation.operation(A, ASize, B, BSize, R1);
	R2Len = impl->operation.operation(B, BSize, A, ASize, R2);
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
		return result;
	}	

	return CheckSum(A, ASize, B, BSize, R1, R1Len);
}

static _result_t  test_associative_prop_unit(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
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
	R_tempLen = impl->operation.operation(A, ASize, B, BSize, R_temp);
	R1Len = impl->operation.operation(R_temp, R_tempLen, C, CSize, R1);

	R_tempLen = impl->operation.operation(B, BSize, C, CSize, R_temp);
	R2Len = impl->operation.operation(R_temp, R_tempLen, A, ASize, R2);

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


_result_t test_zero_is_neutral_element_of_sum(CLOCK_T * delta_t, _operationdescriptor* impl, void*userData)
{
	UNUSED(userData);
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
	R1Len = impl->operation.operation(A, ASize, B, BSize, R1); /*0+B*/
	*delta_t = precise_clock() - *delta_t;
	R2Len = impl->operation.operation(B, BSize, A, ASize, R2); /*B+0*/

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
#define REPEAT_LONG 10000

void testSum()
{	
	init_test();	
	
	each_op(test_zero_is_neutral_element_of_sum, 1, STR("Testing zero should be neutral element of sum"), NULL, REPEAT_LONG, 0, 0);
	each_op(test_on_1000_unit, 1, STR("Testing that numbers like ff,fe,ff + 1,1 = 1,0,0,0"), NULL, REPEAT_LONG,0,0);
	each_op(test_speed_1_MB_unit, 1, STR("Testing 1MB (512KB+512KB) of data segment allocated numbers"), NULL, REPEAT_LONG, HALF_MEG_NUMBER, HALF_MEG_NUMBER );
	each_op(test_speed_512KB_Plus_2Words_unit, 1, STR("Testing 512KB+2words of data segment allocated numbers"), NULL, REPEAT_LONG, HALF_MEG_NUMBER,2);
	each_op(test_commutative_prop_unit, 1, STR("Test Commutative Property"), NULL, REPEAT_LONG,0,0);
	each_op(test_associative_prop_unit, 1, STR("Test Associative Property"), NULL, REPEAT_LONG,0,0);
	each_op(test_last_carry, 0, STR("Testing last carry (1 + allonebits equals to 1 followed by all zero bits)"), NULL, REPEAT_LONG,0,0);
}


