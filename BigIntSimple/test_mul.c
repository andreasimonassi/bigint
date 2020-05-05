#include "test.h"
#define HALF_MEG_NUMBER (1024*1024/sizeof(reg_t)/2)
#define BIG_NUMBER (1024*1024/sizeof(reg_t))
#define MBIG_NUMBER (160*1024/sizeof(reg_t))
static reg_t _HALF_MEG_mA[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_mB[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_mC[HALF_MEG_NUMBER];
static reg_t _MEG_mRESULT1[BIG_NUMBER];
static reg_t _MEG_mRESULT2[BIG_NUMBER];
static reg_t _MEG_mRESULT3[BIG_NUMBER];
static reg_t _MEG_mRESULT4[BIG_NUMBER];

static uint_fast64_t _rand_seed_3;

static void each_op(_result_t(*unit_test)(CLOCK_T* outAlgorithmElapsedTime, _operationdescriptor* operation, void* userData), int boolRepeat,
	_char_t const* const test_description, void* userData, unsigned int numberOfRepeat, double operand_size1, double operand_size2
)
{
	_operationdescriptor* descriptor = arithmetic->multiply;
	int i;
	for(i = 0; i < arithmetic->multiplycount; ++i)
	{
		_rand_seed_3 = _R(0x4d595df4d0f33173); /*deterministic , i want all tests to be reproducible*/
		srand((unsigned int)_rand_seed_3);

		if (boolRepeat)
		{
			run_test_repeat(unit_test, descriptor, test_description, userData, numberOfRepeat, operand_size1, operand_size2);
		}
		else
		{
			run_test_single(unit_test, descriptor, test_description, userData, operand_size1, operand_size2);
		}

		descriptor++;
	}
}

static _result_t multiply_by_zero_returns_zero(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);

	_result_t result = _OK;

	numsize_t ASize = rand() % 10 + 2;
	randNum(&_rand_seed_3, _HALF_MEG_mA, ASize);
	randNum(&_rand_seed_3, _HALF_MEG_mB, ASize);

	CLOCK_T t1 = precise_clock();
	numsize_t R1Size = impl->operation.operation(_HALF_MEG_mA, ASize, NULL, 0, _MEG_mRESULT1);
	t1 = precise_clock() - t1;

	if (R1Size != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("Result 1 should be zero, see dump"));

		dumpNumber(_HALF_MEG_mA, STR("A"), ASize);
		dumpNumber(_MEG_mRESULT1, STR("ActualResult"), R1Size);
	}
	CLOCK_T t2 = precise_clock();
	numsize_t R2Size = impl->operation.operation(_HALF_MEG_mB, ASize, NULL, 0, _MEG_mRESULT1);
	t2 = precise_clock() - t2;

	if (R2Size != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("Result 2 should be zero, see dump"));

		dumpNumber(_HALF_MEG_mB, STR("B"), ASize);
		dumpNumber(_MEG_mRESULT1, STR("ActualResult"), R2Size);
	}

	*delta_t = t1 + t2;
	return result;
}

