
#include<immintrin.h>
#include<iostream>

void read_AVX(void* src, size_t nbytes, size_t iterations)
{
    __m256i* src_avx = (__m256i*)src;
    __m256i ymm[16], sum;
    _mm256_zeroall();

    for (int i = 0; i < 16; ++i)
        ymm[i] = _mm256_setzero_si256();

    while (iterations--)
    {
        for (size_t i = 0; i < nbytes/256; i++)
        {
            ymm[0 ]= _mm256_load_si256(src_avx);
            ymm[8 ] = _mm256_add_epi64(ymm[8], ymm[0]);
            ymm[1 ]= _mm256_load_si256(src_avx+1);
            ymm[9 ] = _mm256_add_epi64(ymm[9], ymm[1]);
            ymm[2 ]= _mm256_load_si256(src_avx+2);
            ymm[10]  = _mm256_add_epi64(ymm[10], ymm[2]);
            ymm[3 ]= _mm256_load_si256(src_avx+3);
            ymm[11]  = _mm256_add_epi64(ymm[11], ymm[3]);
            ymm[4 ]= _mm256_load_si256(src_avx+4);
            ymm[12]  = _mm256_add_epi64(ymm[12], ymm[4]);
            ymm[5 ]= _mm256_load_si256(src_avx+5);
            ymm[13]  = _mm256_add_epi64(ymm[13], ymm[5]);
            ymm[6 ]= _mm256_load_si256(src_avx+6);
            ymm[14]  = _mm256_add_epi64(ymm[14], ymm[6]);
            ymm[7 ]= _mm256_load_si256(src_avx+7);
            ymm[15]  = _mm256_add_epi64(ymm[15], ymm[7]);

            src_avx += 8;
        }
        src_avx = (__m256i*)src;
    }

    sum = _mm256_setzero_si256();
    for (int i = 8; i < 16; ++i)
        sum = _mm256_add_epi64(sum, ymm[i]);
    
    _mm256_store_si256(src_avx, sum);
}
