#include "test.h"

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
	int charsize = ASize * sizeof(reg_t) * 2 + 1;
	int memsize = charsize * sizeof(_char_t);

	_char_t * buffer = (_char_t*)malloc(memsize);

	if (buffer == NULL)
	{
		_fprintf(stderr, STR("[ERROR] CANNOT DUMP NUMBER, NO MEMORY FOR MALLOC"));
		return;
	}

	FillHexString(buffer, charsize, A, ASize);
	_fprintf(stderr, STR("\n%s = \""), name);

	_fprintf(stderr, buffer);

	_fprintf(stderr, STR("\";\n"));
	free(buffer);
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

void run_test_repeat(_result_t(*unit_test)(CLOCK_T* out_algorithmExecutionTiming, _operationdescriptor*, void* userData),
	_operationdescriptor* descriptor,
	_char_t const* const test_description,
	void* userData,
	unsigned int repeatCount, double operand1_size, double operand2_size
)
{

	test_statistics * result = (test_statistics*)malloc(sizeof(test_statistics));
	MY_ASSERT(result, NOMEM);

	copystr(test_description, result->test_description);
	


	unsigned int j;

	result->test_result = _OK;

	LOG_INFO(STR("%s on %s, operand sizes are [%.0f] and [%.0f], repeat [%d] times ..."), 
		test_description, 
		descriptor->implementation_description,
		operand1_size,
		operand2_size,
		repeatCount);

	CLOCK_T overall = precise_clock();
	CLOCK_T cumulative = clock_zero();
	CLOCK_T delta = cumulative;
	CLOCK_T feedbacktimeout = precise_clock();


	for (j = 0; j < repeatCount; ++j)
	{
		result->test_result = unit_test(&delta, descriptor, userData);
		if (FAILED(result->test_result))
		{			
			break;
		}		

		cumulative += delta;
		//assert(delta >= 0);

		if (seconds_from_clock(precise_clock() - feedbacktimeout) > 5.0)
		{
			LOG_INFO(STR("\tIT'S A LONG OPERATION: %d/%d"), j, repeatCount);
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
	result->operand1_size = operand1_size;
	result->operand2_size = operand2_size;

	if (result->inner_time_sec > 0)
		result->avg_operations_per_second = j / result->inner_time_sec;
	else
		result->avg_operations_per_second = 0;


	test_statistics_collection_ADD(&(descriptor->results), result);

	LOG_INFO(STR("cumulative inner time: %f sec"),  result->inner_time_sec);

}

void run_test_single(_result_t(*unit_test)(CLOCK_T * out_algorithmExecutionTiming, _operationdescriptor*, void * userData),
	_operationdescriptor* op,
	_char_t const * const test_description, void*userData, double operand1_size, double operand2_size)
{
	test_statistics * result = (test_statistics*)malloc(sizeof(test_statistics));
	MY_ASSERT(result, NOMEM);

	copystr(test_description, result->test_description);

	result->test_result = _OK;

	LOG_INFO(STR("%s on %s version"), test_description, op->implementation_description);

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
	result->operand1_size = operand1_size;
	result->operand2_size = operand2_size;
	result->outer_time_sec = seconds_from_clock(overall );
	result->inner_time_sec = seconds_from_clock(cumulative);
	
	
	if (result->inner_time_sec > 0)
		result->avg_operations_per_second = result->number_of_iterations / result->inner_time_sec;
	else
		result->avg_operations_per_second = 0;

	test_statistics_collection_ADD(&(op->results), result);
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

		_fprintf(stdout, STR("\"%s\"\t\"%s\"\t\"%s\"\t%E\t%E\t%E\t%E\t\"%s\"\t%E\t%E\t%E\t%E\n"),
			
			impl, func,  item->test_description , item->inner_time_sec, item->outer_time_sec,
			item->number_of_iterations, item->avg_operations_per_second, 			
			(OK(item->test_result) ? STR("ok") : STR("failed")),
			relative_inner_time, 
			relative_outer_time, item->operand1_size, item->operand2_size
			);
	}
}

void write_opsummary(_char_t * operatordescr, _operationdescriptor* descriptor, size_t count)
{
	_operationdescriptor* iterator = descriptor;
	int i ;
	for(i=0; i< count;++i)
	{
		_summary(iterator->implementation_description, operatordescr, &(iterator->results), &(descriptor->results));
		iterator++;
	}
}

void write_summary()
{
	_fprintf(stdout, STR("\"Implementation\"\t\"Operation\"\t\"Test Description\"\t\"Inner Elapsed Seconds\"\t\"Outer Elapsed Seconds\"\t\"Number Of Iterations\"\t\"Average Op Per Second\"\t\"Result\"\t\"Relative inner time\"\t\"Relative outer time\"\t\"Operand1 Size\"\t\"Operand2 Size\"\n"));

	write_opsummary(STR("Addition"),arithmetic->sum, arithmetic->sumcount);
	write_opsummary(STR("Subtraction"), arithmetic->subtract, arithmetic->subtractcount);
	write_opsummary(STR("Multiplication"), arithmetic->multiply, arithmetic->multiplycount);
	write_opsummary(STR("Division"), arithmetic->divide, arithmetic->dividecount);
	
}


void cleanup_op(_operationdescriptor* descriptor, size_t count) {
	_operationdescriptor* iterator = descriptor;
	int i;
	for(i=0;i<count;++i)
	{
		test_statistics_collection_CLEAR(&(descriptor->results));
		iterator++;
	}
}

void cleanup()
{
	cleanup_op(arithmetic->sum, arithmetic->sumcount);
	cleanup_op(arithmetic->subtract, arithmetic->subtractcount);
	cleanup_op(arithmetic->multiply, arithmetic->multiplycount);
	cleanup_op(arithmetic->divide, arithmetic->dividecount);
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