static _result_t multiply_by_one_is_identity(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;

	numsize_t ASize = rand() % 10 + 2;
	randNum(&_rand_seed_3, _HALF_MEG_mA, ASize);
	reg_t one = 1;

	CLOCK_T t1 = precise_clock();
	numsize_t R1Size = impl->operation.operation(_HALF_MEG_mA, ASize, &one, 1, _MEG_mRESULT1);
	t1 = precise_clock() - t1;

	if (R1Size != ASize)
	{
		result = _FAIL;
		LOG_ERROR(STR("A * 1 should be equals to A"));

		dumpNumber(_HALF_MEG_mA, STR("A"), ASize);
		dumpNumber(_MEG_mRESULT1, STR("ActualResult"), R1Size);
	}
	else if (CompareWithPossibleLeadingZeroes(_HALF_MEG_mA, ASize, _MEG_mRESULT1, R1Size) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("A * 1 should be equals to A"));

		dumpNumber(_HALF_MEG_mA, STR("A"), ASize);
		dumpNumber(_MEG_mRESULT1, STR("ActualResult"), R1Size);
	}

	CLOCK_T t2 = precise_clock();
	numsize_t R2Size = impl->operation.operation(&one, 1, _HALF_MEG_mA, ASize, _MEG_mRESULT1);
	t2 = precise_clock() - t2;

	if (R2Size != ASize)
	{
		result = _FAIL;
		LOG_ERROR(STR("1 * A should be equals to A"));

		dumpNumber(_HALF_MEG_mA, STR("A"), ASize);
		dumpNumber(_MEG_mRESULT1, STR("ActualResult"), R2Size);
	}
	else if (CompareWithPossibleLeadingZeroes(_HALF_MEG_mA, ASize, _MEG_mRESULT1, R2Size) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("1 * A should be equals to A"));

		dumpNumber(_HALF_MEG_mA, STR("A"), ASize);
		dumpNumber(_MEG_mRESULT1, STR("ActualResult"), R1Size);
	}

	*delta_t = t1 + t2;
	return result;
}

static _result_t multiply_is_commutative(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;

	numsize_t ASize = rand() % 100 + 2;
	numsize_t BSize = rand() % 100 + 2;
	randNum(&_rand_seed_3, _HALF_MEG_mA, ASize);
	randNum(&_rand_seed_3, _HALF_MEG_mB, BSize);

	CLOCK_T t1 = precise_clock();
	numsize_t R1Size = impl->operation.operation(_HALF_MEG_mA, ASize, _HALF_MEG_mB, BSize, _MEG_mRESULT1);
	numsize_t R2Size = impl->operation.operation(_HALF_MEG_mB, BSize, _HALF_MEG_mA, ASize, _MEG_mRESULT2);
	t1 = precise_clock() - t1;

	if (CompareWithPossibleLeadingZeroes(_MEG_mRESULT1, R1Size, _MEG_mRESULT2, R2Size) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("A * B should be equals to B * A"));

		dumpNumber(_HALF_MEG_mA, STR("A"), ASize);
		dumpNumber(_HALF_MEG_mB, STR("B"), BSize);
		dumpNumber(_MEG_mRESULT1, STR("R1"), R1Size);
		dumpNumber(_MEG_mRESULT2, STR("R2"), R2Size);
	}

	*delta_t = t1;
	return result;
}

static _result_t multiply_is_associative(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;

	numsize_t ASize = rand() % 100 + 2;
	numsize_t BSize = rand() % 100 + 2;
	numsize_t CSize = rand() % 100 + 2;
	randNum(&_rand_seed_3, _HALF_MEG_mA, ASize);
	randNum(&_rand_seed_3, _HALF_MEG_mB, BSize);
	randNum(&_rand_seed_3, _HALF_MEG_mC, CSize);

	CLOCK_T t1 = precise_clock();
	numsize_t R1Size = impl->operation.operation(_HALF_MEG_mA, ASize, _HALF_MEG_mB, BSize, _MEG_mRESULT1);
	R1Size = impl->operation.operation(_MEG_mRESULT1, R1Size, _HALF_MEG_mC, CSize, _MEG_mRESULT2);
	numsize_t R2Size = impl->operation.operation(_HALF_MEG_mB, BSize, _HALF_MEG_mC, CSize, _MEG_mRESULT3);
	R2Size = impl->operation.operation(_MEG_mRESULT3, R2Size, _HALF_MEG_mA, ASize, _MEG_mRESULT4);
	t1 = precise_clock() - t1;

	if (CompareWithPossibleLeadingZeroes(_MEG_mRESULT2, R1Size, _MEG_mRESULT4, R2Size) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("(A * B) * C should be equals to A * (B * C)"));

		dumpNumber(_HALF_MEG_mA, STR("A"), ASize);
		dumpNumber(_HALF_MEG_mB, STR("B"), BSize);
		dumpNumber(_HALF_MEG_mB, STR("C"), BSize);
		dumpNumber(_MEG_mRESULT1, STR("R1"), R1Size);
		dumpNumber(_MEG_mRESULT2, STR("R2"), R2Size);
	}

	*delta_t = t1;
	return result;
}

