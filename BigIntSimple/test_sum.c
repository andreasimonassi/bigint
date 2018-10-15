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

static reg_t * initA();
static reg_t * initB();

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

static reg_t * initA()
{
	int i;
	reg_t * A;

	A = malloc(A_BYTES);
	MY_ASSERT(A, NOMEM);

	A[0] = _R(-1);

	for (i = 1; i < B_REG_WORDS; i++)
	{
		A[i] = _R(-2);
	}

	for (i = B_REG_WORDS; i < A_REG_WORDS; i++)
	{
		A[i] = _R(-1);
	}

	return A;
}

static reg_t * initB()
{
	int i;
	reg_t * B;
	B = malloc(B_BYTES);
	MY_ASSERT(B, NOMEM);
	for (i = 0; i < B_REG_WORDS; i++)
	{
		B[i] = _R(1);
	}
	return B;
}

static void each_op(_result_t(*unit_test)(CLOCK_T* outElapsedTime, struct _operation_implementations*), int boolRepeat,
	_char_t const * const test_description
	)
{
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
	reg_t * A;
	reg_t * B;
	reg_t * R;
	_result_t result = _OK;

	/* Initialization */

	R = malloc(R_BYTES);
	MY_ASSERT(R, NOMEM);
	A = initA();
	B = initB();



	reg_t ndigits;
	*delta_t = precise_clock();
	ndigits = impl->addition(A, A_REG_WORDS, B, B_REG_WORDS, R);
	*delta_t = precise_clock() - *delta_t;

	if (FAILED(shouldBeAllZeroesExceptMSD(R)))
	{
		LOG_ERROR(STR("The implementation returned unexpected value, view dump"));
		result = _FAIL;

		_fprintf(stderr,STR("DUMP OF A"));
		dumpNumber(A, STR("A"), A_REG_WORDS);
		_fprintf(stderr,STR( "DUMP OF B"));
		dumpNumber(B, STR("B"), B_REG_WORDS);
		_fprintf(stderr,STR( "Expected RESULT should have same size of A + 1 and must have all zeroes except the most significant which must be 1"));

	}


	/* Cleanup */

	free(A);
	free(B);
	free(R);
	return result;
}


static _result_t  test_commutative_prop_unit(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;
	/*the array is in reverse order, less significative on the left...*/
	reg_t * A;
	reg_t * B;
	reg_t * R1;
	reg_t * R2;

	/* Initialization */
	reg_t ASize = rand() % 500;
	reg_t BSize = rand() % 500;
	reg_t RSize = 1 + (ASize > BSize ? ASize : BSize);

	reg_t R1Len;
	reg_t R2Len;
	A = randNum(ASize);
	B = randNum(BSize);
	R1 = malloc(RSize * sizeof(reg_t));
	R2 = malloc(RSize * sizeof(reg_t));

	MY_ASSERT((A && B && R1 && R2), STR("CANNOT ALLOCATE MEMORY"));
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

	free(A);
	free(B);
	free(R1);
	free(R2);

	
	return result;

}



_result_t test_zero_is_neutral_element_of_sum(CLOCK_T * delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;
	/*the array is in reverse order, less significative on the left...*/
	reg_t * A;
	reg_t * B;
	reg_t * R1;
	reg_t * R2;

	/* Initialization */
	reg_t ASize = 0;
	reg_t BSize = rand() % 100;
	reg_t RSize = 1 + (ASize > BSize ? ASize : BSize);

	reg_t R1Len;
	reg_t R2Len;

	int c;

	A = NULL;
	B = randNum(BSize);
	R1 = malloc(RSize * sizeof(reg_t));
	R2 = malloc(RSize * sizeof(reg_t));
	MY_ASSERT((R1 && R2), NOMEM);

	
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

	
	free(B);
	free(R1);
	free(R2);

	return result;
}


