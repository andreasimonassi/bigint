#include "test.h"
#define HALF_BIG_NUMBER (1024*1024/sizeof(reg_t)/2)
#define BIG_NUMBER (1024*1024/sizeof(reg_t))
#define MBIG_NUMBER (256*1024/sizeof(reg_t))
static reg_t _HALF_MEG_dA[HALF_BIG_NUMBER];
static reg_t _HALF_MEG_dB[HALF_BIG_NUMBER];
static reg_t _HALF_MEG_dC[HALF_BIG_NUMBER];
static reg_t _HALF_MEG_dD[HALF_BIG_NUMBER];
static reg_t _HALF_MEG_dE[HALF_BIG_NUMBER];
static reg_t _HALF_MEG_dF[HALF_BIG_NUMBER];

static uint_fast64_t _rand_seed_4;

static void each_op(_result_t(*unit_test)(CLOCK_T* outAlgorithmElapsedTime, struct _operation_implementations*), int boolRepeat,
	_char_t const * const test_description
)
{

	for (int i = 0; i < number_of_arithmetics; ++i)
	{
		if (arithmetics[i].division == NULL)
		{
			continue;
		}

		_rand_seed_4 = _R(0x4d595df4d0f33173); /*deterministic , i want all tests to be reproducible*/
		srand((unsigned int)_rand_seed_4);


		if (boolRepeat)
		{
			run_test_repeat(unit_test, &(arithmetics[i]), &(arithmetics[i].division_test_results), test_description);
		}
		else
		{
			run_test_single(unit_test, &(arithmetics[i]), &(arithmetics[i].division_test_results), test_description);
		}
	}
}


static _result_t divide_by_zero_returns_error(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;
	numsize_t ASize = rand() % 10 + 2;
	randNum(&_rand_seed_4, _HALF_MEG_dA, ASize);

	CLOCK_T t1 = precise_clock();
	numsize_t q_size;
	numsize_t r_size;
	_div_result_t divresult = impl->division(_HALF_MEG_dA, ASize, NULL, 0, NULL, &q_size, NULL, &r_size);

	*delta_t = precise_clock() - t1;
	
	if (divresult != DIV_BY_ZERO)
	{
		result = _FAIL;
		LOG_ERROR(STR("Expected DIV by zero"));
	}	
	return result;
}


static _result_t divide_by_one_is_identity(CLOCK_T* delta_t, struct _operation_implementations* impl)
{

	_result_t result = _OK;
	numsize_t m = rand() % 10 + 1;
	reg_t* A = _HALF_MEG_dA;
	reg_t* Q = _HALF_MEG_dB;
	reg_t* R = _HALF_MEG_dC;

	randNum(&_rand_seed_4, _HALF_MEG_dA, m);
	reg_t B[2] = { 1,0 };
	numsize_t n = 1;

	CLOCK_T t1 = precise_clock();
	numsize_t q_size;
	numsize_t r_size;
	_div_result_t divresult = impl->division(A, m, B, n, Q, &q_size, R, &r_size);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		dumpNumber(A, STR("A"), m);
		LOG_ERROR(STR("Division returned error"));
	}

	if (r_size != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("A / 1 should not return any remainder but r_size is not zero"));
		dumpNumber(A, L"A", m);
	}

	if (CompareWithPossibleLeadingZeroes(Q, q_size, A, m) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("(A / 1) should be equals to A"));

		dumpNumber(A, STR("A"), m);
		dumpNumber(Q, STR("Result"), q_size);
	}
	return result;
}

