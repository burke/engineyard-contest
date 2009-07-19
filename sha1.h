/*
 * Implementation of the SHA-1 message-digest algorithm, optimized for passwords with length < 16
 * (see http://tools.ietf.org/html/rfc3174)
 *
 * Author: Daniël Niggebrugge
 * License: Use and share as you wish at your own risk, please keep this header ;)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, [...] etc :p
 *
 */

#include <emmintrin.h>

typedef unsigned int UINT4;

#define K0 0x5A827999
#define K1 0x6ED9EBA1
#define K2 0x8F1BBCDC
#define K3 0xCA62C1D6

#define H0 0x67452301
#define H1 0xEFCDAB89
#define H2 0x98BADCFE
#define H3 0x10325476
#define H4 0xC3D2E1F0

#define	F_00_19(b,c,d)	((((c) ^ (d)) & (b)) ^ (d)) 
#define	F_20_39(b,c,d)	((b) ^ (c) ^ (d))
#define F_40_59(b,c,d)	(((b) & (c)) | (((b)|(c)) & (d))) 
#define	F_60_79(b,c,d)	F_20_39(b,c,d)

#define SSE_ROTATE(a,n) _mm_or_si128(_mm_slli_epi32(a, n), _mm_srli_epi32(a, (32-n)))
#define SSE_ROTATE_BACK(a,n) _mm_or_si128(_mm_srli_epi32(a, n), _mm_slli_epi32(a, (32-n)))

#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#define ROTATE_BACK(a,n)     (((a)>>(n))|(((a)&0xffffffff)<<(32-(n))))

/* A nice byte order reversal from Wei Dai <weidai@eskimo.com> */
#if defined(WIN32)
/* 5 instructions with rotate instruction, else 9 */
#define Endian_Reverse32(a) \
	{ \
	unsigned long l=(a); \
	(a)=((ROTATE(l,8)&0x00FF00FF)|(ROTATE(l,24)&0xFF00FF00)); \
	}
#else
/* 6 instructions with rotate instruction, else 8 */
#define Endian_Reverse32(a) \
	{ \
	unsigned long l=(a); \
	l=(((l&0xFF00FF00)>>8L)|((l&0x00FF00FF)<<8L)); \
	(a)=ROTATE(l,16L); \
	}
#endif

#define SSE_Endian_Reverse32(a) \
{ \
__m128i l=(a); \
(a)=((SSE_ROTATE(l,8)&m)|(SSE_ROTATE(l,24)&m2)); \
}

#define SSE_EXPAND(t) \
{ \
	sse_W[t] = SSE_ROTATE(sse_W[t-3] ^ sse_W[t-8] ^ sse_W[t-14] ^ sse_W[t-16],1); \
}

#define SSE_EXPAND_3(t) \
{ \
	sse_W[t] = SSE_ROTATE(sse_W[t-3],1); \
}

#define SSE_EXPAND_3_8(t) \
{ \
	sse_W[t] = SSE_ROTATE(sse_W[t-3] ^ sse_W[t-8],1); \
}

#define SSE_EXPAND_3_16(t) \
{ \
	sse_W[t] = SSE_ROTATE(sse_W[t-3] ^ sse_W[t-16],1); \
}

#define SSE_EXPAND_14_16(t) \
{ \
	sse_W[t] = SSE_ROTATE(sse_W[t-14] ^ sse_W[t-16],1); \
}

#define SSE_EXPAND_3_8_14(t) \
{ \
	sse_W[t] = SSE_ROTATE(sse_W[t-3] ^ sse_W[t-8] ^ sse_W[t-14], 1); \
}

#define SSE_ROTATE1(a, b, c, d, e, x) \
	e = e + SSE_ROTATE(a,5) + F_00_19(b,c,d) + x + sse_K; \
	b = SSE_ROTATE(b,30);

#define SSE_ROTATE1_NULL(a, b, c, d, e) \
	e = e + SSE_ROTATE(a,5) + F_00_19(b,c,d) + sse_K; \
	b = SSE_ROTATE(b,30);

#define SSE_ROTATE2_F(a, b, c, d, e, x) \
	e = e + SSE_ROTATE(a,5) + F_20_39(b,c,d) + x + sse_K; \
	b = SSE_ROTATE(b,30);

#define SSE_ROTATE3_F(a, b, c, d, e, x) \
	e = e + SSE_ROTATE(a,5) + F_40_59(b,c,d) + x + sse_K; \
	b = SSE_ROTATE(b,30);

#define SSE_ROTATE4_F(a, b, c, d, e, x) \
	e = e + SSE_ROTATE(a,5) + F_60_79(b,c,d) + x + sse_K; \
	b = SSE_ROTATE(b,30);

#define SSE_ROTATE4(t) \
	sse_temp = SSE_ROTATE(sse_A,5) + F_60_79(sse_B,sse_C,sse_D) + sse_E + sse_W[t] + sse_K; \
	sse_E = sse_D; sse_D = sse_C; \
	sse_C = SSE_ROTATE(sse_B,30); \
	sse_B = sse_A; sse_A = sse_temp;

bool SHA1_CLEAN(const __m128i& plain1, const __m128i& plain2, const __m128i& plain3, const __m128i& plain4, int length, __m128i * mDigest, const __m128i& preCheck);
