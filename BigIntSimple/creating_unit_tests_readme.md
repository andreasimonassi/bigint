# How to add a unit test
## Synopsis
The tests are a list of function that are execute to measure either correctness and speed (when it make sense to measure speed).

The entry point are the **testSum**, **testSub**, **testMul** and **testDiv** function that you find on source files 
**test_sum.c**, **test_sub.c**, **test_mul.c" and **test_div.c** respectively.

To add tests add the call to your test to the **testXXX** function on the appropriate source file.

In order to produce meaningful statistics you need to wrap the call to your test function into a call to 
**each_op** and pass your test function as a callback.

## How to add a test function

Each of the 4 modules **test_sum.c**, **test_sub.c**, **test_mul.c" and **test_div.c** have their own definition of the **each_op** function but
all the modules have the same function signature

```c
static void each_op(_result_t(*unit_test)(CLOCK_T* outAlgorithmElapsedTime, _operationdescriptor* op, void* userData) callback, int boolRepeat,
	_char_t const* const test_description, void* userData, unsigned repeat, double operand1_size, double operand2_size)
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
Your unit test will be called then by each_op, which is purpose is then to collect and store statistical information on your test.

Your unit test signature must be then the following (of course)

```c
_result_t MY_UNIT_TEST(CLOCK_T* outAlgorithmElapsedTime, _operationdescriptor* op, void* userData);
```

- outElapsedTime: test function compute the time spent in inner computation, you should only count the time spent by 
the algorithm you are trying to measure, not the time used to initialize numbers nor the time used to allocate memory,
nor the time spent to print information and so on, **your test function only have to count time spent doing the 
arithmetic operation** (see example below). This measurement could be something you don't care to collect for your test
in such case you just set its value to 0
- _operationdescriptor*: a pointer to the operation implementations to be called, this 

Example implementation of a test function :

```c
static _result_t  template_of_a_unittest(CLOCK_T* delta_t, _operationdescriptor* impl, void* userData)
{
	/* INITIALIZE RETURN VALUE */
	_result_t retval = _OK;
	/* INITIALIZE THE TEST, ALLOCATE STUFF, WHATEVER... DO NOT MEASURE THIS TIME */

	long_init_subrouting();

	/* HERE YOU START TO MEASURE TIME*/
	
	*delta_t = precise_clock();

	/* THE SIGNATURE FOR DIVISION IS SLIGTHLY DIFFERENT, CHECK THE SOURCE CODE */
	/* NOW YOU RUN THE ALGORITHM */
	length = impl->operation.operation(operand1, operand1Size, operand2, operand2size, result);

	/* HERE YOU STOP MEASURING THE TIME AND SET THE OUTPUT PARAMETER VALUE */
	*delta_t = precise_clock() - *delta_t;

	/* THEN CHECK THE RESULT*/
	if (result_is_not_as_expected)
	{
		retval = _FAIL;
	}	
	
	/* Cleanup EVENTUAL MEMORY, ETC*/

	/* IN case of ERROR you might want to print to STDERR some diagnostics */
	if (FAILED(retval))
	{
		dumpNumber(operand1, STR("operand1"), operand1Size);
		dumpNumber(operand2, STR("operand2"), operand2size);
		dumpNumber(result, STR("Actual"), length);
		dumpNumber(expected, STR("Expected"), expectedlenght);
		LOG_INFO(STR("xxx test failed, see dump"));
	}	
	return retval;
}
```

Note that the **each_op** function will collect all the tests results and timing for you, will also call your test function for each possible implementation, will also call your function as many time as configured 
by the parameter repeat

After you create your unit test you want it to be called.

In order for this to happen you must add your unit test to the appropriate test_xxx.c module , for example in test_sum.c add into the testSum() function
the call to your unit test wrapped into each_op.

the init_test() that you see is doing initialization for some specific tests.

You can add your test in any position of the body of the function

```c
void testSum()
{	
	init_test();	
	
	each_op(test_zero_is_neutral_element_of_sum, 1, STR("Testing zero should be neutral element of sum"), NULL, REPEAT_LONG, 0, 0);
	each_op(test_on_1000_unit, 1, STR("Testing that numbers like ff,fe,ff + 1,1 = 1,0,0,0"), NULL, REPEAT_LONG,0,0);
	each_op(test_speed_1_MB_unit, 1, STR("Testing 1MB (512KB+512KB) of data segment allocated numbers"), NULL, REPEAT_LONG, HALF_MEG_NUMBER, HALF_MEG_NUMBER );
	each_op(test_speed_512KB_Plus_2Words_unit, 1, STR("Testing 512KB+2words of data segment allocated numbers"), NULL, REPEAT_LONG, HALF_MEG_NUMBER,2);
	each_op(test_commutative_prop_unit, 1, STR("Test Commutative Property"), NULL, REPEAT_LONG,0,0);
	each_op(test_associative_prop_unit, 1, STR("Test Associative Property"), NULL, REPEAT_LONG,0,0);
	each_op(test_last_carry, 0, STR("Testing last carry (1 + allonebits equals to 1 followed by all zero bits)"), NULL, REPEAT_LONG,0,0);

	/* adding your test */
	each_op(
		my_new_sum_test, /* YOUR FUNCTION */
		0, /* 0 FOR EXECUTE ONCE PER IMPLEMENTATION, 1 TO EXECUTE MANY TIMES PER IMPLEMENTATION (this is because of older code, you can set always to 1)*/
		STR("A description for the test that will appears on statistics"),		/* description*/
		NULL, /* any user data you need to pass to the test */		
		REPEAT_LONG, /* number of repetition, if it is a random test you may want to have a high number of repetition, if you are collectig performance data you may want lower repetitions , if it is a one shot test you set this to 1*/
		0, /* size of operand1 , used only to be added to statistics output, useful to have on statistics output to compare operations for different operator sizes */
		0 /* size of operand2, see above */
		);
	
}
```

## Memory segment allocated bytes.

** PLEASE NOTE THAT YOU DO NOT NEED TO USE THOSE ARRAYS FOR YOUR TEST, THE RELEVANT PART TO CREATE YOUR TEST IS ABOVE, but if you want to reuse those arrays to reduce overhead here some docs: **

On all the modules test_xxx.c, there are preallocated numbers you can use for your tests.

if it is not clear how to use them, i suggest you to allocate other numbers.

Here is a list of array you can use to store data for your own tests, if you want.

### On module test_sum.c

You can reuse those.

```c
static reg_t _HALF_MEG_A[HALF_MEG_NUMBER]; /*unit of HALF_MEG_NUMBER is sizeof(reg_t) so this array will be about 512KB on size*/
static reg_t _HALF_MEG_B[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_RESULT[HALF_MEG_NUMBER + 1];

static reg_t _TWO_WORDS_A[2];
static reg_t _TWO_WORDS_B[2];
```

if you need them to be initialized randomly (consider they are 512KB arrays, so don't exceed)
```c
randNum(&_rand_seed, A, HALF_MEG_NUMBER); /* random initialize A, the seed is reset by each_op for each algorithm , so all algorithm will work on the same set of data*/
randNum(&_rand_seed, B, HALF_MEG_NUMBER);
```

### On module test_sub.c

```c

static reg_t _A[HALF_MEG_NUMBER];
static reg_t _B[HALF_MEG_NUMBER];
static reg_t _C[HALF_MEG_NUMBER];
static reg_t _R1[HALF_MEG_NUMBER];
static reg_t _R2[HALF_MEG_NUMBER];
static reg_t _R3[HALF_MEG_NUMBER];

static reg_t _HALF_MEG_A[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_B[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_RESULT[HALF_MEG_NUMBER + 1];

static reg_t _TWO_WORDS_A[2];
static reg_t _TWO_WORDS_B[2];
```

### On module test_mul.c

Here we have thast result arrays are 1 MB on size because multiplying 2 512KB numbers produce a 1 MB result

```c
#define HALF_MEG_NUMBER (1024*1024/sizeof(reg_t)/2)
#define BIG_NUMBER (1024*1024/sizeof(reg_t))
#define MBIG_NUMBER (160*1024/sizeof(reg_t))
static reg_t _HALF_MEG_mA[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_mB[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_mC[HALF_MEG_NUMBER];
static reg_t _MEG_mRESULT1[BIG_NUMBER];
static reg_t _MEG_mRESULT2[BIG_NUMBER];
static reg_t _MEG_mRESULT3[BIG_NUMBER];
static reg_t _MEG_mRESULT4[BIG_NUMBER];
```

if we need to create some random numbers for our test we can do as this
```c
randNum(
&_rand_seed_3, /* the global seed, reinitialized automatically by each_op */
_HALF_MEG_mA,  /* the target array to be filled by random digits */
ASize          /* the required size (wich must be less than HALF_MEG_NUMBER of course)*/
);    
```

### On module test_div.c



```c
#define HALF_MEG_NUMBER (1024*1024/sizeof(reg_t)/2)
#define BIG_NUMBER (1024*1024/sizeof(reg_t))
#define MBIG_NUMBER (256*1024/sizeof(reg_t))
static reg_t _HALF_MEG_dA[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_dB[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_dC[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_dD[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_dE[HALF_MEG_NUMBER];
static reg_t _HALF_MEG_dF[HALF_MEG_NUMBER];
```

if we need to create some random numbers for our test we can do as this
```c
randNum(&_rand_seed_4, _HALF_MEG_dA, ASize);   
```

## A note about random numbers
We do not want numbers to be too much "random", this is the reason why the **each_op** function will reseed the pseudo random generator
to a constant value at each call, so that each implementation is called always with the same set of random numbers,
this way the tests will be reproducible.

the randNum produce pseudo random numbers with (hopefully) hig entropy, so if you need lot of zeroes in your random number create your 
own randomize function, but please make it in a way that results are reproducible (either use the _rand_seed as seed or create 
your own seed and reset it each time the each_op is called)

on the implementation we don't need high quality random numbers, we need to create them fast, so we use a simple multiply and increment modular sequence see source code for credits.

## Make a test run once or run many times
Second parameter of each op is an int value, when it is 0 then the test is run once 
(for example a test using well known numbers or a very heavy test), otherwise the test is run in a loop
(for example when you run tests on random numbers) , this is a reminescence of a past implementation (I used a constant number of repetitions for all tests in a module),
nowadays you can control the number of iterations by setting the second parameter always to 1 and to set the repetition count instead