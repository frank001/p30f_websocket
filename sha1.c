#include "sha1.h"

#define leftRotateDWORD(x, n) (((x) << (n)) | ((x) >> (32-(n))))

BYTE lastBlock[64];

static void SHA1HashBlock(BYTE* data, DWORD* h0, DWORD* h1, DWORD* h2, DWORD* h3, DWORD* h4);

void SHA1Initialize(HASH_SUM* theSum) {
	theSum->h0 = 0x67452301;
	theSum->h1 = 0xEFCDAB89;
	theSum->h2 = 0x98BADCFE;
	theSum->h3 = 0x10325476;
	theSum->h4 = 0xC3D2E1F0;
	theSum->bytesSoFar = 0;
	theSum->hashType = HASH_SHA1;
}

void SHA1AddData(HASH_SUM* theSum, BYTE* data, WORD len)
{
	BYTE *blockPtr;

	// Seek to the first free byte
	blockPtr = theSum->partialBlock + ( theSum->bytesSoFar & 0x3f );

	// Update the total number of bytes
	theSum->bytesSoFar += len;

	// Copy data into the partial block
	while(len != 0u)
	{
		*blockPtr++ = *data++;

		// If the partial block is full, hash the data and start over
		if(blockPtr == theSum->partialBlock + 64)
		{
			SHA1HashBlock(theSum->partialBlock, &theSum->h0, &theSum->h1,
							 &theSum->h2, &theSum->h3, &theSum->h4);
			blockPtr = theSum->partialBlock;
		}
		
		len--;
	}
	
}

static void SHA1HashBlock(BYTE* data, DWORD* h0, DWORD* h1, DWORD* h2, DWORD* h3, DWORD* h4) {
	DWORD a, b, c, d, e, f, k, temp;
	DWORD_VAL *w = (DWORD_VAL*) lastBlock;
	BYTE i, back3, back8, back14;

	// Set up a, b, c, d, e
	a = *h0;
	b = *h1;
	c = *h2;
	d = *h3;
	e = *h4;

	// Set up the w[] vector
	if(lastBlock == data)
	{// If they're the same, just swap endian-ness
		for(i = 0; i < 16u; i++)
		{
			back3 = data[3];
			data[3] = data[0];
			data[0] = back3;
			back3 = data[1];
			data[1] = data[2];
			data[2] = back3;
			data += 4;
		}
	}
	else
	{// Otherwise, copy values in swaping endian-ness as we go
		for(i = 0; i < 16u; i++)
		{
			w[i].v[3] = *data++;
			w[i].v[2] = *data++;
			w[i].v[1] = *data++;
			w[i].v[0] = *data++;
		}
	}
	back3 = 13;
	back8 = 8;
	back14 = 2;

	// Main mixer loop for 80 operations
	for(i = 0; i < 80u; i++)
	{
		if(i <= 19u)
		{
			f = (b & c) | ((~b) & d);
			k = 0x5A827999;
		}
		else if(i >= 20u && i <= 39u)
		{
			f = b ^ c ^ d;
			k = 0x6ED9EBA1;
		}
		else if(i >= 40u && i <= 59u)
		{
			f = (b & c) | (b & d) | (c & d);
			k = 0x8F1BBCDC;
		}
		else
		{
			f = b ^ c ^ d;
			k = 0xCA62C1D6;
		}

		// Calculate the w[] value and store it in the array for future use
		if(i >= 16u)
		{
			#if defined(HI_TECH_C)
			// This section is unrolled for HI_TECH_C because it cannot parse
			// the expression used by the other compilers
			DWORD temp2;
			temp = w[back3].Val;
			temp2 = w[back8].Val;
			temp ^= temp2;
			temp2 = w[back14].Val;
			temp ^= temp2;
			temp2 = w[i&0x0f].Val;
			temp ^= temp2;
			w[i&0x0f].Val = leftRotateDWORD(temp, 1);
			#else
			w[i&0x0f].Val = leftRotateDWORD( ( w[back3].Val ^ w[back8].Val ^
											w[back14].Val ^ w[i&0x0f].Val), 1);
			#endif
			back3 += 1;
			back8 += 1;
			back14 += 1;
			back3 &= 0x0f;
			back8 &= 0x0f;
			back14 &= 0x0f;
		}
		
		// Calculate the new mixers
		temp = leftRotateDWORD(a, 5) + f + e + k + w[i & 0x0f].Val;
		e = d;
		d = c;
		c = leftRotateDWORD(b, 30);
		b = a;
		a = temp;
	}

	// Add the new hash to the sum
	*h0 += a;
	*h1 += b;
	*h2 += c;
	*h3 += d;
	*h4 += e;

}