static _result_t wc(CLOCK_T* delta_t, struct _operation_implementations* impl)
{

	_result_t result = _OK;

	reg_t A[] = {
	0x8DFE2527F5F052E0,0x78AEF6A5620A4F64,0xAD007D0AE4E67F6C,0x10EB264DB9F903B8,0x0 };

	reg_t B[] = {
	0xEA427434A5879DC6,0x112A5374AE1B79DD,0x0 };

	reg_t Q[] = {
	0x45CE2945CA377628,0xFC51CD353CC58B87,0x0,0x0,0x0 };

	reg_t R[] = {
	0x26F8E9A2747C67F0,0x0,0x0,0x0,0x0 };

	reg_t ComputedQ[] = {
	0x0,0x0,0x0,0x0
	};

	reg_t ComputedR[] = {
	0x0,0x0,0x0,0x0
	};

	numsize_t asize = 4;
	numsize_t bsize = 2;
	numsize_t qsize = 2;
	numsize_t rsize = 1;
	numsize_t computedQsize;
	numsize_t computedRsize;


	CLOCK_T t1 = precise_clock();
	_div_result_t divresult = impl->division(A, asize, B, bsize, ComputedQ, &computedQsize, ComputedR, &computedRsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		LOG_ERROR(STR("Division returned error"));
	}

	if (CompareWithPossibleLeadingZeroes(Q, qsize, ComputedQ, computedQsize) != 0
		|| CompareWithPossibleLeadingZeroes(R, rsize, ComputedR, computedRsize) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("we expect that ComputedQ = Q and if ComputedR = R"));

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(ComputedQ, STR("ComputedQ"), computedQsize);
		dumpNumber(ComputedR, STR("ComputedR"), computedRsize);
	}
	return result;
}

static _result_t wc2(CLOCK_T* delta_t, struct _operation_implementations* impl)
{

	_result_t result = _OK;

	reg_t A[] = {
		0x5e9fbe25e6eb9112,		0xe8299ce5fbfb3fe5,		0x8ec67bc51f44e96e,		0x00aab33b3d06c251		,0x0 };

	reg_t B[] = {
		0xe9f4f650ade7fbd9,0x012f257f0a79f1dd,0x0 };

	reg_t Q[] = {
	0x0,0x0,0x0,0x0,0X0
	};

	reg_t R[] = {
	0x0,0x0,0x0,0x0,0X0
	};

	numsize_t asize = 4;
	numsize_t bsize = 2;
	numsize_t qsize;
	numsize_t rsize;


	CLOCK_T t1 = precise_clock();
	_div_result_t divresult = impl->division(A, asize, B, bsize, Q, &qsize, R, &rsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		LOG_ERROR(STR("Division returned error"));	

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
	}
	return result;
}



static _result_t divide_ok_to_definition(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;
	
	/*Q = A/B, R is A mod B and A = Q*B + R*/
	numsize_t asize;
	numsize_t bsize = rand() % 2 + 1; 
	numsize_t qsize = rand() % 2 + 1;
	numsize_t rsize = 1;
	numsize_t computedQsize;
	numsize_t computedRsize;

	reg_t* A = _HALF_MEG_dA;
	reg_t* B = _HALF_MEG_dB;
	reg_t* Q = _HALF_MEG_dC;
	reg_t* R = _HALF_MEG_dD;
	reg_t* ComputedQ = _HALF_MEG_dE;
	reg_t* ComputedR = _HALF_MEG_dF;

	randNum(&_rand_seed_4, B, bsize);
	randNum(&_rand_seed_4, Q, qsize);
	randNum(&_rand_seed_4, R, 1);


	/* to compute A as B*Q+R we need to pick operation SUM and MUL, we try to pick from current arithmetic otherwise we get it from reference impl */
	
	/* pickup sum and subtraction operations */
	operation sum = impl->addition;
	operation mul = impl->multiplication;
	if (sum == NULL)
	{
		sum = arithmetics[0].addition;
	}
	if (sum == NULL)
	{
		LOG_ERROR(STR("Sum operation is not defined for reference arithmetic"));
		return _FAIL;
	}

	if (mul == NULL)
	{
		mul = arithmetics[0].multiplication;
	}
	if (mul == NULL)
	{
		LOG_ERROR(STR("Multiplication operation is not defined for reference arithmetic"));
		return _FAIL;
	}


	/* we want B > R */
	if (CompareWithPossibleLeadingZeroes(R, rsize, B, bsize) > 0)
	{
		reg_t * temp = R;
		R = B;
		B = temp;
		numsize_t temps = rsize;
		rsize = bsize;
		bsize = temps;
	}

	/* now we can compute A = Q*B+R*/

	asize = mul(Q, qsize, B, bsize, ComputedQ); /* store temp result into ComputedQ*/
	asize = sum(ComputedQ, asize, R, rsize, A);

	/* we're ready to test: we expect that ComputedQ = Q and if ComputedR = R */
	
	CLOCK_T t1 = precise_clock();	
	_div_result_t divresult = impl->division(A, asize, B, bsize, ComputedQ, &computedQsize, ComputedR, &computedRsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		LOG_ERROR(STR("Division returned error"));
	}
	
	if (CompareWithPossibleLeadingZeroes(Q, qsize, ComputedQ, computedQsize) != 0
		|| CompareWithPossibleLeadingZeroes(R,rsize, ComputedR, computedRsize) != 0)		
	{
		result = _FAIL;
		LOG_ERROR(STR("we expect that ComputedQ = Q and if ComputedR = R"));

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(ComputedQ, STR("ComputedQ"), computedQsize);
		dumpNumber(ComputedR, STR("ComputedR"), computedRsize);
	}
	return result;
}