static _result_t multiply_subtraction_sum_inrail(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);

	operation sum = arithmetic->sum[0].operation.operation;
	operation sub = arithmetic->subtract[0].operation.operation;

	_result_t result = _OK;
	numsize_t BaseSize = rand() % 5 + 2; 
	numsize_t Num1Size = 1;
	numsize_t MultiplierSize;
	numsize_t ResultSize;

	/*
	computing space requirement:

	highest bit can move 1 positions to the left per iteration

	having 400 iterations in the worst case (16 bit words) 400/16 25 words
	or move 400/64 = 6 words for 64 bit words

	thus we need to allocate 25 words + 7 for results in worst case using 400 iterations.

	since we have a 65K words array we have enough space we'll use 100 words per
	number

	*/

	reg_t* base = _HALF_MEG_mA;
	reg_t* num1 = &_HALF_MEG_mA[100];
	reg_t* num2 = &_HALF_MEG_mA[200];
	reg_t* num3 = &_HALF_MEG_mA[300];
	reg_t* multiplier = &_HALF_MEG_mA[400];
	reg_t* test = &_HALF_MEG_mA[500];
	reg_t* one = &_HALF_MEG_mA[600];

	reg_t* temp;

	randNum(&_rand_seed_3, base, BaseSize);
	*delta_t = 0;
	
	if (sum == NULL)
	{
		LOG_ERROR(STR("Sum operation is not defined for reference arithmetic"));
		return _FAIL;
	}
	
	if (sub == NULL)
	{
		LOG_ERROR(STR("Subtraction operation is not defined for reference arithmetic"));
		return _FAIL;
	}

	/*copy base into num1 (num1 = base + 0)*/
	Num1Size = sum(base, BaseSize, NULL, 0, num1);

	/*set multiplier = 1*/
	*one = multiplier[0] = 1;
	MultiplierSize = 1;

	for (int i = 0; i < 200; ++i)
	{
		if (CompareWithPossibleLeadingZeroes(base, BaseSize, num1, Num1Size) <= 0)
		{
			Num1Size = sum(num1, Num1Size, num1, Num1Size, num2);
			/* swap num1 num2 */
			temp = num1;
			num1 = num2;
			num2 = temp;

			MultiplierSize = sum(multiplier, MultiplierSize, multiplier, MultiplierSize, num3);
			/*swap multiplier num3*/
			temp = num3;
			num3 = multiplier;
			multiplier = temp;
		}
		else
		{
			if ((rand() & 0x7) == 0) /*if rightmost bits are 0 means rand() divides 8 this is 12,5% probability if uniform distribution*/
			{
				Num1Size = sub(num1, Num1Size, base, BaseSize, num2);
				/* swap num1 num2 */
				temp = num1;
				num1 = num2;
				num2 = temp;

				MultiplierSize = sub(multiplier, MultiplierSize, one, 1, num3);
				/*swap multiplier num3*/
				temp = num3;
				num3 = multiplier;
				multiplier = temp;
			}
			else
			{
				Num1Size = sum(num1, Num1Size, num1, Num1Size, num2);
				/* swap num1 num2 */
				temp = num1;
				num1 = num2;
				num2 = temp;

				MultiplierSize = sum(multiplier, MultiplierSize, multiplier, MultiplierSize, num3);
				/*swap multiplier num3*/
				temp = num3;
				num3 = multiplier;
				multiplier = temp;
			}
		}

		/* only measure time taken by multiplication */
		CLOCK_T t1 = precise_clock();
		ResultSize = impl->operation.operation(base, BaseSize, multiplier, MultiplierSize, test);
		*delta_t += precise_clock() - t1;

		/* check result and sum */
		if (CompareWithPossibleLeadingZeroes(test, ResultSize, num1, Num1Size) != 0)
		{
			result = _FAIL;
			LOG_ERROR(STR("(base * multiplier) should be equals to base + ... + base (multiplier times)"));

			dumpNumber(base, STR("base"), BaseSize);
			dumpNumber(multiplier, STR("multiplier"), MultiplierSize);
		}
	}

	return result;
}

