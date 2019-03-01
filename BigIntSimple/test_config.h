#include "test.h"


struct _operation_implementations
/* describe here your impl */
arithmetics[] = {
	{
		STR("C (Reference impl)"),
		LongSumWithCarryDetection,
		LongSub,
		LongMultiplication,
		LongDivision
	},
	{
		STR("Assembler (x64)"),
		LongSumAsm,
		LongSubAsm,
		LongMulAsm,
		NULL
	},
	{
		STR("Assembler (x64, variant 1)"),
		NULL,
		LongSubAsmVariant_1,
		NULL,
		NULL
	}
	/*this implementation is constantly slower than reference
	,
	{   
		
		STR("C (carry detection with branches)"),
		LongSumWithCarryDetectionV2,
		NULL,
		NULL,
		NULL
	},*/
	/* this implementation is constantly slower than reference
	{
		STR("C (using single byte carry)"),
		LongSumWithCarryDetectionV3,
		NULL,
		NULL,
		NULL
	}*/
};


int number_of_arithmetics = sizeof(arithmetics) / sizeof(struct _operation_implementations);
