/* 
 * File:   sha1.h
 * Author: frans
 *
 * Created on 1 maart 2016, 9:21
 */

#ifndef SHA1_H
#define	SHA1_H


    
typedef unsigned long DWORD;
typedef unsigned int WORD;
typedef unsigned char BYTE;


typedef union _DWORD_VAL
{
    DWORD Val;
	WORD w[2];
    BYTE v[4];
    struct
    {
        WORD LW;
        WORD HW;
    } word;
    struct
    {
        BYTE LB;
        BYTE HB;
        BYTE UB;
        BYTE MB;
    } byte;
    struct
    {
        unsigned char b0:1;
        unsigned char b1:1;
        unsigned char b2:1;
        unsigned char b3:1;
        unsigned char b4:1;
        unsigned char b5:1;
        unsigned char b6:1;
        unsigned char b7:1;
        unsigned char b8:1;
        unsigned char b9:1;
        unsigned char b10:1;
        unsigned char b11:1;
        unsigned char b12:1;
        unsigned char b13:1;
        unsigned char b14:1;
        unsigned char b15:1;
        unsigned char b16:1;
        unsigned char b17:1;
        unsigned char b18:1;
        unsigned char b19:1;
        unsigned char b20:1;
        unsigned char b21:1;
        unsigned char b22:1;
        unsigned char b23:1;
        unsigned char b24:1;
        unsigned char b25:1;
        unsigned char b26:1;
        unsigned char b27:1;
        unsigned char b28:1;
        unsigned char b29:1;
        unsigned char b30:1;
        unsigned char b31:1;
    } bits;
} DWORD_VAL;



typedef enum {
    HASH_MD5	= 0u,		// MD5 is being calculated
    HASH_SHA1				// SHA-1 is being calculated
} HASH_TYPE;


typedef struct
{
    DWORD h0;				// Hash state h0
    DWORD h1;				// Hash state h1
    DWORD h2;				// Hash state h2
    DWORD h3;				// Hash state h3
    DWORD h4;				// Hash state h4
    DWORD bytesSoFar;		// Total number of bytes hashed so far
    BYTE partialBlock[64];	// Beginning of next 64 byte block
    HASH_TYPE hashType;		// Type of hash being calculated
} HASH_SUM;



void SHA1Initialize(HASH_SUM* theSum);
void SHA1AddData(HASH_SUM* theSum, BYTE* data, WORD len);
void SHA1Calculate(HASH_SUM* theSum, BYTE* result);


#endif	/* SHA1_H */

