#include "test.h"


struct _operation_implementations
/* describe here your impl */
arithmetics[] = {
	{
		STR("C (Reference impl)"),
		LongSumWithCarryDetection,
		LongSub,
		LongMultiplicationV2,
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
		LongMulAsmVariant_1,
		NULL
	},
	{
		STR("Multiplication no assembly version 1"),
		NULL,
		NULL,
		LongMultiplicationNoAssembly,
		NULL
	},	
	{
		STR("Multiplication no assembly version 2"),
		NULL,
		NULL,
		LongMultiplicationNoAssemblyV2,
		NULL
	},
	{
		STR("C variant 1"),
		NULL,
		NULL,
		LongMultiplication,
		LongDivisionReadable
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
