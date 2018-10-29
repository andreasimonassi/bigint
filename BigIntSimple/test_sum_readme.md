# Implementation notes on test_sum.c
## Synopsis
The tests are a list of function that are execute to measure either correctness and speed (when it make sense to measure speed).

The entry point is the **testSum** function, to add tests add the call to your test to the **testSum** function.

In order to produce meaningful statistics you need to wrap the call to your test function into a call to **each_op** and pass your test function as a callback.

## How to add a test function

The signature of the **each_op** function is 

```c
static void each_op(
    _result_t(*unit_test)(CLOCK_T* outElapsedTime, struct _operation_implementations*), 
    int boolRepeat,
	_char_t const * const test_description
	)
	
```
the test function must return a **\_result\_t** , defined in **test.h** header file.

```c
/* make clear what is the result type of a function */
#define _OK 0
#define _FAIL 1
#define OK(x) (x==0)
#define FAILED(x) (x!=0)
typedef int _result_t;
```
Callback parameters are:
- outElapsedTime: test function compute the time spent in inner computation, you should only count the time spent by the _sum function to complete, not the time used to initialize numbers nor the time used to allocate memory, to print information and so on, **your test function only have to count time spent doing the SUM**
- _operation_implementations*: a pointer to the sum function that the test must call

Example implementation of a test function that does check for commutative property:

```c
static _result_t  test_commutative_prop_unit(CLOCK_T* delta_t, struct _operation_implementations* impl)
{
	_result_t result = _OK;
	
	/* Initialize numbers used by the test, maybe you can use random numbers or
	   use well known numbers
	*/
	your_initialization_code_here();
	
	/*
	measure inner time took by the sum function (don't measure time used for initialization and 
	don't measure time spent by check outcome
	*/
	
	*delta_t = precise_clock();
	R1Len = impl->addition(A, ASize, B, BSize, R1); 
	R2Len = impl->addition(B, BSize, A, ASize, R2);
	*delta_t = precise_clock() - *delta_t;
	
	/*check the result*/
	if(your_post_condition_are_not_met())
	    result = _FAILED;
	
	/*eventually dump numbers that caused the test to fail so you can revise 
	the implementation of the sum for that particular numbers */
	if (FAILED(result))
	{
		dumpNumber(R1, STR( "ActualResultOfA_plus_B"), R1Len);
		dumpNumber(R2, STR("ActualResultOfB_plus_A"), R2Len);
		dumpNumber(A, STR("A"), ASize);
		dumpNumber(B, STR("B"), BSize);
		LOG_INFO(STR("Commutative prop test failed, see dump"));
	}
	return result;
}
```

Note that the **each_op** function will collect all the tests results and timing for you, will also call your test function for each possible implementation, will also call your function as many time as configured by the define

```c
#define TEST_ITERATIONS 100000
```

## A note about random numbers
We do not want numbers to be too much "random" so the **each_op** function will reseed the pseudo random generator at each call, so that each implementation is called always with the same set of random numbers, this way the tests will be comparable.

for example the commutative property check function generate pseudo random values that are predictable in this way:
```c
    /* 
    do not call srand, srand has been called automatically by each_op,
    the rand will return always the same numbers for all the implementations its 
    value is still random, increasing number of iterations you will hit different numbers
    */
	reg_t ASize = rand() % TEST_NUMBER_WORDS;
	reg_t BSize = rand() % TEST_NUMBER_WORDS;
	
	reg_t R1Len;
	reg_t R2Len;

    /*
    the randNum will be called always using the _rand_seed in & out parameter
    so that each implementation will be tested with the same set of numbers giving
    reproducible results, the _rand_seed has been initialized by each_op, the
    randNum function uses a pseudo random generator that I hope is correct
    and have good statistical properties
    */
	randNum(&_rand_seed, A, ASize);
	randNum(&_rand_seed, B, BSize);
```

## Make a test run once or run many times
Second parameter of each op is an int value, when it is 0 then the test is run once (for example a test using well known numbers or a very heavy test), otherwise the test is run in a loop (for example when you run tests on random numbers) 