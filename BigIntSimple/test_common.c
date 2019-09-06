#include "test_config.h"

#include <memory.h>


#define _4bitstoascii(a) (a<10?a+48:a+55)

#ifdef BIG_ENDIAN
#error implement printHex
#else
static wchar_t* printHex(reg_t x, wchar_t * buffer)
{	
	const int s = sizeof(reg_t);
	wchar_t * buffercopy = buffer;
	unsigned char const * const c = (unsigned char const * const)(&x);
	int i = s-1;
	unsigned char byte;
	/*/remove leading zeroes*/
	while (i > 0 && c[i] == 0)
	{
		i--;
	}
	
	for (; i >=0; --i)
	{
		byte = c[i] >> 4;
		*buffer =  (wchar_t)_4bitstoascii(byte);
		++buffer;
		byte = c[i] & 0xF;
		*buffer = (wchar_t)_4bitstoascii(byte);
		++buffer;
	}	

	*buffer = 0;
	return buffercopy;
}
#endif
void dumpNumber(reg_t * A, _char_t* name, numsize_t ASize)
{
	_char_t buffer[sizeof(reg_t) * 2 + 1];
	_fprintf(stderr, STR("\nreg_t %s [] = {\n"), name);
	for (numsize_t i = 0; i < ASize; ++i)
	{
		if (i != 0)
		{
			_fprintf(stderr, STR(","));
			if (i % 6 == 0)
				_fprintf(stderr, STR("\n"));
		}
		_fprintf(stderr, STR("0x"));
		_fprintf(stderr, printHex(A[i], buffer));
		
	}
	_fprintf(stderr, STR("};\n"));
}

void initTest()
{
	srand((unsigned int)time(NULL));
	for (int i = 0; i < number_of_arithmetics; ++i)
	{		
		arithmetics[i].division_test_results.capacity = arithmetics[i].division_test_results.count = 0;
		arithmetics[i].addition_test_results.capacity = arithmetics[i].addition_test_results.count = 0;
		arithmetics[i].subtraction_test_results.capacity = arithmetics[i].subtraction_test_results.count = 0;
		arithmetics[i].multiplication_test_results.capacity = arithmetics[i].multiplication_test_results.count = 0;

		arithmetics[i].division_test_results.items = NULL;
		arithmetics[i].addition_test_results.items = NULL;
		arithmetics[i].subtraction_test_results.items = NULL;
		arithmetics[i].multiplication_test_results.items = NULL;
	}
}

uint_fast32_t rand32(uint_fast64_t * const refState)
{
	/*
PCR random generator credit O'Neill, Melissa http://www.pcg-random.org/
*/
	static uint_fast64_t const multiplier = 6364136223846793005u;
	static uint_fast64_t const increment = 1442695040888963407u;	/* Or an arbitrary odd constant*/
	uint_fast64_t x;
	x = *refState;
	uint_fast8_t count = (uint_fast8_t)(x >> 59);

	*refState = x * multiplier + increment;
	x ^= x >> 18;
	x = x >> 27;
	return ((uint_fast32_t)x) >> count;
}

void randNum(uint_fast64_t * const refState, reg_t * const A, numsize_t size)
{	
	/*
	PCR random generator credit O'Neill, Melissa http://www.pcg-random.org/ 
	*/
	static uint_fast64_t const multiplier = 6364136223846793005u;
	static uint_fast64_t const increment = 1442695040888963407u;	/* Or an arbitrary odd constant*/

	numsize_t i;
	numsize_t sz = size * sizeof(reg_t) / sizeof(uint_fast32_t);
	uint_fast32_t * B = (uint_fast32_t *) A;
	uint_fast64_t x;
	uint_fast32_t y;
	uint_fast32_t z;

	for (i = 0; i < sz; i++)
	{
		x = *refState;
		uint_fast8_t count = (uint_fast8_t)(x >> 59);

		*refState = x * multiplier + increment;
		x ^= x >> 18;	
		x = x >> 27;
		y = ((uint_fast32_t)x) >> count;
		z = ((uint_fast32_t)x) << (32 - count);		

		B[i] = y | z;
	}
}

