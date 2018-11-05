#include "test.h"


struct _operation_implementations
/* describe here your impl */
arithmetics[] = {
	{
		STR("C (Reference impl)"),
		LongSumWithCarryDetection,
		LongSub,
		LongMultiplication,
		NULL
	},
	{
		STR("Assembler (x64)"),
		LongSumAsm,
		LongSubAsm,
		NULL,
		NULL
	},
	{
		STR("Assembler (x64, variant 1)"),
		NULL,
		LongSubAsmVariant_1,
		NULL,
		NULL
	},
};


int number_of_arithmetics = sizeof(arithmetics) / sizeof(struct _operation_implementations);