static _result_t divide_ok_to_definition_ext(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;

	/*Q = A/B, R is A mod B and A = Q*B + R*/
	numsize_t asize;
	numsize_t bsize = rand() % 100 + 1;
	numsize_t qsize = rand() % 100 + 1;
	numsize_t rsize = 1;
	numsize_t computedQsize;
	numsize_t computedRsize;

	reg_t* A = _HALF_MEG_dA;
	reg_t* B = _HALF_MEG_dB;
	reg_t* Q = _HALF_MEG_dC;
	reg_t* R = _HALF_MEG_dD;
	reg_t* ComputedQ = _HALF_MEG_dE;
	reg_t* ComputedR = _HALF_MEG_dF;

	randNum(&_rand_seed_4, B, bsize);
	randNum(&_rand_seed_4, Q, qsize);
	randNum(&_rand_seed_4, R, 1);


	/* to compute A as B*Q+R we need to pick operation SUM and MUL, we try to pick from current arithmetic otherwise we get it from reference impl */

	/* pickup sum and subtraction operations */
	operation sum = impl->addition;
	operation mul = impl->multiplication;
	if (sum == NULL)
	{
		sum = arithmetics[0].addition;
	}
	if (sum == NULL)
	{
		LOG_ERROR(STR("Sum operation is not defined for reference arithmetic"));
		return _FAIL;
	}

	if (mul == NULL)
	{
		mul = arithmetics[0].multiplication;
	}
	if (mul == NULL)
	{
		LOG_ERROR(STR("Multiplication operation is not defined for reference arithmetic"));
		return _FAIL;
	}


	/* we want B > R */
	if (CompareWithPossibleLeadingZeroes(R, rsize, B, bsize) > 0)
	{
		reg_t * temp = R;
		R = B;
		B = temp;
		numsize_t temps = rsize;
		rsize = bsize;
		bsize = temps;
	}

	/* now we can compute A = Q*B+R*/

	asize = mul(Q, qsize, B, bsize, ComputedQ); /* store temp result into ComputedQ*/
	asize = sum(ComputedQ, asize, R, rsize, A);

	/* we're ready to test: we expect that ComputedQ = Q and if ComputedR = R */

	CLOCK_T t1 = precise_clock();
	_div_result_t divresult = impl->division(A, asize, B, bsize, ComputedQ, &computedQsize, ComputedR, &computedRsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		LOG_ERROR(STR("Division returned error"));
	}

	if (CompareWithPossibleLeadingZeroes(Q, qsize, ComputedQ, computedQsize) != 0
		|| CompareWithPossibleLeadingZeroes(R, rsize, ComputedR, computedRsize) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("we expect that ComputedQ = Q and if ComputedR = R"));

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(ComputedQ, STR("ComputedQ"), computedQsize);
		dumpNumber(ComputedR, STR("ComputedR"), computedRsize);
	}
	return result;
}

