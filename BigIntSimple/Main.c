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
	
	
	testParse();
	testBSR();
	testCompare();
	testSum();
	/*testSub();	
	testMul();		
	testDiv();*/
	

	write_summary();

	return 0;
}