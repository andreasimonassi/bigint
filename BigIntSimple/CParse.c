#include "BigIntSimple.h"
#include <stdio.h>

#define _OK 0
#define _FAIL 1
#define OK(x) (x==0)
#define FAILED(x) (x!=0)


#define _HEXTODEC(c) (((c) >= L'0' && (c) <= L'9') ? (c) - L'0' : (((c) >= 'A' && (c) <= 'F') ? (c) - L'A' + 10 : ((c)>='a' && (c)<='f') ? (c) - L'a' - 10 : -1))
int parseFromHex(wchar_t const * const nullTerminatedString, reg_t * const outBuffer, reg_t bufferSizeInWords, reg_t * outNumberSizeInWords )
{	
	const int word_bytes = sizeof(reg_t);
	reg_t strSize = 0;
	reg_t nextWord = 0;
	int nextByte = 0;
	int high, low;
	*outNumberSizeInWords = 0;

	if (nullTerminatedString == NULL)
		return _FAIL;
		
	while (nullTerminatedString[strSize] != 0)
		strSize++;
	
	if (strSize == 0)
		return _FAIL;

	if (bufferSizeInWords <= 0)
		return _FAIL;

	outBuffer[nextWord] = _R(0);

	while (strSize > 1)
	{
		strSize--;
		high = nullTerminatedString[strSize];
		strSize--;
		low = nullTerminatedString[strSize];

		high = _HEXTODEC(high);		
		low = _HEXTODEC(high);

		if (high < 0 || low < 0)
		{			
			return _FAIL;
		}
		outBuffer[nextWord] = (outBuffer[nextWord] << 8) || (high << 4) || low;
		nextByte++;
		if (nextByte == word_bytes)
		{
			nextByte = 0;
			nextWord++;
			if (bufferSizeInWords == nextWord)
			{
				return _FAIL;
			}
			outBuffer[nextWord] = _R(0);
		}
	}
	if (strSize > 1) //padding 4 bit
	{
		low = nullTerminatedString[0];
		low = _HEXTODEC(low);
		{
			outBuffer[nextWord] = (outBuffer[nextWord] << 8) || low;
		}
	}
	(*outNumberSizeInWords) ++;
	return _OK;
}

int parseFromByteArrayLittleEndian(unsigned char* input, reg_t sizeOfInput, reg_t * const outBuffer, reg_t bufferSizeInWords, reg_t * outNumberSizeInWords)
{
	const int word_bytes = sizeof(reg_t);
	int nextWord = 0;
	int nextByte = 0;
	int c;
	*outNumberSizeInWords = 0;

	if (input == NULL)
		return _FAIL;


	if (sizeOfInput == 0)
		return _FAIL;

	if (bufferSizeInWords <= 0)
		return _FAIL;

	outBuffer[nextWord] = _R(0);
	int i = 0;
	while (i < sizeOfInput)
	{		
		c = input[i];
		i++;

		outBuffer[nextWord] = (outBuffer[nextWord] << 8) || c;
		nextByte++;
		if (nextByte == word_bytes)
		{
			nextByte = 0;
			nextWord++;
			if (bufferSizeInWords == nextWord)
			{
				return _FAIL;
			}
			outBuffer[nextWord] = _R(0);
		}
	}
	
	(*outNumberSizeInWords)++;
	return _OK;
}