static _result_t divide_small_vs_big(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;

	numsize_t qsize;
	numsize_t asize = rand() % 100 + 1;
	numsize_t bsize = rand() % 100 + 1;
	numsize_t rsize;	

	reg_t* A = _HALF_MEG_dA;
	reg_t* B = _HALF_MEG_dB;
	reg_t* Q = _HALF_MEG_dC;
	reg_t* R = _HALF_MEG_dD;	

	randNum(&_rand_seed_4, A, asize);
	randNum(&_rand_seed_4, B, bsize);

	/* we want A < B */
	if (CompareWithPossibleLeadingZeroes(A, asize, B, bsize) > 0)
	{
		/*swap A <-> B*/
		reg_t * temp = A;
		A = B;
		B = temp;
		numsize_t temps = asize;
		asize = bsize;
		bsize = temps;
	}

	CLOCK_T t1 = precise_clock();
	_div_result_t divresult = impl->division(A, asize, B, bsize, Q, &qsize, R, &rsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		LOG_ERROR(STR("Division returned error"));

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}

	if (CompareWithPossibleLeadingZeroes(Q, qsize, NULL, 0) != 0
		|| CompareWithPossibleLeadingZeroes(R, rsize, B, bsize) != 0		
		)
	{
		result = _FAIL;
		LOG_ERROR(STR("we expect that when A < B then A / B -> Q=0, R=B"));

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
	}

	return result;

}


static _result_t divide_speed_test(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;

	/*Q = A/B, R is A mod B and A = Q*B + R*/
	numsize_t asize = 2000; /* my current impl is too slow to be play better than that size*/
	numsize_t bsize = 50;	
	numsize_t qsize, rsize;
	

	reg_t* A = _HALF_MEG_dA;
	reg_t* B = _HALF_MEG_dB;
	reg_t* Q = _HALF_MEG_dC;
	reg_t* R = _HALF_MEG_dD;
	

	randNum(&_rand_seed_4, A, asize);
	randNum(&_rand_seed_4, B, bsize);

	/* we want A > B */
	if (CompareWithPossibleLeadingZeroes(A, asize, B, bsize) < 0)
	{
		/*swap A <-> B*/
		reg_t * temp = A;
		A = B;
		B = temp;
		numsize_t temps = asize;
		asize = bsize;
		bsize = temps;
	}
	
	/* we're ready to test */

	CLOCK_T t1 = precise_clock();
	_div_result_t divresult = impl->division(A, asize, B, bsize, Q, &qsize, R, &rsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		LOG_ERROR(STR("Division returned error"));
	
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
	}
	return result;
}



void testDiv()
{
		
	
	each_op(wc2, 0, STR("DIV: well known values 2")); /* need to check this special cases */
	each_op(wc, 0, STR("DIV: well known values"));

	each_op(divide_by_zero_returns_error, 0, STR("DIV: Check divide by zero"));
	each_op(divide_by_one_is_identity, 1, STR("DIV: Divide by one must returns same as A"));
	each_op(divide_ok_to_definition, 1, STR("DIV: Q = A/B, R is A mod B and A = Q*B + R (R is a single reg_t digit)"));
	each_op(divide_ok_to_definition_ext, 1, STR("DIV: big values Q = A/B, R is A mod B and A = Q*B + R (R is a single reg_t digit)"));
	each_op(divide_small_vs_big, 1, STR("DIV: when A < B then A / B = 0 and remainder is B"));
	each_op(divide_speed_test, 1, STR("DIV: speed test (2000 Words / 50 Words)"));



}