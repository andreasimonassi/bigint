#include "BigIntSimple.h"
#include <stdio.h>
#include <stdlib.h>
/*
TODO: tests, don't use this until not tested
*/
#define _OK 0
#define _FAIL 1
#define OK(x) (x==0)
#define FAILED(x) (x!=0)

reg_t * AllocNumOrFail(numsize_t numwords)
{
	reg_t * ret = calloc(numwords, sizeof(reg_t));
	if (ret == NULL)
	{
		perror("IT'S THE END OF MEMORY");
		abort();
	}
	return ret;
}

#define _HEXTODEC(c) (((c) >= L'0' && (c) <= L'9') ? (c) - L'0' : (((c) >= 'A' && (c) <= 'F') ? (c) - L'A' + 10 : ((c)>='a' && (c)<='f') ? (c) - L'a' - 10 : -1))
#define _TOHEX(k)   (wchar_t) ((k) <= 9 ? (k) + 48 : (k) + 55)

_result_t parseFromHex(wchar_t const * const nullTerminatedString, reg_t * outBuffer, numsize_t bufferSizeInWords, numsize_t * nextWord )
{	
	/*TODO: MAKE IT ENCODING AGNOSTIC, here we expect input strings to be wchar_t but each character to be contained into a wchar_t
	this restriction could be removed
	*/
	const int word_bytes = sizeof(reg_t);
	reg_t strSize = 0;
	int nextByte = 0;
	int dhigh, dlow;
	reg_t high, low;
	*nextWord = 0;

	if (nullTerminatedString == NULL)
		return _FAIL;
		
	while (nullTerminatedString[strSize] != 0)
		strSize++;
	
	if (strSize == 0)
		return _FAIL;

	if (bufferSizeInWords <= 0)
		return _FAIL;

	*outBuffer = _R(0);

	while (strSize > 1)
	{
		strSize--;		
		dlow = nullTerminatedString[strSize];
		strSize--;
		dhigh = nullTerminatedString[strSize];

		dhigh = _HEXTODEC(dhigh);		
		dlow = _HEXTODEC(dlow);

		if (dhigh < 0 || dlow < 0)
		{			
			return _FAIL;
		}
		high = ((reg_t)dhigh) << (8 * sizeof(reg_t) - 4);
		low = ((reg_t)dlow) << (7 * sizeof(reg_t));
		*outBuffer = ((*outBuffer) >> 8) | high  | low;
		nextByte++;
		if (nextByte == word_bytes && strSize > 0)
		{
			nextByte = 0;
			(*nextWord)++;
			outBuffer++;
			if (bufferSizeInWords == *nextWord)
			{
				return _FAIL;
			}
			(*outBuffer) = _R(0);
		}
	}

	int padding = (word_bytes - nextByte)*8;

	if (strSize == 1) /*padding 4 bit*/
	{
		dlow = nullTerminatedString[0];
		dlow = _HEXTODEC(dlow);
		low = ((reg_t)dlow) << (8* sizeof(reg_t)-4);
		{
			*outBuffer = ((*outBuffer) >> 4) |  low;
		}
		padding -= 4;
	}

	if (padding > 0)
	{
		*outBuffer = (*outBuffer) >> padding;
	}
	(*nextWord)++;
	return _OK;
}

_result_t parseFromByteArrayLittleEndian(unsigned char* input, numsize_t sizeOfInput, reg_t * const outBuffer, numsize_t bufferSizeInWords, numsize_t * outNumberSizeInWords)
{
	const unsigned short word_bytes = sizeof(reg_t);
	numsize_t nextWord = 0;
	numsize_t nextByte = 0;
	int c;
	*outNumberSizeInWords = 0;

	if (input == NULL)
		return _FAIL;


	if (sizeOfInput == 0)
		return _FAIL;

	if (bufferSizeInWords <= 0)
		return _FAIL;

	outBuffer[nextWord] = _R(0);
	numsize_t i = 0;
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

_result_t FillHexString(wchar_t*buffer, unsigned bufferSize, reg_t* number, numsize_t size)
{
	numsize_t i;
	const int word_bytes = sizeof(reg_t);
	int skipzero = 1;
	for (i = size; i > 0 ; )
	{
		i--;
		if (number[i] == 0)
			continue;
		int current_byte = word_bytes - 1;
		while (current_byte >= 0)
		{
			int byte = (int)( number[i] >> (current_byte * 8)) & 0xff;
			current_byte--;
			int hi = byte >> 4;
			int lo = byte & 0xf;
			if (skipzero)
			{
				if (hi != 0)
				{
					skipzero = 0;
					if (bufferSize <= 2)
						return _FAIL;
					*buffer++ = _TOHEX(hi);
					*buffer++ = _TOHEX(lo);
					bufferSize-=2;
					continue;
				}
				else if (skipzero && lo != 0)
				{
					skipzero = 0;
					if (bufferSize <= 1)
						return _FAIL;
					*buffer++ = _TOHEX(lo);
					bufferSize--;
					continue;
				}
			}
			else
			{
				if (bufferSize <= 1)
					return _FAIL;
				*buffer++ = _TOHEX(hi);
				*buffer++ = _TOHEX(lo);
				bufferSize -= 2;
			}			
		}
		if (bufferSize <= 1)
			return _FAIL;
		*buffer = 0;
	}
	return _OK;
}
