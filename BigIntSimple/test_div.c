#include "test.h"
#define HALF_MEG_NUMBER (1024*1024/sizeof(reg_t)/2)
#define BIG_NUMBER (1024*1024/sizeof(reg_t))
#define MBIG_NUMBER (256*1024/sizeof(reg_t))

static reg_t _HALF_MEG_dA[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_dB[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_dC[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_dD[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_dE[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_dF[HALF_MEG_NUMBER];

static uint_fast64_t _rand_seed_4;

/* In order to test division with different operand sizes create the following structure, it is used just for test*/
struct _speedtest_param
{
	numsize_t asize;
	numsize_t bsize;
};

#ifdef  _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
void startReadableDivisionDebug();
void printReadableDivisionDebug();
#endif //  

static void each_op(_result_t(*unit_test)(CLOCK_T* outAlgorithmElapsedTime, struct _operation_implementations*, void*userData), int boolRepeat,
	_char_t const * const test_description,  void  *  userData
)
{

	for (int i = 0; i < number_of_arithmetics; ++i)
	{
		if (arithmetics[i].division == NULL)
		{
			continue;
		}

#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
		startReadableDivisionDebug();
#endif	


		_rand_seed_4 = _R(0x4d595df4d0f33173); /*deterministic , i want all tests to be reproducible*/
		srand((unsigned int)_rand_seed_4);


		if (boolRepeat)
		{
			run_test_repeat(unit_test, &(arithmetics[i]), &(arithmetics[i].division_test_results), test_description, userData);
		}
		else
		{
			run_test_single(unit_test, &(arithmetics[i]), &(arithmetics[i].division_test_results), test_description, userData);
		}
#ifdef _IMPLEMENTATION_DIVISION_IMPROVED_COLLECT_VERBOSE_DATA
		printReadableDivisionDebug();
#endif
	}



}


static _result_t divide_by_zero_returns_error(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);
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
		if (divresult == OK)
			LOG_ERROR(STR("Division returned OK, but DIV_BY_ZERO was expected"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		return result;
	}
	return result;
}





static _result_t divide_by_one_is_identity(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);

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
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), m);
		dumpNumber(B, STR("B"), n);
		return result;
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



static _result_t divide_A_small_than_B_result_0_q_A(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);

	_result_t result = _OK;
	numsize_t m = rand() % 10 + 1;
	numsize_t n = m + 2;
	numsize_t actualSize;
	reg_t* A = _HALF_MEG_dA;
	reg_t* B = _HALF_MEG_dB;
	reg_t* Q = _HALF_MEG_dF;
	reg_t* R = _HALF_MEG_dC;
	reg_t* temp = _HALF_MEG_dD;
	reg_t* temp2 = _HALF_MEG_dE;

	randNum(&_rand_seed_4, _HALF_MEG_dA, m);
	randNum(&_rand_seed_4, _HALF_MEG_dB, n);

	int cmp = CompareWithPossibleLeadingZeroes(A, m, B, n);
	if (cmp == 0)
	{
		n++;
		B[n] = 1;
	}
	else if (cmp > 0)
	{
		reg_t* temp = B;
		B = A;
		A = temp;

		numsize_t stemp = n;
		n = m;
		m = stemp;
	}

	CLOCK_T t1 = precise_clock();
	numsize_t qsize;
	numsize_t rsize;
	_div_result_t divresult = impl->division(A, m, B, n, Q, &qsize, R, &rsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), m);
		dumpNumber(B, STR("B"), n);
		return result;
	}

	actualSize = LongMultiplication(Q, qsize, B, n, temp);
	actualSize = LongSumWithCarryDetection(temp, actualSize, R, rsize, temp2);
	if (CompareWithPossibleLeadingZeroes(A, m, temp2, actualSize) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("assertion failed: Q * B + R == A "));

		dumpNumber(A, STR("A"), m);
		dumpNumber(B, STR("B"), n);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(temp2, STR("ACTUAL_RESULT"), actualSize);
	}

	else if (CompareWithPossibleLeadingZeroes(R, rsize, B, n) >= 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("assertion failed: we expect R < B"));

		dumpNumber(A, STR("A"), m);
		dumpNumber(B, STR("B"), n);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
	}

	else if (qsize != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("assertion failed: we expect Q == 0"));

		dumpNumber(A, STR("A"), m);
		dumpNumber(B, STR("B"), n);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
	}

	return result;
}

