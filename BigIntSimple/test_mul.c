#include "test.h"


#define HALF_MEGABYTE_NUMBER_WORDS (1024*1024/sizeof(reg_t)/2)

#define MEGABYTE_NUMBER_WORDS (1024*1024/sizeof(reg_t))

static reg_t _HALF_MEG_mA[HALF_MEGABYTE_NUMBER_WORDS];
static reg_t _HALF_MEG_mB[HALF_MEGABYTE_NUMBER_WORDS];
static reg_t _HALF_MEG_mC[HALF_MEGABYTE_NUMBER_WORDS];
static reg_t _MEG_mRESULT1[MEGABYTE_NUMBER_WORDS];
static reg_t _MEG_mRESULT2[MEGABYTE_NUMBER_WORDS];

static uint_fast64_t _rand_seed_3;

static void each_op(_result_t(*unit_test)(CLOCK_T* outAlgorithmElapsedTime, struct _operation_implementations*), int boolRepeat,
	_char_t const * const test_description
)
{

	for (int i = 0; i < number_of_arithmetics; ++i)
	{
		if (arithmetics[i].multiplication == NULL)
		{
			continue;
		}

		_rand_seed_3 = _R(0x4d595df4d0f33173); //deterministic , i want all tests to be reproducible
		srand((unsigned int)_rand_seed_3);


		if (boolRepeat)
		{
			run_test_repeat(unit_test, &(arithmetics[i]), &(arithmetics[i].multiplication_test_results), test_description);
		}
		else
		{
			run_test_single(unit_test, &(arithmetics[i]), &(arithmetics[i].multiplication_test_results), test_description);
		}
	}
}

static _result_t multiply_by_zero_returns_zero(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;
	//reg_t ASize = rand() % A_REG_WORDS;
	numsize_t ASize = rand() % 10+2;
	randNum(&_rand_seed_3, _HALF_MEG_mA, ASize);
	randNum(&_rand_seed_3, _HALF_MEG_mB, ASize);

	CLOCK_T t1 = precise_clock();
	numsize_t R1Size = impl->multiplication(_HALF_MEG_mA, ASize, NULL, 0, _MEG_mRESULT1);
	t1 = precise_clock() - t1;

	
	if (R1Size != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("Result 1 should be zero, see dump"));

		dumpNumber(_HALF_MEG_mA, STR("A"), ASize);
		dumpNumber(_MEG_mRESULT1, STR("ActualResult"), R1Size);
	}
	CLOCK_T t2 = precise_clock();
	numsize_t R2Size = impl->multiplication(_HALF_MEG_mB, ASize, NULL, 0, _MEG_mRESULT1);
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

static _result_t multiply_by_one_is_identity(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;
	//reg_t ASize = rand() % A_REG_WORDS;
	numsize_t ASize = rand() % 10 + 2;
	randNum(&_rand_seed_3, _HALF_MEG_mA, ASize);
	reg_t one = 1;

	CLOCK_T t1 = precise_clock();
	numsize_t R1Size = impl->multiplication(_HALF_MEG_mA, ASize, &one, 1, _MEG_mRESULT1);
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
	numsize_t R2Size = impl->multiplication(&one, 1, _HALF_MEG_mA, ASize, _MEG_mRESULT1);
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


static _result_t multiply_is_commutative(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;
	//reg_t ASize = rand() % A_REG_WORDS;
	numsize_t ASize = rand() % 2 + 2;
	numsize_t BSize = rand() % 2 + 2;
	randNum(&_rand_seed_3, _HALF_MEG_mA, ASize);
	randNum(&_rand_seed_3, _HALF_MEG_mB, BSize);

	CLOCK_T t1 = precise_clock();
	numsize_t R1Size = impl->multiplication(_HALF_MEG_mA, ASize, _HALF_MEG_mB, BSize, _MEG_mRESULT1);
	numsize_t R2Size = impl->multiplication(_HALF_MEG_mB, BSize, _HALF_MEG_mA, ASize, _MEG_mRESULT2);
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
	   	 

	*delta_t = t1 ;
	return result;
}



static _result_t multiply_is_associative(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;
	//reg_t ASize = rand() % A_REG_WORDS;
	numsize_t ASize = rand() % 10 + 2;
	numsize_t BSize = rand() % 10 + 2;
	numsize_t CSize = rand() % 10 + 2;
	randNum(&_rand_seed_3, _HALF_MEG_mA, ASize);
	randNum(&_rand_seed_3, _HALF_MEG_mB, BSize);
	randNum(&_rand_seed_3, _HALF_MEG_mC, CSize);

	CLOCK_T t1 = precise_clock();
	numsize_t R1Size = impl->multiplication(_HALF_MEG_mA, ASize, _HALF_MEG_mB, BSize, _MEG_mRESULT1);
	R1Size = impl->multiplication(_MEG_mRESULT1, R1Size, _HALF_MEG_mC, CSize, _MEG_mRESULT2);
	numsize_t R2Size = impl->multiplication(_HALF_MEG_mB ,BSize, _HALF_MEG_mC, CSize, _MEG_mRESULT2);
	R1Size = impl->multiplication(_MEG_mRESULT2, R2Size, _HALF_MEG_mA, ASize, _MEG_mRESULT2);
	t1 = precise_clock() - t1;


	if (CompareWithPossibleLeadingZeroes(_MEG_mRESULT1, R1Size, _MEG_mRESULT2, R2Size) != 0)
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


static _result_t multiply_well_known1(CLOCK_T*delta, struct _operation_implementations*impl)
{
	reg_t A[] = {
0x26F8E9A2747C67F0,0x754A646616FDABC4,0x370B5486762F71BE };

	reg_t B[] = {
	0x3A6FE4714641E35C,0x3964850F813FC7BC,0x71971188E2C1F3EF,0x176D56D1C414D802,
	0x9497380AF2E9A9EB,0xEA754304CC701AA9, 0xBDF7F70710CB690E,0x13375D591BB50EA5 };
	reg_t R[12];
	*delta = 0;
	impl->multiplication(A, sizeof(A) / sizeof(reg_t), B, sizeof(B) / sizeof(reg_t), R);
	return _OK;
}

void testMul()
{
	each_op(multiply_by_zero_returns_zero, 0, STR("MUL: Number by zero equals zero"));
	each_op(multiply_by_one_is_identity, 0, STR("MUL: Multiply by one is identity"));
	each_op(multiply_is_commutative, 1, STR("MUL: Multiply is commutative"));
	each_op(multiply_is_associative, 1, STR("MUL: Multiply is associative"));
	each_op(multiply_well_known1, 0, STR("MUL: Well known values test 1"));
}