static void copystr(_char_t const * const src, _char_t* dest)
{
	numsize_t i = 0;
	for (i = 0; i < MAXSTRING; ++i)
	{
		dest[i] = src[i];
		if (dest[i] == 0)
			return;
	}
}

static void test_statistics_collection_ADD(test_statistics_collection * destination_array, test_statistics * result)
{
	void* temp;
	if (destination_array->capacity == destination_array->count)
	{
		destination_array->capacity += 10;
		temp = (test_statistics**)realloc(destination_array->items, destination_array->capacity * sizeof(test_statistics*));
		MY_ASSERT(temp, STR("CAN'T ALLOCATE MEMORY"));
		
		destination_array->items = temp;
	}
	
	destination_array->items[destination_array->count] = result;
	destination_array->count++;
}

static void test_statistics_collection_CLEAR(test_statistics_collection * array)
{
	for (int i = 0; i < array->count; ++i)
	{
		free(array->items[i]);
	}
	free(array->items);
	array->items = NULL;
	array->capacity =  array->count = 0;
}

void run_test_repeat(_result_t(*unit_test)(CLOCK_T * out_algorithmExecutionTiming, struct _operation_implementations*, void* userData),
	struct _operation_implementations* op,
	test_statistics_collection * destination_array,
	_char_t const * const test_description,
	void *  userData
	)
{

	test_statistics * result = (test_statistics*)malloc(sizeof(test_statistics));
	MY_ASSERT(result, NOMEM);

	copystr(test_description, result->test_description);
	


	int j;

	result->test_result = _OK;

	LOG_INFO(STR("%s on %s version..."), result->test_description, op->implementation_description);

	CLOCK_T overall = precise_clock();
	CLOCK_T cumulative = clock_zero();
	CLOCK_T delta = cumulative;
	CLOCK_T feedbacktimeout = precise_clock();


	for (j = 0; j < _ITERATIONS_FOR_RANDOM_TEST; ++j)
	{
		result->test_result = unit_test(&delta, op, userData);
		if (FAILED(result->test_result))
		{			
			break;
		}		

		cumulative += delta;
		//assert(delta >= 0);

		if (seconds_from_clock(precise_clock() - feedbacktimeout) > 5.0)
		{
			LOG_INFO(STR("\tITS A LONG OPERATION: %d/%d"), j, _ITERATIONS_FOR_RANDOM_TEST);
			feedbacktimeout = precise_clock();
		}
		else if (seconds_from_clock(precise_clock() - overall) > MAX_OUTER_TIME_FOR_TESTING_SEC)
		{
			LOG_INFO(STR("\tOPERATION TIMEOUT"));
			break;
		}
	}
	if (FAILED(result->test_result))
	{
		LOG_INFO(STR("...FAILED"));
	}
	else
	{
		LOG_INFO(STR("...PASSED"));
	}
	

	overall = precise_clock() - overall;

	result->outer_time_sec = seconds_from_clock(overall);
	result->inner_time_sec = seconds_from_clock(cumulative);
	result->number_of_iterations = j;

	if (result->inner_time_sec > 0)
		result->avg_operations_per_second = j / result->inner_time_sec;
	else
		result->avg_operations_per_second = 0;


	test_statistics_collection_ADD(destination_array, result);

	LOG_INFO(STR("cumulative inner time: %f sec"),  result->inner_time_sec);

}

void run_test_single(_result_t(*unit_test)(CLOCK_T * out_algorithmExecutionTiming, struct _operation_implementations*, void * userData),
	struct _operation_implementations* op,
	test_statistics_collection * destination_array,
	_char_t const * const test_description, void*userData)
{
	test_statistics * result = (test_statistics*)malloc(sizeof(test_statistics));
	MY_ASSERT(result, NOMEM);

	copystr(test_description, result->test_description);

	result->test_result = _OK;

	LOG_INFO(STR("%s on %s version"), result->test_description, op->implementation_description);

	CLOCK_T cumulative = clock_zero();
	CLOCK_T overall = precise_clock();
	
	result->test_result = unit_test(&cumulative, op, userData);
	if (FAILED(result->test_result))
	{
		LOG_INFO(STR("...FAILED"));	
		result->number_of_iterations = 0;
	}
	else
	{
		LOG_INFO(STR("...PASSED"));
		result->number_of_iterations = 1;
	}

	overall = precise_clock() - overall;
	result->outer_time_sec = seconds_from_clock(overall );
	result->inner_time_sec = seconds_from_clock(cumulative);
	
	if (result->inner_time_sec > 0)
		result->avg_operations_per_second = result->number_of_iterations / result->inner_time_sec;
	else
		result->avg_operations_per_second = 0;

	test_statistics_collection_ADD(destination_array, result);
}