static _result_t wc(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);

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
		if(divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
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

static _result_t _510_div_5_eq_102(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;

	/*5 in base 10 is base/2... on base 2^sizeof(reg_t) 0x80000....0 is like _5*/
	reg_t _5 = _R(1) << ((sizeof(reg_t)<<3)-1);

	reg_t A[] = {		
		0,
		1,
		_5, 0x0 };
	reg_t B[] = {
		 _5, 0 };
	reg_t Q[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t R[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t ExpectedQ[] = { 2, 0, 1 };
	reg_t ExpectedR[] = { 0 };
	numsize_t expqsize=3;
	numsize_t exprsize=0;

	numsize_t asize = 3;
	numsize_t bsize = 1;
	numsize_t qsize;
	numsize_t rsize;


	CLOCK_T t1 = precise_clock();
	_div_result_t divresult = impl->division(A, asize, B, bsize, Q, &qsize, R, &rsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}
	if (CompareWithPossibleLeadingZeroes(Q, qsize, ExpectedQ, expqsize) != 0
		||
		CompareWithPossibleLeadingZeroes(R, rsize, ExpectedR, exprsize) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("EXPECTED VALUES NOT MEET"));
	}

	if (result == _FAIL)
	{
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(ExpectedQ, STR("ExpectedQ"), 4);
		dumpNumber(ExpectedR, STR("ExpectedR"), 1);
	}


	return result;

}


static _result_t _510_div_52_eq_9_r_42(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;

	
	/* numbers are: 
	A = 5,1,0 (decimal)
	B = 5,2 (decimal)
	expected result is
	Q = 9 /decimal
	R = 4, 2

	but on CPU size
	A = "base/2,1,0"
	B = "base/2,2"

	Q = (base-1)
	R = (base/2)-1, 2	
	
	*/
	reg_t _5 = _R(1) << ((sizeof(reg_t)<<3) - 1);

	reg_t A[] = {
		0,
		1,
		_5, 0x0 };
	reg_t B[] = {
			2,
		 _5, 0x00 };
	reg_t Q[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t R[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t ExpectedQ[] = { -1};
	reg_t ExpectedR[] = { 2, _5-1 };
	numsize_t expqsize = 1;
	numsize_t exprsize = 2;


	numsize_t asize = 3;
	numsize_t bsize = 2;
	numsize_t qsize;
	numsize_t rsize;


	CLOCK_T t1 = precise_clock();
	_div_result_t divresult = impl->division(A, asize, B, bsize, Q, &qsize, R, &rsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}
	if (CompareWithPossibleLeadingZeroes(Q, qsize, ExpectedQ, expqsize) != 0
		||
		CompareWithPossibleLeadingZeroes(R, rsize, ExpectedR, exprsize) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("EXPECTED VALUES NOT MEET"));
	}

	if (result == _FAIL)
	{
		
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(ExpectedQ, STR("ExpectedQ"), 4);
		dumpNumber(ExpectedR, STR("ExpectedR"), 1);
	}


	return result;

}

static _result_t wc3(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;
	reg_t A[] = {
		0x9be3a689d1f19fc0,
		0xd52991985bf6af10,
		0xdc2d5219d8bdc6f9,
		0xe9bf91c519078d70, 0x0 };
	reg_t B[] = {
		0xe592143e04ff1ef0, 0x0 };
	reg_t Q[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t R[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t ExpectedQ[] = { 0x75421d1c541433b6, 0xe34ad1170b3cf83a, 0x04a89d9e9ce0d519 , 1 };
	reg_t ExpectedR[] = { 0x413c3b1e46a7d120 };

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
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}
	if (CompareWithPossibleLeadingZeroes(Q, qsize, ExpectedQ, 4) != 0
		||
		CompareWithPossibleLeadingZeroes(R, rsize, ExpectedR, 1) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("EXPECTED VALUES NOT MEET"));
	}

	if (result == _FAIL)
	{
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(ExpectedQ, STR("ExpectedQ"), 4);
		dumpNumber(ExpectedR, STR("ExpectedR"), 1);
	}


	return result;

}



static _result_t AplusSingleDigit_div_A_eq_1_R_SingleDigit(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;



	reg_t A[] = {
		550,
		33,
		44, 0x0 };
	reg_t B[] = {
			0,
		33,
		44, 0x0 };
	reg_t Q[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t R[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t ExpectedQ[] = { 1 };
	reg_t ExpectedR[] = { 550 };
	numsize_t expqsize = 1;
	numsize_t exprsize = 1;


	numsize_t asize = 3;
	numsize_t bsize = 3;
	numsize_t qsize;
	numsize_t rsize;


	CLOCK_T t1 = precise_clock();
	_div_result_t divresult = impl->division(A, asize, B, bsize, Q, &qsize, R, &rsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}
	if (CompareWithPossibleLeadingZeroes(Q, qsize, ExpectedQ, expqsize) != 0
		||
		CompareWithPossibleLeadingZeroes(R, rsize, ExpectedR, exprsize) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("EXPECTED VALUES NOT MEET"));
	}

	if (result == _FAIL)
	{

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(ExpectedQ, STR("ExpectedQ"), 4);
		dumpNumber(ExpectedR, STR("ExpectedR"), 1);
	}


	return result;

}


static _result_t A_div_A_eq_1_R_0(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;



	reg_t A[] = {
		0,
		33,
		44, 0x0 };
	reg_t B[] = {
			0,
		33,
		44, 0x0 };
	reg_t Q[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t R[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t ExpectedQ[] = { 1 };
	reg_t ExpectedR[] = { 0};
	numsize_t expqsize = 1;
	numsize_t exprsize = 0;


	numsize_t asize = 3;
	numsize_t bsize = 3;
	numsize_t qsize;
	numsize_t rsize;


	CLOCK_T t1 = precise_clock();
	_div_result_t divresult = impl->division(A, asize, B, bsize, Q, &qsize, R, &rsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}
	if (CompareWithPossibleLeadingZeroes(Q, qsize, ExpectedQ, expqsize) != 0
		||
		CompareWithPossibleLeadingZeroes(R, rsize, ExpectedR, exprsize) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("EXPECTED VALUES NOT MEET"));
	}

	if (result == _FAIL)
	{

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(ExpectedQ, STR("ExpectedQ"), 4);
		dumpNumber(ExpectedR, STR("ExpectedR"), 1);
	}


	return result;

}


static _result_t wc5(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	_result_t result = _OK;

	reg_t A[] = {
		0x86ca219c582758d4,0x6749c0d8b8d75d1b,0x839487ff90234ab4,0xdb649f5f94b52b2c,	
		0x0 /*padding*/ };

	reg_t B[] = { 0x2eb863295316181a,0xb8f227f68360867e,
		0x0 /*padding*/ };

	reg_t Q[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t R[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	
	reg_t ExpectedQ[] = { 0xccf323a68316dc2d, 0x2fae6b4045c92ad3 ,  0x1 };
	reg_t ExpectedR[] = { 0xb1ca8eda6052c442, 0xb1a0dc34f3685c6c  };

	numsize_t expqsize = 3;
	numsize_t exprsize = 2;
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
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}

	if(
			CompareWithPossibleLeadingZeroes(Q, qsize, ExpectedQ, expqsize) != 0
		||
			CompareWithPossibleLeadingZeroes(R, rsize, ExpectedR, exprsize) != 0
		)
	{
		result = _FAIL;
		LOG_ERROR(STR("EXPECTED VALUES NOT MEET"));
	}

	if (result == _FAIL)
	{
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(ExpectedQ, STR("ExpectedQ"), expqsize);
		dumpNumber(ExpectedR, STR("ExpectedR"), exprsize);
	}

	return result;
}

static _result_t wc4(CLOCK_T* delta_t, struct _operation_implementations* impl, void*userData)
{
	UNUSED(userData);
	/*
	
	if you assert that the guess loop must take 2 tries at most, then this test will make your 
	implementation fail if it will handle that specific case in a not so correct way, or it may 
	let your code throw arithmetic overflow 

	[b-1, 9, 9]  DIV [b-1, 8, 8] => Q => [1] R => [1, 1]

	decimal example : 999 / 988 => Q => 1, Remainder 11

	*/

	_result_t result = _OK;
	reg_t A[] = {
		9,
		9,
		_R(-1),
		0x0 /*padding*/ };
	reg_t B[] = { 8,
		8,
		_R(-1) ,
		0x0 /*padding*/ };
	reg_t Q[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t R[] = {
		0x0,0x0,0x0,0x0,0X0
	};
	reg_t ExpectedQ[] = { 0x1 };
	reg_t ExpectedR[] = { 0x1, 0x1};

	numsize_t asize = 3;
	numsize_t bsize = 3;
	numsize_t qsize;
	numsize_t rsize;


	CLOCK_T t1 = precise_clock();
	_div_result_t divresult = impl->division(A, asize, B, bsize, Q, &qsize, R, &rsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}

	if 
		(
			CompareWithPossibleLeadingZeroes(Q, qsize, ExpectedQ, 1) != 0
				||
			CompareWithPossibleLeadingZeroes(R, rsize, ExpectedR, 2) != 0
		)
	{
		result = _FAIL;
		LOG_ERROR(STR("EXPECTED VALUES NOT MEET"));
	}

	if (result == _FAIL)
	{
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(ExpectedQ, STR("ExpectedQ"), 4);
		dumpNumber(ExpectedR, STR("ExpectedR"), 1);
	}

	return result;
}

static _result_t wc2(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);


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
			if (divresult == DIV_BY_ZERO)
				LOG_ERROR(STR("Division returned DIV BY ZERO error"));
			else
				LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		
		

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
	}
	return result;
}

static _result_t divide_ok_to_definition(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);
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
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
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

static _result_t divide_ok_to_definition_ext(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;

	/*Q = A/B, R is A mod B and A = Q*B + R*/
	numsize_t asize;
	numsize_t bsize = rand() % 128 + 1;
	numsize_t qsize = rand() % 128 + 1;
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
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
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

static _result_t divide_small_vs_big(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;

	numsize_t qsize;
	numsize_t asize = rand() % 128 + 1;
	numsize_t bsize = rand() % 8 + 1;
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
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}

	if (CompareWithPossibleLeadingZeroes(Q, qsize, NULL, 0) != 0
		|| CompareWithPossibleLeadingZeroes(R, rsize, A, asize) != 0		
		)
	{
		result = _FAIL;
		LOG_ERROR(STR("we expect that when A < B then A / B -> Q=0, R=A"));

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
	}

	return result;
}


static _result_t divide_testing_zeroes(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	/*This test is going to discover if guessing algorithm is correct when An is less than B
	in cases like "xy00xy00" / "xy" you get this:

	at step 1
	A1 = xy                                    ; (the remaining digits are "00xy00")
	Q1 = "x"/"x" = "1"                         ; guessing algorithm: correct guess is 1		
	A2 = "xy"- ("1"*"xy") = "0" 
	A2 = "00"                                  ; append next remaining digit which is "0" thus  A2 = "00"
	A2 = trim("00") = "0"                      ; removing leading zeroes
	Q = "1"                                    ; appending Qw to Q

	at step 2
	A2 = "0"                                   ;(remaining digits are "0xy00")
	Q2 = "0" / "x" = "0"                       ;correct guess must be 0	
	A3 = "0" - ("0"*"xy") = "0"                  
	A3 = "00"                                  ;append next digit which is "0"
	A3 = trim("00") = "0"
	Q = "10"                                   ; Append Q2 to Q 

	at step3

	A3 = "0"                                   ;(remaining digits are "xy00")
	Q3 = "0" / "x" = "0"                      ;correct guess must be 0
	A4 = "0" - ("0"*"xy") = "0"
	A4 = "0x"                                  ;append next digit which is "x", thus A3 become "0" after removing leading zeroes
	A4 = trim("0x") = "x"
	Q = "100"                                  ; Append Q2 to Q

	at step4

	A4 = "x"                                   ;(remaining digits are "y00")
	Q4 = "x" / "x" = "1"                       ;correct guess must be 0, but it will be discovered by guessing algorithm
	A5 = "x" - ("1"*"xy") < "0"				   ; we need to adjust guess
	Q4 = "1"-"1" = "0"						   ;guess has been adjusted
	A5 = "x" - ("0"*"xy") = "0"
	A5 = "xy"                                  ;append next digit which is "y"
	Q = "1000"                                  ; Append Q4 to Q

	at step5

	A5 = "xy"                                   ;(remaining digits are "00")
	Q5 = "x" / "x" = "1"                        ;correct guess is 1
	A6 = "xy" - ("1"*"xy") = "0"				; guess is correct	
	A6 = "0"                                    ;append next digit which is "0"
	Q = "10001"                                 ; Append Q4 to Q

	at step6

	A6 = "0"                                    ;(remaining digits are "0")
	Q6 = "0" / "x" = "0"                        ;correct guess is 0
	A7 = "0" - ("0"*"xy") = "0"				    ;guess is correct
	A7 = "00"                                   ;append next digit which is "0"
	A7 = trim(A7) = "0"
	Q = "100010"                                 ; Append Q4 to Q


	at step7 (last step)

	A7 = "0"                                    ;(no digits left)
	Q7 = "0" / "x" = "0"                        ;correct guess is 0
	A8 = "0" - ("0"*"xy") = "0"				    ;guess is correct
	A8 = "00"                                   ;append next digit which is "0"
	A8 = trim(A8) = "0"
	Q = "1000100"                                 ; Append Q4 to Q


	since no digits are left then we quit, remainder  = A8 = "0"

	result must be: Q= "1000100", R = "0"

	*/
	UNUSED(userData);
	_result_t result = _OK;

	/*Q = A/B, R is A mod B and A = Q*B + R*/
	numsize_t asize = 8; /* my current impl is too slow to be play better than that size*/
	numsize_t bsize = 2;
	numsize_t qsize = 7, rsize=1, expected_qsize=7, expected_rsize=0;
	   

	reg_t A[] = {
	0,0,0,0,0,0,0,0,0 };

	reg_t B[] = {
	0,0,0};

	reg_t Q[] = {
	0,0,0,0,0,0,0,0 };

	reg_t R[] = {
	0,0,0,0,0,0,0,0 };

	reg_t ComputedQ[] = {
	0,0,1,0,0,0,1
	};

	reg_t ComputedR[] = {
	0
	};


	randNum(&_rand_seed_4, A + 6, 2);

	B[0] = A[2] = A[6];
	B[1] = A[3] = A[7];
	

	/* we're ready to test */

	CLOCK_T t1 = precise_clock();
	_div_result_t divresult = impl->division(A, asize, B, bsize, Q, &qsize, R, &rsize);
	*delta_t = precise_clock() - t1;

	if (divresult != OK)
	{
		result = _FAIL;
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}
	
	if (CompareWithPossibleLeadingZeroes(Q, qsize, ComputedQ, expected_qsize) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("assertion failed: xy00xy00 / xy must be Q= 1000100 R=0"));

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
	}
	return result;
}

static _result_t divide_random_test(CLOCK_T* delta_t, struct _operation_implementations* impl, void* userData)
{
	UNUSED(userData);
	_result_t result = _OK;

	/*Q = A/B, R is A mod B and A = Q*B + R*/
	numsize_t asize = 4; /* my current impl is too slow to be play better than that size*/
	numsize_t bsize = 1 + (rand() & 0x1);
	numsize_t qsize, rsize, actualSize;

	reg_t* A = _HALF_MEG_dA;
	reg_t* B = _HALF_MEG_dB;
	reg_t* Q = _HALF_MEG_dC;
	reg_t* R = _HALF_MEG_dD;
	reg_t* temp = _HALF_MEG_dE;
	reg_t* actual = _HALF_MEG_dF;

	randNum(&_rand_seed_4, A, asize);
	randNum(&_rand_seed_4, B, bsize);

	/* we want A > B */
	if (CompareWithPossibleLeadingZeroes(A, asize, B, bsize) < 0)
	{
		/*swap A <-> B*/
		reg_t* temp2 = A;
		A = B;
		B = temp2;
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
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}

	actualSize = LongMultiplication(Q, qsize, B, bsize, temp);
	actualSize = LongSumWithCarryDetection(temp, actualSize, R, rsize, actual);
	if (CompareWithPossibleLeadingZeroes(A, asize, actual, actualSize) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("assertion failed: Q * B + R == A "));

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(actual, STR("ACTUAL"), actualSize);
	}

	if (CompareWithPossibleLeadingZeroes(R, rsize, B, bsize) >= 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("assertion failed: R < B"));

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
	}


	return result;
}

static _result_t divide_speed_test(CLOCK_T* delta_t, struct _operation_implementations* impl, void * userData)
{
	_result_t result = _OK;
	struct _speedtest_param* p1 = (struct _speedtest_param*) userData;
	/*Q = A/B, R is A mod B and A = Q*B + R*/

	numsize_t qsize, rsize;	
	numsize_t asize = p1->asize, bsize = p1->bsize;
	numsize_t actualSize;

	reg_t* A = _HALF_MEG_dA;
	reg_t* B = _HALF_MEG_dB;
	reg_t* Q = _HALF_MEG_dC;
	reg_t* R = _HALF_MEG_dD;
	reg_t* temp = _HALF_MEG_dE;
	reg_t* actual = _HALF_MEG_dF;

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
		if (divresult == DIV_BY_ZERO)
			LOG_ERROR(STR("Division returned DIV BY ZERO error"));
		else
			LOG_ERROR(STR("Division returned Generic error (probably end of memory)"));
		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		return result;
	}

	actualSize = LongMultiplication(Q, qsize, B, bsize, temp);
	actualSize = LongSumWithCarryDetection(temp, actualSize, R, rsize, actual);
	if (CompareWithPossibleLeadingZeroes(A, asize, actual, actualSize) != 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("assertion failed: we want Q * B + R == A "));

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
		dumpNumber(actual, STR("ACTUAL"), actualSize);
	}

	if (CompareWithPossibleLeadingZeroes(R, rsize, B, bsize) >= 0)
	{
		result = _FAIL;
		LOG_ERROR(STR("assertion failed: we want R < B"));

		dumpNumber(A, STR("A"), asize);
		dumpNumber(B, STR("B"), bsize);
		dumpNumber(Q, STR("Q"), qsize);
		dumpNumber(R, STR("R"), rsize);
	}


	return result;
}







