#include "test.h"

void testBSR()
{
	_fprintf(stderr,STR("Testing BitScanReverse function "));
	reg_t response = BitScanReverse(0x1);
	MY_ASSERT(response == 0, STR("Expected 0"));
	response = BitScanReverse(0x2);
	MY_ASSERT(response == 1,  STR("Expected 1"));
	response = BitScanReverse(0x4);
	MY_ASSERT(response == 2,  STR("Expected 2"));
	response = BitScanReverse(0x40);
	MY_ASSERT(response == 6,  STR("Expected 6"));
	response = BitScanReverse((reg_t)0x1 << (((reg_t)sizeof(reg_t) * (reg_t)8) - (reg_t)2));
	MY_ASSERT(response == (sizeof(reg_t) * 8) -2,  STR("expected other value!!"));
	_fprintf(stderr,STR(" PASSED\n"));
}