void SHA1Calculate(HASH_SUM* theSum, BYTE* result)
{
	DWORD h0, h1, h2, h3, h4;
	BYTE i, *partPtr, *endPtr;

	// Initialize the hash variables
	h0 = theSum->h0;
	h1 = theSum->h1;
	h2 = theSum->h2;
	h3 = theSum->h3;
	h4 = theSum->h4;

	// Find out how far along we are in the partial block and copy to last block
	partPtr = theSum->partialBlock;
	endPtr = partPtr + ( theSum->bytesSoFar & 0x3f );
	for(i = 0; partPtr != endPtr; i++)
	{
		lastBlock[i] = *partPtr++;
	}

	// Add one more bit and 7 zeros
	lastBlock[i++] = 0x80;

	// If there's 8 or more bytes left to 64, then this is the last block
	if(i > 56u)
	{// If there's not enough space, then zero fill this and add a new block
		// Zero pad the remainder
		for( ; i < 64u; lastBlock[i++] = 0x00);

		// Calculate a hash on this block and add it to the sum
		SHA1HashBlock(lastBlock, &h0, &h1, &h2, &h3, &h4);

		//create a new block for the size
		i = 0;
	}

	// Zero fill the rest of the block
	for( ; i < 56u; lastBlock[i++] = 0x00);

	// Fill in the size, in bits, in big-endian
	lastBlock[63] = theSum->bytesSoFar << 3;
	lastBlock[62] = theSum->bytesSoFar >> 5;
	lastBlock[61] = theSum->bytesSoFar >> 13;
	lastBlock[60] = theSum->bytesSoFar >> 21;
	lastBlock[59] = theSum->bytesSoFar >> 29;
	lastBlock[58] = 0;
	lastBlock[57] = 0;
	lastBlock[56] = 0;

	// Calculate a hash on this final block and add it to the sum
	SHA1HashBlock(lastBlock, &h0, &h1, &h2, &h3, &h4);
	
	// Format the result in big-endian format
	*result++ = ((BYTE*)&h0)[3];
	*result++ = ((BYTE*)&h0)[2];
	*result++ = ((BYTE*)&h0)[1];
	*result++ = ((BYTE*)&h0)[0];
	*result++ = ((BYTE*)&h1)[3];
	*result++ = ((BYTE*)&h1)[2];
	*result++ = ((BYTE*)&h1)[1];
	*result++ = ((BYTE*)&h1)[0];
	*result++ = ((BYTE*)&h2)[3];
	*result++ = ((BYTE*)&h2)[2];
	*result++ = ((BYTE*)&h2)[1];
	*result++ = ((BYTE*)&h2)[0];
	*result++ = ((BYTE*)&h3)[3];
	*result++ = ((BYTE*)&h3)[2];
	*result++ = ((BYTE*)&h3)[1];
	*result++ = ((BYTE*)&h3)[0];
	*result++ = ((BYTE*)&h4)[3];
	*result++ = ((BYTE*)&h4)[2];
	*result++ = ((BYTE*)&h4)[1];
	*result++ = ((BYTE*)&h4)[0];
}

