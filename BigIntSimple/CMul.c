#include "BigIntSimple.h"

reg_t LongMultiplication(reg_t* A, reg_t ASize, reg_t * B, reg_t BSize, reg_t* R)
{
	
		/*
		   Muliply number A composed of N digit of size WORD
		   [A0][A1]...[Am]
		   by number B
		   [B0][B1]...[Bn]
		*/
		_word_t r;
		_word_struct b; //number B[n]

		unsigned outBuffSize = m + n;

		for (unsigned int k = 0; k < outBuffSize; ++k)
			R[k] = 0; //reset output array


		for (unsigned int j = 0; j < m; ++j)// read  left number
		{
			if (A[j] == 0)continue; // relatively low-cost optimization for multiply by 0, it should be measured..
			for (unsigned int i = 0; i < n; ++i) // read right number
			{
				int k = i + j;
				if (B[i] == 0)
					continue;
				b.dword = B[i];
				b.dword = b.dword * A[j];

				r = b.Pair.H;
				b.dword = (_dword_t)R[k] + b.Pair.L;
				R[k] = b.Pair.L;

				++k;
				b.dword = (_dword_t)R[k] + r + b.Pair.H;
				R[k] = b.Pair.L;
				r = b.Pair.H;
				while (r)
				{
					b.dword = r + (_dword_t)R[++k];       //sum the previous carry to Result[i+j+k]	
					R[k] = b.Pair.L;
					r = b.Pair.H; //if there is still a carry move it to low word.
				}
			}
		}
	}