_result_t test_well_known_number(CLOCK_T *delta_t, struct _operation_implementations* impl)
{
	reg_t A[] = {
		0xa1591971,0xc74d4861,0xfed765be,0xb8bc142,0x94b26ebd,0xa6c7c191,
		0x2ffad636,0x85b10cb7,0xde028533,0xd53ba75d,0xec5b9efc,0x495d76b,
		0xe7d4871a,0xbe31220c,0x9b7fe457,0xf1c04ba7,0x16ad9bbc,0xc83259e7,
		0xa6f0d091,0xb2b990b3,0xd517ea5f,0x5bc57536,0x73358cf0,0xb108cdd7,
		0x8c999b4c,0xdf739d41,0x6cd63dbd,0x54362b5b,0xa1bcd7cb,0xbcc3fa4f,
		0xf75b0e3f,0x5cad186,0x7d06c623,0x5b3db4a9,0x7e4d25c0,0x2b4ce7a2,
		0x88018f9e,0x2269f298,0x6834aa86,0x2d4597f5,0x2a355b82,0x7ad0dc62,
		0x1d96c6da,0x753c07c8,0xcc295118,0xccf99b4f,0x5fda004,0x6ab95f62,
		0xd4659aa8,0x7e6c57a9,0x88f0624c,0x7444c80d,0xac715d7a,0xfab237b6,
		0xef831fa,0xfb666a0f,0xbad6c3d5,0xa65264c0,0xff9c3855,0x67a66cb1,
		0x681bf403,0xecd4060d,0xc3649ca6,0x218df63b,0x1dc8174a,0x33c144e5,
		0xc2d88835,0x8fa232f5,0x406554f3,0xe2a04692,0x6481224a,0x1a6d4626,
		0x3b7bd445,0x63f9355b,0x12e6922d,0x2a765a18,0x7591bf8a };

	reg_t B[] = {
		0x1d553987,0x328f0125,0x28469711,0x56303d08,0x783a795f,0x2e05977c,
		0x7a64245a,0x46df7409,0xdc331e2b,0x6ccf7b78,0x8a582a3b,0xad268fd3,
		0xb1c54f33,0x5675d662,0xbfabd1fb,0xf38db4ae,0x5f7a35be,0x5280cf82,
		0x80e440e4,0x809d0ce4,0x4ed9f715,0xca778f1f,0x378ba03c,0xbdddbebc,
		0xe76abf81,0x25e23d82,0x4b28184c,0x10d6f27d,0x90b6b248,0xcb4a02f4,
		0x2444d15c,0xe20ed270,0xb244fab2,0x243506bb,0x2a66f3b4,0x9c0f83dd,
		0xb69cbf09,0x982e701f,0xc517a59b,0xa461300c,0x2472893, 0x8fba686a,
		0x5dde0f5b,0x648aff44,0xcc5e9a,  0x716318e2,0x5a435a3a,0x421418ce,
		0x17b38964,0xa6b0a6d1,0x24ceae81,0xa887a4f2,0xae85cf39,0x96293a7d,
		0x23083277,0xfd69cbf8,0x2fc95b63,0xa957fd2f,0xbf780f66,0xa944b428,
		0x1075329, 0x48c1162e,0x61a76c94,0x149f88ca,0x8cc8e0d2,0xd9f0aec4,
		0x6f1b724a,0xa6036e25,0x389229a7,0xc66aed37,0x535e49d0,0xc7f78f82,
		0x6df057f0,0x75b9fad0,0x73f8186e,0xdf031329,0x936791f5 };

	reg_t R[] = {
		0xBEAE52F8,0xF9DC4986,0x271DFCCF,0x61BBFE4B,0xCECE81C, 0xD4CD590E,
		0xAA5EFA90,0xCC9080C0,0xBA35A35E,0x420B22D6,0x76B3C938,0xB1BC673F,
		0x9999D64D,0x14A6F86F,0x5B2BB653,0xE54E0056,0x7627D17B,0x1AB32969,
		0x27D51176,0x33569D98,0x23F1E175,0x263D0456,0xAAC12D2D,0x6EE68C93,
		0x74045ACE,0x555DAC4, 0xB7FE560A,0x650D1DD8,0x32738A13,0x880DFD44,
		0x1B9FDF9C,0xE7D9A3F7,0x2F4BC0D5,0x7F72BB65,0xA8B41974,0xC75C6B7F,
		0x3E9E4EA7,0xBA9862B8,0x2D4C5021,0xD1A6C802,0x2C7C8415,0xA8B44CC,
		0x7B74D636,0xD9C7070C,0xCCF5AFB2,0x3E5CB34F,0x6040FA3F,0xACCD7830,
		0xEC19240C,0x251CFE7A,0xADBF10CE,0x1CCC6CFF,0x5AF72CB4,0x90DB7234,
		0x32006472,0xF8D03607,0xEAA01F39,0x4FAA61EF,0xBF1447BC,0x10EB20DA,
		0x6923472D,0x35951C3B,0x250C093B,0x362D7F06,0xAA90F81C,0xDB1F3A9,
		0x31F3FA80,0x35A5A11B,0x78F77E9B,0xA90B33C9,0xB7DF6C1B,0xE264D5A8,
		0xA96C2C35,0xD9B3302B,0x86DEAA9B,0x9796D41, 0x8F95180, 0x1 };

	reg_t ASize = sizeof(A) / 8;
	reg_t BSize = sizeof(B) / 8;
	reg_t RSize = sizeof(R) / 8;

	reg_t * R1 = (reg_t*) malloc(sizeof(reg_t) *RSize);
	reg_t * R2 = (reg_t*) malloc(sizeof(reg_t) *RSize);
	MY_ASSERT((R1 && R2), NOMEM);

	_result_t result = _OK;

	*delta_t = precise_clock() - *delta_t;
	reg_t R1Len = impl->addition(A, ASize, B, BSize, R1);
	*delta_t = precise_clock() - *delta_t;
	reg_t R2Len = impl->addition(B, BSize, A, ASize, R2);
	
	if (R1Len != R2Len)
	{
		LOG_ERROR(STR("A + B should be equals R, view dump"));
		result = _FAIL;
	}
	if (R1Len != RSize)
	{
		LOG_ERROR(STR("A + B should be equals R, view dump"));
		result = _FAIL;
	}


	int c = CompareWithPossibleLeadingZeroes(R1, R1Len, R2, R2Len);

	if (c != 0)
	{
		LOG_ERROR(STR("A + B should be equals B + A, view dump"));
		result = _FAIL;
	}

	c = CompareWithPossibleLeadingZeroes(R1, R1Len, R, RSize);

	if (c != 0)
	{
		LOG_ERROR(STR("A + B should be equals R, view dump"));
		result = _FAIL;
	}

	_fprintf(stderr, STR("Dump of numbers\n"));
	dumpNumber(A, STR("A"), R1Len);
	dumpNumber(B, STR("B"), R2Len);
	dumpNumber(R1, STR("ActualResultOf_A_plus_B"), R1Len);
	dumpNumber(R2, STR("ActualResultOf_B_plus_A"), R2Len);
	dumpNumber(R, STR("ExpectedResult"), RSize);
	LOG_INFO(STR("Commutative prop test failed, see dump"));

	free(R1);
	free(R2);

	return result;
}

void testSum()
{
	each_op(test_commutative_prop_unit, 1, STR("Test Commutative Property"));
	each_op(test_on_1000_unit, 1, STR("Testing that numbers like ff,fe,ff + 1,1 = 1,0,0,0"));

	each_op(test_well_known_number, 0, STR("Testing well known numbers"));	
	each_op(test_zero_is_neutral_element_of_sum, 1, STR("Testing zero should be neutral element of sum"));
	
}


