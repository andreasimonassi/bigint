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
static void shouldBeAllZeroesExceptMSD(reg_t *);
static void sum_c_version(reg_t *A, reg_t * B, reg_t * R);
static void sum_asm_x64_version(reg_t *A, reg_t * B, reg_t * R);



static void sum_c_version(reg_t*A, reg_t*B, reg_t*R)
{
	clock_t begin, end;
	int ndigits;
	double time_spent;
	int i;
	printf("Testing speed for C version of sum: ");
	begin = clock();
	for (i = 0; i<TEST_ITERATIONS; ++i)
		ndigits = LongSum(A, A_REG_WORDS, B, B_REG_WORDS, R);
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	double d = TEST_ITERATIONS *A_REG_WORDS*B_REG_WORDS / time_spent;
	printf("%e reg_words processed per second\n", d);
	d = TEST_ITERATIONS / time_spent;
	printf("%e sums per second\n", d);
}

static void sum_asm_x64_version(reg_t*A, reg_t*B, reg_t*R)
{
	clock_t begin, end;
	int ndigits;
	double time_spent;
	int i;
	printf("Testing speed for ASM version of sum: ");
	begin = clock();
	for (i = 0; i<TEST_ITERATIONS; ++i)
		ndigits = LongSumAsm(A, A_REG_WORDS, B, B_REG_WORDS, R);
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

	double d = TEST_ITERATIONS *A_REG_WORDS*B_REG_WORDS / time_spent;
	printf("%e reg_words processed per second\n", d);
	d = TEST_ITERATIONS / time_spent;
	printf("%e sums per second\n", d);
}

static void shouldBeAllZeroesExceptMSD(reg_t * R)
{
	int i;
	int k = R_REG_WORDS;
	for (i = 0; i < k - 1; i++)
	{
		if (R[i] != _R(0))
		{
			printf(" ** Assert failed ** Expected 1 followed by %d zeroes, but %p found at position %d\n", k - 1, R[i], i);
			return;
		}
	}
	if (R[k - 1] != _R(1))
	{
		printf(" ** Assert failed ** Expected 1 followed by %d zeroes, but MSD was %d\n", k - 1, R[k - 1]);
	}
}

static reg_t * initA()
{
	int i;
	reg_t * A;

	A = malloc(A_BYTES);
	MY_ASSERT(A);

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
	MY_ASSERT(B);
	for (i = 0; i < B_REG_WORDS; i++)
	{
		B[i] = _R(1);
	}
	return B;
}



void test_first()
{
	/*the array is in reverse order, less significative on the left...*/
	reg_t * A;
	reg_t * B;
	reg_t * R;

	/* Initialization */

	R = malloc(R_BYTES);
	MY_ASSERT(R);
	A = initA();
	B = initB();
	
	/* Testing C version */
	sum_c_version(A, B, R);
	shouldBeAllZeroesExceptMSD(R);

	/* Testing Assembler x64 version */
	sum_asm_x64_version(A, B, R);
	shouldBeAllZeroesExceptMSD(R);

	/* Cleanup */

	free(A);
	free(B);
	free(R);
	
}

void test_commutative_prop_ams_single_norandom_unit()
{
	/*the array is in reverse order, less significative on the left...*/
	reg_t * A;
	reg_t * B;
	reg_t * R1;
	reg_t * R2;

	/* Initialization */
	reg_t ASize = 0;
	reg_t BSize = 4;
	reg_t RSize = 1 + (ASize > BSize ? ASize : BSize);

	reg_t R1Len;
	reg_t R2Len;
	A = 0;
	B = malloc(BSize * sizeof(reg_t));
	B[0] = 0xb730c46bd0664044;
	B[1] = 0x9d9122f622a13b32;
	B[2] = 0x0299cab0da1f8be1;
	B[3] = 0xc57e802c499d72b9;

	R1 = malloc(RSize * sizeof(reg_t));
	R2 = malloc(RSize * sizeof(reg_t));
	MY_ASSERT((R1 && R2));

	R1Len = LongSumAsm(A, ASize, B, BSize, R1);
	R2Len = LongSumAsm(B, BSize, A, ASize, R2);

	MY_ASSERT(R1Len == R2Len);

	int c = CompareWithPossibleLeadingZeroes(R1, R1Len, R2, R2Len);

	MY_ASSERT(c == 0);

	/* Cleanup */

	free(A);
	free(B);
	free(R1);
	free(R2);
}

