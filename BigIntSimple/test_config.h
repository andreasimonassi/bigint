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
	/*{
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
	},*/
	/*{
		STR("C variant 1"),
		NULL,
		NULL,
		LongMultiplication,
		LongDivisionReadable
	},*/
	{   
		STR("C (Karatsuba mult v1 (threshold 8)"),
		NULL,
		NULL,
		KaratsubaMultiplicationUsingPortablePrimitive8,
		NULL
	},
	{
		STR("C (Karatsuba mult v1 (threshold 12)"),
		NULL,
		NULL,
		KaratsubaMultiplicationUsingPortablePrimitive12,
		NULL
	},
	{
		STR("C (Karatsuba mult v1 (threshold 16)"),
		NULL,
		NULL,
		KaratsubaMultiplicationUsingPortablePrimitive16,
		NULL
	},
	{
		STR("C (Karatsuba mult v1 (threshold 24)"),
		NULL,
		NULL,
		KaratsubaMultiplicationUsingPortablePrimitive24,
		NULL
	}
	,
	{
		STR("C (Karatsuba mult v1 (threshold 32)"),
		NULL,
		NULL,
		KaratsubaMultiplicationUsingPortablePrimitive32,
		NULL
	}
	/*, this implementation is constantly slower than reference
	{
		STR("C (using single byte carry)"),
		LongSumWithCarryDetectionV3,
		NULL,
		NULL,
		NULL
	}*/
};


int number_of_arithmetics = sizeof(arithmetics) / sizeof(struct _operation_implementations);
