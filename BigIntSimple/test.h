#include "BigIntSimple.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <locale.h>
#include <stdint.h>

#define UNUSED(x) (void)(x)


/* START ----------------------------------
SOME DEFS TO TWEAK FOR EACH PLATFORM */

#define MAXSTRING 512
//#define USE_WCHAR i decided that the string are going to be always multibyte,
#define LITTLE_ENDIAN 

#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#define CLOCK_T unsigned long long
#undef FAILED
#undef BitScanReverse
#else
#warning maybe you want to define a better CLOCK_T for your platform, also checkout test_common.c

#define CLOCK_T clock_t
#endif

/* DEFINITIONS */

#ifndef LITTLE_ENDIAN
#define #BIG_ENDIAN
#endif


	

	typedef wchar_t _char_t;
	#define STR(x) L ## x	
	#define EXPAND_(a) STR(a)
	#define WFILE EXPAND_(__FILE__)	
	#define LOG_INFO(x, ...) fwprintf(stderr, L"[INFO] " x L"\n", ##__VA_ARGS__)
	#define MY_ASSERT(c, x, ...) if (!(c)) { fwprintf(stderr, L"assertion failed: (%s %d) - " x L"\n",  WFILE, __LINE__, ##__VA_ARGS__); abort(); }
	#define LOG_ERROR(x, ...) fwprintf(stderr, L"[ERROR] " x L"\n", ##__VA_ARGS__)
	
	#define NOMEM L"NO MEMORY"	
	#define _fprintf fwprintf	
	#define _sprintf  swprintf 


	/*
	----- END PLATFORM SPECIFIC
	*/

/* make clear what is the result type of a function */
#define _OK 0
#define _FAIL 1
#define OK(x) (x==0)
#define FAILED(x) (x!=0)





#define MAX_OUTER_TIME_FOR_TESTING_SEC 100
#define _ITERATIONS_FOR_RANDOM_TEST 100


void randNum(uint_fast64_t * const refState, reg_t * const A, numsize_t ASize);
uint_fast32_t rand32(uint_fast64_t * const refState);

typedef struct _test_statistics
{	
	double avg_operations_per_second;
	double number_of_iterations;
	double inner_time_sec;
	double outer_time_sec;
	_result_t test_result;
	_char_t  test_description[MAXSTRING];
}test_statistics;

typedef struct _test_statistics_collection
{
	int capacity;
	int count;
	test_statistics ** items;
}test_statistics_collection;


typedef numsize_t(*operation) (reg_t* A, numsize_t ASize, reg_t * B, numsize_t BSize, reg_t* R) ;

typedef _div_result_t(*div_operation) (reg_t *A, numsize_t m, reg_t *B, numsize_t n, reg_t * Q, numsize_t * q, reg_t * R, numsize_t * r);
/* this structure holds all of our different implementation so it can 
   compare all the possible implementations for speed
*/
struct _operation_implementations
{
	/* to print something useful during tests */
	_char_t * implementation_description;

	/* the arithmetic functions being tested, make it null if not (yet) supported by your impl*/
	operation addition;
	operation subtraction;
	operation multiplication;
	div_operation division;

	/* count how many tests results are available for each operation */	

	test_statistics_collection  addition_test_results;
	test_statistics_collection  subtraction_test_results;
	test_statistics_collection  multiplication_test_results;
	test_statistics_collection  division_test_results;
};


extern struct _operation_implementations arithmetics[];
extern int number_of_arithmetics;

void dumpNumber(reg_t * A, _char_t* name, numsize_t ASize);

void run_test_repeat(_result_t(*unit_test)(CLOCK_T * out_algorithmExecutionTiming, struct _operation_implementations*,  void  *  userData),
	struct _operation_implementations* op,
	test_statistics_collection * destination_array,
	 _char_t const * const test_description,
	void *  userData,
	unsigned int repeatCount
	);


void run_test_single(_result_t(*unit_test)(CLOCK_T * out_algorithmExecutionTiming, struct _operation_implementations*, void * userData),
	struct _operation_implementations* op,
	test_statistics_collection * destination_array,
	_char_t const * const test_description,
	void * userData);



void initTest();
void testCompare();
void testSum();
void testSub();
void testMul();
void testDiv();
void testBSR();
void testParse();

void write_summary();

void cleanup();

CLOCK_T precise_clock();
CLOCK_T clock_zero();
double seconds_from_clock(CLOCK_T clock);