void test_commutative_prop_unit(reg_t (*sumfunc)(reg_t*, reg_t, reg_t*,reg_t))
{
	/*the array is in reverse order, less significative on the left...*/
	reg_t * A;
	reg_t * B;
	reg_t * R1;
	reg_t * R2;

	/* Initialization */
	reg_t ASize = rand() % 100;
	reg_t BSize = rand() % 100;
	reg_t RSize = 1 + (ASize > BSize ? ASize : BSize);

	reg_t R1Len;
	reg_t R2Len;
	A = randNum(ASize);
	B = randNum(BSize);
	R1 = malloc(RSize * sizeof(reg_t));
	R2 = malloc(RSize * sizeof(reg_t));
	MY_ASSERT((R1 && R2));

	R1Len = sumfunc(A, ASize, B, BSize, R1);
	R2Len = sumfunc(B, BSize, A, ASize, R2);

	MY_ASSERT(R1Len == R2Len);

	int c = CompareWithPossibleLeadingZeroes(R1, R1Len, R2, R2Len);
	if (c != 0)
	{
		dumpNumber(R1, R1Len);
		dumpNumber(R2, R2Len);
		dumpNumber(A, ASize);
		dumpNumber(B, BSize);
	}
	MY_ASSERT(c == 0);

	/* Cleanup */

	free(A);
	free(B);
	free(R1);
	free(R2);
}

void test_commutative_prop_unit_c()
{
	test_commutative_prop_unit(LongSum);
}

void test_commutative_prop_unit_asm()
{
	test_commutative_prop_unit(LongSumAsm);
}

void test_commutative_prop()
{
	printf("Testing commutative property (C version): ");
	repeat(5000, test_commutative_prop_unit_c);
	printf(" PASSED \n");
	printf("Testing commutative property (ASM version): ");
	repeat(5000, test_commutative_prop_unit_asm);
	printf(" PASSED \n");
}


void test_zero_is_neutral_element_of_sum_c_unit()
{
	
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
	MY_ASSERT((R1 && R2));

	/* Testing C version */
	R1Len = LongSum(A, ASize, B, BSize, R1); /*0+B*/
	R2Len = LongSum(B, BSize, A, ASize, R2); /*B+0*/

	MY_ASSERT(R1Len == R2Len);

	c = CompareWithPossibleLeadingZeroes(R1, R1Len, B, BSize); /*0+B=B*/
	MY_ASSERT(c == 0);

	c = CompareWithPossibleLeadingZeroes(R2, R2Len, B, BSize); /*B+0=B*/
	MY_ASSERT(c == 0);

	/* Cleanup */

	
	free(B);
	free(R1);
	free(R2);
}

void test_zero_is_neutral_element_of_sum()
{
	printf("Testing 0 is neutral element of sum (C version):");
	repeat(5000, test_zero_is_neutral_element_of_sum_c_unit);
	printf(" PASSED \n");
}

void immed()
{
	printf("Testing well known numbers:");
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
		0xb69cbf09,0x982e701f,0xc517a59b,0xa461300c,0x2472893,0x8fba686a,
		0x5dde0f5b,0x648aff44,0xcc5e9a,0x716318e2,0x5a435a3a,0x421418ce,
		0x17b38964,0xa6b0a6d1,0x24ceae81,0xa887a4f2,0xae85cf39,0x96293a7d,
		0x23083277,0xfd69cbf8,0x2fc95b63,0xa957fd2f,0xbf780f66,0xa944b428,
		0x1075329,0x48c1162e,0x61a76c94,0x149f88ca,0x8cc8e0d2,0xd9f0aec4,
		0x6f1b724a,0xa6036e25,0x389229a7,0xc66aed37,0x535e49d0,0xc7f78f82,
		0x6df057f0,0x75b9fad0,0x73f8186e,0xdf031329,0x936791f5 };

	reg_t ASize = sizeof(A) / 8;
	reg_t BSize = sizeof(B) / 8;
	reg_t RSize = 1 + (ASize > BSize ? ASize : BSize);
	
	reg_t R1 = malloc(sizeof(reg_t) *RSize);
	reg_t R2 = malloc(sizeof(reg_t) *RSize);
	MY_ASSERT((R1 && R2));

	reg_t R1Len = LongSum(A, ASize, B, BSize, R1);
	reg_t R2Len = LongSum(B, BSize, A, ASize, R2);

	MY_ASSERT(R1Len == R2Len);

	int c = CompareWithPossibleLeadingZeroes(R1, R1Len, R2, R2Len);

	MY_ASSERT(c == 0);

	free(R1);
	free(R2);
}

void testSum()
{
	immed();
	test_first();
	test_commutative_prop();
	test_zero_is_neutral_element_of_sum();
	
}