void testDiv()
{
	

	/*
	Those tests are not portable outside 64 bits machine.

	You can use those functions to check for specific values which make other tests fail,
	edit them and enable them just when debuggin
	*/
#ifdef _DEBUG
	each_op(wc5, 0, STR("DIV: well known values 5"), NULL);/* got overflow with that numbers*/
	each_op(wc4, 0, STR("DIV: well known values 4"), NULL);
	each_op(wc3, 0, STR("DIV: well known values 3"), NULL); 
	each_op(wc2, 0, STR("DIV: well known values 2"), NULL);
	each_op(wc, 0, STR("DIV: well known values"), NULL);
#endif

	/*the following 2 tests will help find guessing error when numbers start with same digit, division should pass this tests*/
	each_op(_510_div_5_eq_102, 0, STR("Handles the cases like decimal 510/5=102 (digit 0 equal, then digits 1 and 2 are 2 times b/2)"), NULL);
	each_op(_510_div_52_eq_9_r_42, 0, STR("Handles the cases like decimal 510/52=9 R 42"), NULL);


	each_op(A_div_A_eq_1_R_0, 0, STR("A/A, Q = 1, R = 0"), NULL);
	each_op(AplusSingleDigit_div_A_eq_1_R_SingleDigit, 0, STR("A/(A + single digit), Q = 1, R = single_digit"), NULL);

	each_op(divide_random_test, 1, STR("DIV: random tests on small numbers"), NULL);

	each_op(divide_by_zero_returns_error, 0, STR("DIV: Check divide by zero"), NULL);
	each_op(divide_by_one_is_identity, 1, STR("DIV: Divide by one must returns same as A"), NULL);
	each_op(divide_ok_to_definition, 1, STR("DIV: Q = A/B, R is A mod B and A = Q*B + R (R is a single reg_t digit)"), NULL);
	each_op(divide_ok_to_definition_ext, 1, STR("DIV: big values Q = A/B, R is A mod B and A = Q*B + R (R is a single reg_t digit)"), NULL);
	each_op(divide_small_vs_big, 1, STR("DIV: when A < B then A / B = 0 and remainder is B"), NULL);
	each_op(divide_testing_zeroes, 1, STR("DIV: Testing guess algorithm on corner cases xy00xy00/xy: Q=1000100 R=0"), NULL);
	each_op(divide_A_small_than_B_result_0_q_A, 1, STR("DIV: Testing SmallNumber/BigNumber, should be like Q=0 R=A"), NULL);

	struct _speedtest_param p1;
	p1.asize = 2048;


	p1.bsize = 1;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 1 Words)"), &p1);
	p1.bsize = 2;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 2 Words)"), &p1);
	p1.bsize = 4;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 4 Words)"), &p1);
	p1.bsize = 8;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 8 Words)"), &p1);
	p1.bsize = 16;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 16 Words)"), &p1);
	p1.bsize = 32;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 32 Words)"), &p1);
	p1.bsize = 64;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 64 Words)"), &p1);
	p1.bsize = 128;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 128 Words)"), &p1);
	p1.bsize = 257;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 256 Words)"), &p1);
	p1.bsize = 512;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 512 Words)"), &p1);
	p1.bsize = 1024;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 1024 Words)"), &p1);
	p1.bsize = 1536;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 1536 Words)"), &p1);
	p1.bsize = 1792;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 1792 Words)"), &p1);
	p1.bsize = 1920;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 1920 Words)"), &p1);
	p1.bsize = 1984;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 1984 Words)"), &p1);
	p1.bsize = 2016;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 2016 Words)"), &p1);
	p1.bsize = 2032;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 2032 Words)"), &p1);
	p1.bsize = 2040;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 2040 Words)"), &p1);
	p1.bsize = 2044;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 2044 Words)"), &p1);
	p1.bsize = 2046;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 2046 Words)"), &p1);
	p1.bsize = 2047;
	each_op(divide_speed_test, 1, STR("DIV: speed test (2048 Words / 2047 Words)"), &p1);
	
		




}