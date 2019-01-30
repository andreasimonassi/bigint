#include "test.h"


void parseHexTest(wchar_t const * const text, reg_t * expected, numsize_t expectedSize)
{
	_result_t result;
	reg_t * A = AllocNumOrFail(expectedSize);
	numsize_t ASize;
	result = parseFromHex(text, A, expectedSize, &ASize);
	if (result != _OK )
	{
		LOG_ERROR(STR("HEX PARSE: %s FAILED!"), text);
		free(A);
		exit(1);
	}
	if(ASize != expectedSize || CompareWithPossibleLeadingZeroes(A, ASize, expected, expectedSize) != 0)
	{
		LOG_ERROR(STR("HEX PARSE: %s FAILED SEE DUMP"), text);
		dumpNumber(A, L"Actual", ASize);
		dumpNumber(expected, L"Expected", expectedSize);
		free(A);
		exit(1);
	}
	free(A);
	LOG_INFO(STR("HEX PARSE: %s PASSED"), text);
}


void parseHexAndBack()
{
	_result_t result;
	reg_t A [10];
	numsize_t ASize;
	wchar_t buffer[500];
	wchar_t *_text = L"123456789012345678901234567890";
	result = parseFromHex(_text, A, 10, &ASize);
	if (result != _OK)
	{
		LOG_ERROR(STR("HEX PARSE and BACK 123456789012345678901234567890 FAILED!"));	
		exit(1);
	}
	result = FillHexString(buffer, 500, A, ASize);
	if (result != _OK)
	{
		LOG_ERROR(STR("HEX PARSE and BACK: cant'write to buffer"));

		exit(1);
	}
	if ( wcscmp(buffer, _text) != 0)
	{
		LOG_ERROR(STR("HEX PARSE and BACK: %s != %s"), _text, buffer);
		
		exit(1);
	}
	LOG_INFO(STR("HEX PARSE and BACK: %s PASSED"), _text);

}



void fillHexStringFailWhenBufferSmall()
{
	_result_t result;
	reg_t A[10];
	numsize_t ASize;
	wchar_t buffer[30];
	wchar_t *txt = L"123456789012345678901234567890";
	result = parseFromHex(txt, A, 10, &ASize);
	if (result != _OK)
	{
		LOG_ERROR(STR("HEX PARSE and BACK Must fail: 123456789012345678901234567890 FAILED!"));
		exit(1);
	}
	result = FillHexString(buffer, 30, A, ASize);
	if (result == _OK)
	{
		LOG_ERROR(STR("HEX PARSE and BACK Must Fail: should have failed cause buffer size is 30 but string buffer should be 31"));

		exit(1);
	}
	
	LOG_INFO(STR("HEX PARSE and BACK Must Fail: %s PASSED"), txt);

}

void parseVarious()
{
	reg_t  a[] = { 0x1ffd };
	parseHexTest(L"1FFD", a, 1);

#pragma warning(disable:4127) 
	if (sizeof(reg_t) == 8)
#pragma warning(default:4127) 
	{
		reg_t b[] = { 0x1ffd, 0x3333 };
		parseHexTest(L"33330000000000001FFD", b, 2);
		reg_t c[] = { 0x1ffd, 0x333 };
		parseHexTest(L"3330000000000001FFD", c, 2);
	}
#pragma warning(disable:4127) 
	else if (sizeof(reg_t) == 4)
#pragma warning(default:4127) 
	{
		reg_t b[] = { 0x1ffd, 0x3333 };
		parseHexTest(L"333300001FFD", b, 2);
		reg_t c[] = { 0x1ffd, 0x333 };
		parseHexTest(L"33300001FFD", c, 2);
	}

	else
	{
		perror("IMPLEMENT TEST PARSER FOR THE CURRENT SIZE OF reg_t");
		exit(1);
	}

}

void testParse()
{

	parseVarious();
	parseHexAndBack();
	fillHexStringFailWhenBufferSmall();

}