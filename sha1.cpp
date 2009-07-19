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

#include "sha1.h"

#ifdef WIN32
//these seem to be defined already in linux
inline __m128i operator +(const __m128i &a, const __m128i &b){
	return _mm_add_epi32(a,b);
}
inline __m128i operator ^(const __m128i &a, const __m128i &b){
	return _mm_xor_si128(a,b);
}
inline __m128i operator |(const __m128i &a, const __m128i &b){
	return _mm_or_si128(a,b);
}
inline __m128i operator &(const __m128i &a, const __m128i &b){
	return _mm_and_si128(a,b);
}
#endif

const __m128i m=_mm_set1_epi32(0x00FF00FF);
const __m128i m2=_mm_set1_epi32(0xFF00FF00);


bool SHA1_CLEAN(const __m128i& plain1, const __m128i& plain2, const __m128i& plain3, const __m128i& plain4, int length, __m128i * mDigest, const __m128i& preCheck)
{
	
    int           t;                 /* Loop counter                */
 	__m128i    sse_W[80];
 	__m128i    sse_A, sse_B, sse_C, sse_D, sse_E, sse_temp;
	__m128i    sse_K;

    // precalculated first steps
	sse_K = _mm_set1_epi32(K0);


  sse_W[0] = _mm_set_epi32(
    __builtin_ia32_vec_ext_v4si ((__v4si)plain1, 0),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain2, 0),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain3, 0),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain4, 0)
    );
	SSE_Endian_Reverse32(sse_W[0]);
	sse_B = _mm_set1_epi32(2679412915) + sse_W[0];

	sse_W[1] = _mm_set_epi32(
    __builtin_ia32_vec_ext_v4si ((__v4si)plain1, 1),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain2, 1),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain3, 1),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain4, 1)
    );
	SSE_Endian_Reverse32(sse_W[1]);
	sse_A = SSE_ROTATE(sse_B,5) + _mm_set1_epi32(1722862861) + sse_W[1];

	sse_W[2] = _mm_set_epi32(
    __builtin_ia32_vec_ext_v4si ((__v4si)plain1, 2),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain2, 2),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain3, 2),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain4, 2)
    );

  SSE_Endian_Reverse32(sse_W[2]);
	sse_temp = SSE_ROTATE(sse_A,5) + ((_mm_set1_epi32(572662306) & sse_B) ^ _mm_set1_epi32(2079550178)) + _mm_set1_epi32(H2+K0) + sse_W[2];
	sse_E = SSE_ROTATE(sse_B,30);
	sse_B = sse_temp;

	sse_W[3] = _mm_set_epi32(
    __builtin_ia32_vec_ext_v4si ((__v4si)plain1, 3),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain2, 3),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain3, 3),
    __builtin_ia32_vec_ext_v4si ((__v4si)plain4, 3)
    );


  SSE_Endian_Reverse32(sse_W[3]);
	sse_temp = SSE_ROTATE(sse_B,5) + (((sse_E ^ _mm_set1_epi32(1506887872)) & sse_A) ^ _mm_set1_epi32(1506887872)) + _mm_set1_epi32(2079550178+K0) + sse_W[3];
	sse_D = SSE_ROTATE(sse_A,30);
	sse_A = sse_temp;

	sse_W[4] = _mm_setzero_si128();
	sse_temp = SSE_ROTATE(sse_A,5) + F_00_19(sse_B,sse_D,sse_E) + _mm_set1_epi32(1506887872+K0);
	sse_C = SSE_ROTATE(sse_B,30);
	sse_B = sse_A;
	sse_A = sse_temp;
	
	SSE_ROTATE1_NULL( sse_A, sse_B, sse_C, sse_D, sse_E );
	SSE_ROTATE1_NULL( sse_E, sse_A, sse_B, sse_C, sse_D );
	SSE_ROTATE1_NULL( sse_D, sse_E, sse_A, sse_B, sse_C );
	SSE_ROTATE1_NULL( sse_C, sse_D, sse_E, sse_A, sse_B );
	SSE_ROTATE1_NULL( sse_B, sse_C, sse_D, sse_E, sse_A );
	SSE_ROTATE1_NULL( sse_A, sse_B, sse_C, sse_D, sse_E );
	SSE_ROTATE1_NULL( sse_E, sse_A, sse_B, sse_C, sse_D );
	SSE_ROTATE1_NULL( sse_D, sse_E, sse_A, sse_B, sse_C );
	SSE_ROTATE1_NULL( sse_C, sse_D, sse_E, sse_A, sse_B );
	SSE_ROTATE1_NULL( sse_B, sse_C, sse_D, sse_E, sse_A );

	sse_W[14]  = _mm_setzero_si128();

	// do last few steps of round 1
	sse_W[15] = _mm_set1_epi32((length << 3)); SSE_ROTATE1( sse_A, sse_B, sse_C, sse_D, sse_E, sse_W[15] ); // set length
    SSE_EXPAND_14_16(16); SSE_ROTATE1( sse_E, sse_A, sse_B, sse_C, sse_D, sse_W[16] );
    SSE_EXPAND_14_16(17); SSE_ROTATE1( sse_D, sse_E, sse_A, sse_B, sse_C, sse_W[17] );
    SSE_EXPAND_3_16(18); SSE_ROTATE1( sse_C, sse_D, sse_E, sse_A, sse_B, sse_W[18] );
    SSE_EXPAND_3_16(19); SSE_ROTATE1( sse_B, sse_C, sse_D, sse_E, sse_A, sse_W[19] );

	// round 2
	sse_K = _mm_set1_epi32(K1);

	for(t = 20; t < 21; t++)
    {
		SSE_EXPAND_3(t);   SSE_ROTATE2_F( sse_A, sse_B, sse_C, sse_D, sse_E, sse_W[t] );
		SSE_EXPAND_3(t+1); SSE_ROTATE2_F( sse_E, sse_A, sse_B, sse_C, sse_D, sse_W[t+1] );
		SSE_EXPAND_3(t+2); SSE_ROTATE2_F( sse_D, sse_E, sse_A, sse_B, sse_C, sse_W[t+2] );
		SSE_EXPAND_3_8(t+3); SSE_ROTATE2_F( sse_C, sse_D, sse_E, sse_A, sse_B, sse_W[t+3] );
		SSE_EXPAND_3_8(t+4); SSE_ROTATE2_F( sse_B, sse_C, sse_D, sse_E, sse_A, sse_W[t+4] );
    }
	for(t = 25; t < 26; t++)
    {
		SSE_EXPAND_3_8(t);   SSE_ROTATE2_F( sse_A, sse_B, sse_C, sse_D, sse_E, sse_W[t] );
		SSE_EXPAND_3_8(t+1); SSE_ROTATE2_F( sse_E, sse_A, sse_B, sse_C, sse_D, sse_W[t+1] );
		SSE_EXPAND_3_8(t+2); SSE_ROTATE2_F( sse_D, sse_E, sse_A, sse_B, sse_C, sse_W[t+2] );
		SSE_EXPAND_3_8(t+3); SSE_ROTATE2_F( sse_C, sse_D, sse_E, sse_A, sse_B, sse_W[t+3] );
		SSE_EXPAND_3_8_14(t+4); SSE_ROTATE2_F( sse_B, sse_C, sse_D, sse_E, sse_A, sse_W[t+4] );
    }
	for(t = 30; t < 31; t++)
    {
		//SSE_EXPAND_3_8_14(t);   SSE_ROTATE2_F( sse_A, sse_B, sse_C, sse_D, sse_E, sse_W[t] ); // this was slower
		SSE_EXPAND(t);   SSE_ROTATE2_F( sse_A, sse_B, sse_C, sse_D, sse_E, sse_W[t] );
		SSE_EXPAND(t+1); SSE_ROTATE2_F( sse_E, sse_A, sse_B, sse_C, sse_D, sse_W[t+1] );
		SSE_EXPAND(t+2); SSE_ROTATE2_F( sse_D, sse_E, sse_A, sse_B, sse_C, sse_W[t+2] );
		SSE_EXPAND(t+3); SSE_ROTATE2_F( sse_C, sse_D, sse_E, sse_A, sse_B, sse_W[t+3] );
		SSE_EXPAND(t+4); SSE_ROTATE2_F( sse_B, sse_C, sse_D, sse_E, sse_A, sse_W[t+4] );
    }
	for(t = 35; t < 36; t++)
    {
		SSE_EXPAND(t);   SSE_ROTATE2_F( sse_A, sse_B, sse_C, sse_D, sse_E, sse_W[t] );
		SSE_EXPAND(t+1); SSE_ROTATE2_F( sse_E, sse_A, sse_B, sse_C, sse_D, sse_W[t+1] );
		SSE_EXPAND(t+2); SSE_ROTATE2_F( sse_D, sse_E, sse_A, sse_B, sse_C, sse_W[t+2] );
		SSE_EXPAND(t+3); SSE_ROTATE2_F( sse_C, sse_D, sse_E, sse_A, sse_B, sse_W[t+3] );
		SSE_EXPAND(t+4); SSE_ROTATE2_F( sse_B, sse_C, sse_D, sse_E, sse_A, sse_W[t+4] );
    }

	// round 3
	sse_K = _mm_set1_epi32(K2);

	for(t = 40; t < 60; t+=5)
	{
		SSE_EXPAND(t);   SSE_ROTATE3_F( sse_A, sse_B, sse_C, sse_D, sse_E, sse_W[t] );
		SSE_EXPAND(t+1); SSE_ROTATE3_F( sse_E, sse_A, sse_B, sse_C, sse_D, sse_W[t+1] );
		SSE_EXPAND(t+2); SSE_ROTATE3_F( sse_D, sse_E, sse_A, sse_B, sse_C, sse_W[t+2] );
		SSE_EXPAND(t+3); SSE_ROTATE3_F( sse_C, sse_D, sse_E, sse_A, sse_B, sse_W[t+3] );
		SSE_EXPAND(t+4); SSE_ROTATE3_F( sse_B, sse_C, sse_D, sse_E, sse_A, sse_W[t+4] );
	}
	
	// round 4 except last 4 steps
	sse_K = _mm_set1_epi32(K3);

	for(t = 60; t < 75; t+=5 )
    {
		SSE_EXPAND(t);   SSE_ROTATE4_F( sse_A, sse_B, sse_C, sse_D, sse_E, sse_W[t] );
		SSE_EXPAND(t+1); SSE_ROTATE4_F( sse_E, sse_A, sse_B, sse_C, sse_D, sse_W[t+1] );
		SSE_EXPAND(t+2); SSE_ROTATE4_F( sse_D, sse_E, sse_A, sse_B, sse_C, sse_W[t+2] );
		SSE_EXPAND(t+3); SSE_ROTATE4_F( sse_C, sse_D, sse_E, sse_A, sse_B, sse_W[t+3] );
		SSE_EXPAND(t+4); SSE_ROTATE4_F( sse_B, sse_C, sse_D, sse_E, sse_A, sse_W[t+4] );
    }
	SSE_EXPAND(75);	SSE_ROTATE4(75);
	
	// check if one of the 4 A's matches with the hash we are searching for
	int result_mask = _mm_movemask_epi8(_mm_cmpeq_epi32(preCheck, sse_A));
	if (result_mask > 0)
	{
		SSE_EXPAND(76); SSE_ROTATE4(76);
		SSE_EXPAND(77); sse_E = F_60_79(sse_B,sse_C,sse_D) + sse_E + sse_W[77];
		SSE_EXPAND(78);	sse_D = F_60_79(sse_A,SSE_ROTATE(sse_B,30),sse_C) + sse_D + sse_W[78];
		SSE_EXPAND(79);	sse_C = sse_C + sse_W[79];

		mDigest[0] = sse_A;
		mDigest[1] = sse_B;
		mDigest[2] = sse_C;
		mDigest[3] = sse_D;
		mDigest[4] = sse_E;

		return true;
	}
	// no match, skip last 4 steps
	else {
		return false;
	}

}
