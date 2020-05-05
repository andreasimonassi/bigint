#include "test.h"



static _operationdescriptor sums [] =
{
	{
		STR("C ref"), /* name of implementation*/
		OP, /* op for sum, mul and sub, DIVOP for div */
		LongSumWithCarryDetection, /* pointer to implementation*/
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("Asm v1"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		LongSumAsm,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	} };

static _operationdescriptor subtractions [] =
{
	{
		STR("C ref"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		LongSub,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("Asm v1"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		LongSubAsm,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("Asm v2"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		LongSubAsmVariant_1,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	}
};

static _operationdescriptor multiplications [] =
{
	{
		STR("C ref"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		LongMultiplicationV2,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("Asm v1"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		LongMulAsm,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("Asm v2"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		LongMulAsmVariant_1,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("C portable"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		LongMultiplicationNoAssembly,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("C portable V2"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		LongMultiplicationNoAssemblyV2,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("Karatsuba T1"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		KaratsubaMultiplicationUsingPortablePrimitive1,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("Karatsuba T8"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		KaratsubaMultiplicationUsingPortablePrimitive8,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("Karatsuba T12"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		KaratsubaMultiplicationUsingPortablePrimitive12,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("Karatsuba T16"),
		OP, /* op for sum, mul and sub, DIVOP for div */
		KaratsubaMultiplicationUsingPortablePrimitive16,
		{0,0,NULL} /* statistics (0 initial size, 0 count, NULL pointer) */
	},
	{
		STR("Karatsuba/Asm T12"),
		OP,
		KaratsubaMultiplicationUsingAssembly12,
		{0,0,NULL}
	}
};

static _operationdescriptor divisions []=
{
	{
		STR("C ref"),
		DIVOP , /* op for sum, mul and sub, DIVOP for div */
		(operation)LongDivision, /* using cast otherwise must initialize the struct using longer syntax*/
		{0,0,NULL}
	},
	{
		STR("C v1"),
		DIVOP , /* op for sum, mul and sub, DIVOP for div */
		(operation)LongDivisionReadable, /* using cast otherwise must initialize the struct using longer syntax*/
		{0,0,NULL}
	}
};

_arithmetic arithmetics = {
	sums, subtractions, multiplications, divisions,
	sizeof(sums) / sizeof(_operationdescriptor),
	sizeof(subtractions) / sizeof(_operationdescriptor),
	sizeof(multiplications) / sizeof(_operationdescriptor),
	sizeof(divisions) / sizeof(_operationdescriptor)
};
_arithmetic* arithmetic = &arithmetics;
/* describe here your impl */

void initTest()
{
	srand((unsigned int)time(NULL));	
	


}

	
		
	