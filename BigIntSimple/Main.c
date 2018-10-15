#include "BigIntSimple.h"
#include "test.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

int main()
{

	setlocale(LC_ALL, "");
	//test_commutative_prop_ams_single_norandom_unit();
	initTest();
	testBSR();
	testCompare();
	testSum();
	testSub();

	write_summary();

	return 0;
}