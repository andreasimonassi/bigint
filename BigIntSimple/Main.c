#include "BigIntSimple.h"
#include "test.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

int main()
{

	setlocale(LC_ALL, "");

	initTest();
	/*
	testBSR();
	testCompare();
	testSum();
	testSub();*/
	testMul();

	write_summary();

	return 0;
}