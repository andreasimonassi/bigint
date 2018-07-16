#include "BigIntSimple.h"
#include "test.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>


int main()
{


	//test_commutative_prop_ams_single_norandom_unit();
	initTest();
	testBSR();
	testCompare();
	testSum();
	testSub();

	return 0;
}