static void _summary(
	_char_t const * const impl, 
	_char_t const * const func, 
	test_statistics_collection const *const op, 
	test_statistics_collection const *const referenceOp)
{
	int i;
	for (i = 0; i < op->count; ++i)
	{
		struct _test_statistics * item = op->items[i];
		struct _test_statistics * reference = referenceOp->items[i];
		double relative_inner_time = 0.0;
		double relative_outer_time = 0.0;

		if (reference->inner_time_sec != 0)
			relative_inner_time =  item->inner_time_sec / reference->inner_time_sec;
		if (reference->outer_time_sec != 0)
			relative_outer_time =  item->outer_time_sec / reference->outer_time_sec;

		_fprintf(stdout, STR("\"%s\"\t\"%s\"\t\"%s\"\t%e\t%e\t%e\t%e\t\"%s\"\t%e\t%e\n"),
			
			impl, func,  item->test_description , item->inner_time_sec, item->outer_time_sec,
			item->number_of_iterations, item->avg_operations_per_second, 			
			(OK(item->test_result) ? STR("ok") : STR("failed")),
			relative_inner_time, 
			relative_outer_time
			);
	}
}

void write_summary()
{
	int i;
	_fprintf(stdout, STR("\"Implementation\"\t\"Operation\"\t\"Test Description\"\t\"Inner Elapsed Seconds\"\t\"Outer Elapsed Seconds\"\t\"Number Of Iterations\"\t\"Average Op Per Second\"\t\"Result\"\t\"Relative inner time\"\t\"Relative outer time\"\n"));

	for (i = 0; i < number_of_arithmetics; ++i)
	{
		_char_t * impl = arithmetics[i].implementation_description;
		_summary(impl, STR("Addition"), &(arithmetics[i].addition_test_results), &(arithmetics[0].addition_test_results));
		_summary(impl,STR("Subtraction"), &(arithmetics[i].subtraction_test_results), &(arithmetics[0].subtraction_test_results));
		_summary(impl, STR("Multiplication"), &(arithmetics[i].multiplication_test_results), &(arithmetics[0].multiplication_test_results));
		_summary(impl, STR("Division"), &(arithmetics[i].division_test_results), &(arithmetics[0].division_test_results));
	}
}


void cleanup()
{
	int i;
	for (i = 0; i < number_of_arithmetics; ++i)
	{
		 test_statistics_collection_CLEAR(&(arithmetics[i].addition_test_results));
		 test_statistics_collection_CLEAR(&(arithmetics[i].subtraction_test_results));
		 test_statistics_collection_CLEAR(&(arithmetics[i].multiplication_test_results));
		 test_statistics_collection_CLEAR(&(arithmetics[i].division_test_results));
	}
}

#if defined(_WIN32) || defined(WIN32)
CLOCK_T precise_clock() {
	unsigned long long o;
	if (QueryPerformanceCounter((LARGE_INTEGER*) &o) != 0)
	{
		return o;
	}	
	return clock_zero();
}
double seconds_from_clock(CLOCK_T clock)
{
	unsigned long long o;
	if (QueryPerformanceFrequency(
		(LARGE_INTEGER*)&o
	) == 0)return 0.0;

	double d = (double)(clock) /(double)o;
	//assert(d > 0);
	return d;
}
CLOCK_T clock_zero()
{	
	return 0ULL;
}
#else
#warning maybe you want to define a better clock function
CLOCK_T precise_clock() {
	return clock();
}
double seconds_from_clock(CLOCK_T clock)
{
	return clock / CLOCKS_PER_SEC;
}
CLOCK_T clock_zero()
{
	return 0L;
}
#endif
