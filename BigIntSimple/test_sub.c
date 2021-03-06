#include "test.h"

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

static uint_fast64_t _rand_seed_2;


#define HALF_MEG_NUMBER (1024*1024/sizeof(reg_t)/2)

#define TEST_NUMBER_WORDS 500


static reg_t _A[HALF_MEG_NUMBER];
static reg_t _B[HALF_MEG_NUMBER];
static reg_t _C[HALF_MEG_NUMBER];
static reg_t _R1[HALF_MEG_NUMBER];
static reg_t _R2[HALF_MEG_NUMBER];
static reg_t _R3[HALF_MEG_NUMBER];

static reg_t _HALF_MEG_A[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_B[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_RESULT[HALF_MEG_NUMBER + 1];

static reg_t _TWO_WORDS_A[2];
static reg_t _TWO_WORDS_B[2];

static reg_t * initA();
static reg_t * initB();
static reg_t * expected();
static numsize_t expected_digits = 1;

static void each_op(_result_t(*unit_test)(CLOCK_T* outAlgorithmElapsedTime, _operationdescriptor*, void * userData), int boolRepeat,
	 _char_t const * const test_description, void * userData, unsigned repeat, double operand1_size, double operand2_size
)
{
	_operationdescriptor* descriptor = arithmetic->subtract;
	int i;
	for(i=0;i<arithmetic->subtractcount;++i)
	{
		_rand_seed_2 = _R(0x4d595df4d0f33173); //deterministic , i want all tests to be reproducible
		srand((unsigned int)_rand_seed_2);


		if (boolRepeat)
		{
			run_test_repeat(unit_test, descriptor, test_description, userData, repeat, operand1_size, operand2_size);
		}
		else
		{
			run_test_single(unit_test, descriptor, test_description, userData, operand1_size, operand2_size);
		}
		descriptor++;
	}
}




static reg_t * initA()
{

	numsize_t i;
	reg_t * A;

	A = (reg_t*) malloc(A_BYTES);
	if (A == NULL)
	{
		perror("CAN'T ALLOCATE MEMORY");
		return NULL; /* compiler throw warnings otherwise*/
	}
	for (i = 0; i < A_REG_WORDS-1; i++)
	{
		A[i] = _R(0);
	}	
	A[A_REG_WORDS - 1] = 1; //msd

	return A;
}

static reg_t * expected()
{
	numsize_t i;
	reg_t * A;

	A = (reg_t*)malloc(A_BYTES);
	if (!A)
		perror("CAN'T ALLOCATE MEMORY");
	else
	{
		A[0] = _R(1);
		for (i = 1; i < B_REG_WORDS; i++)
		{
			A[i] = _R(0);
		}

		for (i = B_REG_WORDS; i < A_REG_WORDS; i++)
		{
			A[i] = _R(-1);
		}

	}
	return A;
}

static reg_t * initB()
{
	numsize_t i;
	reg_t * B;
	B = malloc(B_BYTES);
	if (B)	
	{
		for (i = 0; i < B_REG_WORDS; i++)
		{
			B[i] = _R(-1);
		}
	}
	else
	{
		perror("NO MEMORY");
	}
	return B;
}

static void init_A_B_Big(reg_t ** outA, numsize_t * outASize, reg_t ** outB, numsize_t * outBSize, reg_t ** outR)
{

	numsize_t ASize = 1+rand32(&_rand_seed_2) % (HALF_MEG_NUMBER - 1);
	numsize_t BSize = 1+rand32(&_rand_seed_2) % (HALF_MEG_NUMBER-1);
	numsize_t temp;

	/*ensure A >= B*/
	if (ASize < BSize)
	{
		temp = ASize;
		ASize = BSize;
		BSize = temp;
	}

	randNum(&_rand_seed_2, _A, ASize);
	randNum(&_rand_seed_2, _B, BSize);

	if (_A[ASize - 1] == _R(0))
		_A[ASize - 1] = _R(1);
	//now ensure that A >= B
	if (ASize == BSize )
	{
		reg_t k = ASize - 1;
		while (k>=0 && _A[k] < _B[k])
		{
			_A[k] = _B[k];
			k--;
		}		
	}
	*outA = _A;
	*outB = _B;
	*outASize = ASize;
	*outBSize = BSize;
	*outR = _R1;
}

static _result_t speedTestSubtraction512KB(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	reg_t * A;
	reg_t * B;
	reg_t * R;
	numsize_t ASize;
	numsize_t BSize;
	numsize_t RSize;
	init_A_B_Big(&A, &ASize, &B, &BSize, &R);

	*delta_t = precise_clock();
	RSize = impl->operation.operation(A, ASize, B, BSize, R);
	*delta_t = precise_clock() - *delta_t;

	return _OK;
}


static _result_t BorrowPropagationCornerCase(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;
	/*the array is in reverse order, less significative on the left...*/
	reg_t* A = _A;	
	reg_t* B = _B;
	reg_t* R = _R1;
	reg_t* _exp = _R2;
	numsize_t ASize = 5;
	numsize_t BSize = 4;
	numsize_t expectedSize = 4;
	/* Initialization */

	_A[0] = 1;
	_A[1] = 1;
	_A[2] = 1;
	_A[3] = 1;
	_A[4] = 1;

	_B[0] = 2;
	_B[1] = 1;
	_B[2] = 1;
	_B[3] = 1;
	
	_exp[0] = _R(-1);//corresponds to 9 on base 10 transposed to base register_size
	_exp[1] = _R(-1);
	_exp[2] = _R(-1);
	_exp[3] = _R(-1);

	
	*delta_t = precise_clock();
	numsize_t n = impl->operation.operation(A, ASize, B, BSize, R);
	*delta_t = precise_clock() - *delta_t;

	if (n != expectedSize)
	{
		LOG_ERROR(STR("Unexpected number of digits, see dump"));
		result = _FAIL;
	}
	else if (CompareWithPossibleLeadingZeroes(R, n, _exp, expectedSize) != 0)
	{
		LOG_ERROR(STR("11111 - 1112 should be 9999, see dump"));
		result = _FAIL;
	}
	if (FAILED(result))
	{
		_fprintf(stderr, STR("DUMP : A - B  = R"));
		dumpNumber(A, STR("A"), A_REG_WORDS);
		dumpNumber(B, STR("B"), B_REG_WORDS);
		dumpNumber(_exp, STR("ExpectedResult"), expectedSize);
		dumpNumber(R, STR("ActualResult"), n);
	}

	return result;

}



static _result_t testWellKnownSubtraction(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{	
	UNUSED(userData);
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
	numsize_t n = impl->operation.operation(A, A_REG_WORDS, B, B_REG_WORDS, R);
	*delta_t = precise_clock() - *delta_t;

	if (n != expected_digits)
	{
		LOG_ERROR(STR("Unexpected number of digits, see dump"));
		result = _FAIL;
	}
	else if (CompareWithPossibleLeadingZeroes(R, n, _exp, expected_digits) != 0)
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



static _result_t testMustBorrow(CLOCK_T* delta_t, _operationdescriptor* impl)
{
	_result_t result = _OK;
	/*the array is in reverse order, less significative on the left...*/
	reg_t A[] = {0,0,0,1};
	reg_t B[] = { 1 };
	reg_t ExpectedR[] = { _R(-1), _R(-1), _R(-1) };
	reg_t Actual[4];

	/* Initialization */
	
	/* Testing C version */
	*delta_t = precise_clock();
	numsize_t n = impl->operation.operation(A, 2, B, 1, Actual);
	*delta_t = precise_clock() - *delta_t;

	if (n != 3)
	{
		LOG_ERROR(STR("Unexpected number of digits, see dump"));
		result = _FAIL;
	}
	else if (CompareWithPossibleLeadingZeroes(ExpectedR, n, Actual, n) != 0)
	{
		LOG_ERROR(STR("A - B result was unexpected, see dump"));
		result = _FAIL;
	}
	if (FAILED(result))
	{
		_fprintf(stderr, STR("DUMP : A - B  = R"));
		dumpNumber(A, STR("A"), A_REG_WORDS);
		dumpNumber(B, STR("B"), B_REG_WORDS);
		dumpNumber(ExpectedR, STR("ExpectedResult"), 1);
		dumpNumber(Actual, STR("ActualResult"), n);
	}

	/* Cleanup */
	
	return result;

}



static _result_t testEqualNumbersYieldToZero(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;
	/*the array is in reverse order, less significative on the left...*/
	numsize_t ASize;
	
	ASize = rand() % A_REG_WORDS;

	/* Initialization */

	/* Testing C version */
	*delta_t = precise_clock();
	numsize_t n = impl->operation.operation(_A, ASize, _A, ASize, _R1);
	*delta_t = precise_clock() - *delta_t;

	if (n != 0)
	{
		LOG_ERROR(STR("Result should be zero, see dump"));
		result = _FAIL;
	}	
	if (FAILED(result))
	{
		dumpNumber(_A, STR("A"), ASize);
		dumpNumber(_R1, STR("ActualResult"), n);		
	}

	/* Cleanup */

	return result;

}


static _result_t testSubtractionIsInverseOfSum(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;	
	operation inverse = arithmetic->sum[0].operation.operation;
	numsize_t ASize;
	numsize_t BSize;
	numsize_t RSize;

	numsize_t R1Size;
	numsize_t R2Size;
	numsize_t R3Size;
	
	if (inverse == NULL)
	{
		LOG_ERROR(STR("Sum operation is not defined for reference arithmetic"));
		return _FAIL;
	}

	ASize = rand() % A_REG_WORDS;
	BSize = rand() & B_REG_WORDS;
	RSize = max(ASize, BSize) + 1;

	randNum(&_rand_seed_2, _A, ASize);
	randNum(&_rand_seed_2, _B, BSize);

	R1Size = inverse(_A, ASize, _B, BSize, _R1);

	*delta_t = precise_clock();
	R2Size = impl->operation.operation(_R1, R1Size, _B, BSize, _R2);
	R3Size = impl->operation.operation(_R1, R1Size, _A, ASize, _R3);
	*delta_t = precise_clock() - *delta_t;

	if (R2Size != ASize || R3Size != BSize)
	{
		result = _FAIL;
	}
	else
	{
		int c = CompareWithPossibleLeadingZeroes(_R2, ASize, _A, ASize);
		if (c != 0)
		{
			result = _FAIL;
		}
		c = CompareWithPossibleLeadingZeroes(_R3, BSize, _B, BSize);
		if (c != 0)
		{
			result = _FAIL;
		}
	}

	if (FAILED(result))
	{
		dumpNumber(_A, STR("_A"), ASize);
		dumpNumber(_B, STR("_B"), BSize);
		dumpNumber(_R1, STR("A_plus_B"), R1Size);
		dumpNumber(_R2, STR("A_plus_B_minus_B_should_be_equals_A"), R2Size);
		dumpNumber(_R3, STR("A_plus_B_minus_A_should_be_equals_B"), R3Size);
		LOG_INFO(STR("Sum is expected to be inverse of addition, but test failed see dump"));
	}
	return result;
}

static _result_t testNullBehavesAsZero(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;
	//reg_t ASize = rand() % A_REG_WORDS;
	numsize_t ASize = rand() % 4;
	randNum(&_rand_seed_2, _A, ASize);

	*delta_t = precise_clock();
	numsize_t R2Size = impl->operation.operation(_A, ASize, NULL, 0, _R2);
	*delta_t = precise_clock() - *delta_t;

	if (R2Size != ASize)
	{
		result = _FAIL;
	}
	else
	{
		int c = CompareWithPossibleLeadingZeroes(_A, ASize, _R2, R2Size);
		if (c != 0)
		{
			result = _FAIL;
		}
	}

	if (FAILED(result))
	{
		dumpNumber(_A, STR("_A"), ASize);
		dumpNumber(_R2, STR("A_minus_0_Result"), R2Size);
		LOG_INFO(STR("A - 0 is expected to be equals to A, but test failed see dump"));
	}
	return result;
}

static _result_t testSubtractionIsInverseOfSumOfZero(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;
	operation inverse = arithmetic->subtract->operation.operation;
	

	if (inverse == NULL)
	{
		LOG_ERROR(STR("Sum operation is not defined for reference arithmetic"));
		return _FAIL;
	}

	numsize_t R1Size = inverse(NULL, 0, NULL, 0, _R1);

	*delta_t = precise_clock();
	numsize_t R2Size = impl->operation.operation(_R1, R1Size, NULL, 0, _R2);
	numsize_t R3Size = impl->operation.operation(_R1, R1Size, NULL, 0, _R3);
	*delta_t = precise_clock() - *delta_t;

	if (R2Size != 0 || R3Size != 0)
	{
		result = _FAIL;
	}	

	if (FAILED(result))
	{	
		LOG_INFO(STR("0+0-0 does not return 0"));
	}
	return result;
}

#define REPEAT_LONG 10000
void testSub()
{
	each_op(BorrowPropagationCornerCase, 0, STR("Corner case for borrow propagation"), NULL, 1, 5, 4);
    each_op(testNullBehavesAsZero, 0, STR("A - NULL = A"), NULL, REPEAT_LONG,0,0);
	each_op(testSubtractionIsInverseOfSumOfZero, 0, STR("0 + 0 - 0 = 0"), NULL, REPEAT_LONG,0,0);
	each_op(testWellKnownSubtraction, 1,STR("Testing subtraction with wellknown values"), NULL, REPEAT_LONG,0,0);
	each_op(testSubtractionIsInverseOfSum, 1, STR("Testing subtraction is inverse of sum"), NULL, REPEAT_LONG,0,0);
	each_op(speedTestSubtraction512KB, 1, STR("Test speed on 512KB numbers A-B 10K times"), NULL, REPEAT_LONG,HALF_MEG_NUMBER, HALF_MEG_NUMBER);
	each_op(testEqualNumbersYieldToZero, 1, STR("A - A = 0"), NULL, REPEAT_LONG,0,0);
}