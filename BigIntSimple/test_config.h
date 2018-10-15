#include "test.h"


struct _operation_implementations
/* describe here your impl */
arithmetics[] = {
	{
		STR("C Version (carry detection)"),
		LongSum,
		LongSub,
		NULL,
		NULL
	},
	{
		STR("Assemly version"),
		LongSumAsm,
		NULL,
		NULL,
		NULL
	},
};


int number_of_arithmetics = sizeof(arithmetics) / sizeof(struct _operation_implementations);