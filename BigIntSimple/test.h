#include "BigIntSimple.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#define STR(x) #x
#define MY_ASSERT(x) if (!(x)) { printf("My custom assertion failed: (%s), function %s, file %s, line %d.\n", STR(x), __FUNCTION__, __FILE__, __LINE__); abort(); }


/* must be freed by caller*/
reg_t * randNum();

void repeat(int N, void(*func)());

void dumpNumber(reg_t * A, reg_t ASize);
void test_commutative_prop_ams_single_norandom_unit();

void initTest();
void testCompare();
void testSum();
void testSub();
void testBSR();

