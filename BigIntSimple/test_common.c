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
	//remove leading zeroes
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
void dumpNumber(reg_t * A, _char_t* name,  reg_t ASize)
{
	_char_t buffer[sizeof(reg_t) * 2 + 1];
	_fprintf(stderr, STR("\nreg_t %s [] = {\n"), name);
	for (reg_t i = 0; i < ASize; ++i)
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

reg_t * randNum(reg_t size)
{
	size *= sizeof(reg_t);
	reg_t i;
	unsigned char * B;
	B = malloc(size);
		
	MY_ASSERT(B, NOMEM);
		
	for (i = 0; i < size; i++)
	{
		B[i] = (unsigned char) rand();
	}

	return (reg_t*)B;
}

static void copystr(_char_t const * const src, _char_t* dest)
{
	int i = 0;
	for (i = 0; i < MAXSTRING; ++i)
	{
		dest[i] = src[i];
		if (dest[i] == 0)
			return;
	}
}

static void test_statistics_collection_ADD(test_statistics_collection * destination_array, test_statistics * result)
{
	if (destination_array->capacity == destination_array->count)
	{
		destination_array->capacity += 10;
		destination_array->items = (test_statistics**)realloc(destination_array->items, destination_array->capacity * sizeof(test_statistics*));
		MY_ASSERT(destination_array->items, STR("CAN'T ALLOCATE MEMORY"));
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

void run_test_repeat(_result_t(*unit_test)(CLOCK_T * out_algorithmExecutionTiming, struct _operation_implementations*),
	struct _operation_implementations* op,
	test_statistics_collection * destination_array,
	_char_t const * const test_description
	)
{

	test_statistics * result = (test_statistics*)malloc(sizeof(test_statistics));
	MY_ASSERT(result, NOMEM);

	copystr(test_description, result->test_description);
	


	int j;

	result->test_result = _OK;

	LOG_INFO(STR("%s on %s version"), result->test_description, op->implementation_description);

	CLOCK_T cumulative = clock_zero();
	CLOCK_T delta = cumulative;


	for (j = 0; j < _ITERATIONS_FOR_RANDOM_TEXT; ++j)
	{
		result->test_result = unit_test(&delta, op);
		if (FAILED(result->test_result))
		{
			LOG_ERROR(STR("%s failed on %s"), result->test_description, op->implementation_description);
			break;
		}
		cumulative += delta;
	}

	result->absolute_time_sec = seconds_from_clock(cumulative);
	result->number_of_iterations = j;

	if (result->absolute_time_sec > 0)
		result->avg_operations_per_second = j / result->absolute_time_sec;
	else
		result->avg_operations_per_second = 0;

	test_statistics_collection_ADD(destination_array, result);

}

void run_test_single(_result_t(*unit_test)(CLOCK_T * out_algorithmExecutionTiming, struct _operation_implementations*),
	struct _operation_implementations* op,
	test_statistics_collection * destination_array,
	_char_t const * const test_description)
{
	test_statistics * result = (test_statistics*)malloc(sizeof(test_statistics));
	MY_ASSERT(result, NOMEM);

	copystr(test_description, result->test_description);

	result->test_result = _OK;

	LOG_INFO(STR("%s on %s version"), result->test_description, op->implementation_description);

	CLOCK_T cumulative = clock_zero();
	
	

	
		result->test_result = unit_test(&cumulative, op);
		if (FAILED(result->test_result))
		{
			LOG_ERROR(STR("%s failed on %s"), result->test_description, op->implementation_description);
			result->number_of_iterations = 0;
		}
		else
			result->number_of_iterations = 1;
	

	result->absolute_time_sec = seconds_from_clock(cumulative);
	
	if (result->absolute_time_sec > 0)
		result->avg_operations_per_second = result->number_of_iterations / result->absolute_time_sec;
	else
		result->avg_operations_per_second = 0;

	test_statistics_collection_ADD(destination_array, result);
}

static void _summary(_char_t const* const impl,  _char_t const * const func, test_statistics_collection const *const op)
{
	int i;
	for (i = 0; i < op->count; ++i)
	{
		struct _test_statistics * item = op->items[i];
		_fprintf(stdout, STR("\"%s\"\t\"%s\"\t\"%s\"\t%e\t%e\t%e\t\"%s\"\n"),
			
			impl, func,  item->test_description , item->absolute_time_sec, item->number_of_iterations, item->avg_operations_per_second, 
			
			(OK(item->test_result) ? STR("ok") : STR("failed"))
			);
	}
}

void write_summary()
{
	int i;
	_fprintf(stdout, STR("\"Implementation\"\t\"Operation\"\t\"Test Description\"\t\"Elapsed Seconds\"\t\"Number Of Iterations\"\t\"Average Op Per Second\"\t\"Result\"\n"));

	for (i = 0; i < number_of_arithmetics; ++i)
	{
		_char_t * impl = arithmetics[i].implementation_description;
		_summary(impl, STR("Addition"), &(arithmetics[i].addition_test_results));
		_summary(impl,STR("Subtraction"), &(arithmetics[i].subtraction_test_results));
		_summary(impl, STR("Multiplication"), &(arithmetics[i].multiplication_test_results));
		_summary(impl, STR("Division"), &(arithmetics[i].division_test_results));
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
	LARGE_INTEGER o;
	if (QueryPerformanceCounter(&o) != 0)
	{
		return o.QuadPart;
	}	
	return clock_zero();
}
double seconds_from_clock(CLOCK_T clock)
{
	LARGE_INTEGER o;
	if (QueryPerformanceFrequency(
		&o
	) == 0)return 0.0;

	return (double)clock /(double) o.QuadPart;
}
CLOCK_T clock_zero()
{	
	return 0L;
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