static _result_t multiply_well_known1(CLOCK_T* delta, _operationdescriptor* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;
	reg_t A[12];
	numsize_t ASize;
	reg_t B[12];
	numsize_t BSize;
	reg_t R[24];
	numsize_t RSize;
	reg_t Expected[24];
	numsize_t ExpectedSize;
	parseFromHex(STR("F015F744EECF854CA0D8D0915A7C1237ADAC9A7553A40C9E157A8E03A648D6AB2AB725FC9EEEB4F1051FBD8EDC4CEAE"), Expected, 24, &ExpectedSize);
	parseFromHex(STR("3707A12DF36CDF1AB3716C08B88E2DD06833C7A949CF1BFD"), A, 12, &ASize);
	parseFromHex(STR("45CE2945CA377628112A5374AE1B79DDEA427434A5879DC6"), B, 12, &BSize);

	*delta = 0;
	RSize = impl->operation.operation(A, ASize, B, BSize, R);

	if (CompareWithPossibleLeadingZeroes(R, RSize, Expected, ExpectedSize) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("Well known value multiplication failed"));

		dumpNumber(A, STR("A"), ASize);
		dumpNumber(B, STR("B"), BSize);
		dumpNumber(Expected, STR("Expected"), ExpectedSize);
		dumpNumber(R, STR("Actual"), RSize);
	}
	return result;
}

static _result_t multiply_speed_tests(CLOCK_T* delta, _operationdescriptor* impl, void* userData)
{
	unsigned int buffer = *((unsigned int*)userData);
	reg_t* A = _HALF_MEG_mA;
	reg_t* B = _HALF_MEG_mB;
	reg_t* R = _MEG_mRESULT1;

	randNum(&_rand_seed_3, A, buffer);

	randNum(&_rand_seed_3, B, buffer);

	CLOCK_T t1 = precise_clock();

	impl->operation.operation(A, buffer, B, buffer, R);

	*delta = precise_clock() - t1;

	return _OK;
}

#define REPEAT_LONG 10000

void testMul()
{

	/* BE AWARE 
	
	if your multiply algorithm have a threshold it may be possible that the tests will not hit many part of your code 
	because many tests works on not too heavy numbers.

	please provide a variant of the algorithm with a very low threshold (on test_config.h) so that the tests will enter those least used 
	part of the code.
	*/
	each_op(multiply_by_zero_returns_zero, 0, STR("Number by zero equals zero"), NULL, REPEAT_LONG, 0, 0);
	each_op(multiply_by_one_is_identity, 0, STR("Multiply by one is identity"), NULL, REPEAT_LONG, 0, 0);
	each_op(multiply_well_known1, 0, STR("Well known values test 1"), NULL, REPEAT_LONG, 0, 0);
	each_op(multiply_subtraction_sum_inrail, 0, STR("Multiplication is adeherent to definition of repetition of sums"), NULL, REPEAT_LONG, 100, 100);
	each_op(multiply_is_commutative, 1, STR("Multiply is commutative"), NULL, REPEAT_LONG, 100, 100);
	each_op(multiply_is_associative, 1, STR("Multiply is associative"), NULL, REPEAT_LONG, 100, 100);

	int wordsize = 8;
	const unsigned int times = 1;

	/*	unsigned ‭times = 32768‬;*/

	for (int i = 0; i < 13; ++i)
	{		
		each_op(multiply_speed_tests, 1, STR("Speed testing"), &wordsize, times, (double)wordsize,(double)wordsize);
		wordsize <<= 1;
		/*times >>= 1;*/
	}
}