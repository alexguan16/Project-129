#include <stdint.h>
#include <stddef.h>
#include <immintrin.h>
#include <string.h>
#include <stdio.h>

void* memSearch(uint8_t* haystack, size_t hSz, uint8_t* needle, size_t nSz) {
	for(int i = 0; i < hSz - nSz; i++) {
		if(0 == memcmp(haystack + i, needle, nSz)) return haystack + i;
	}

	return NULL;
}

__attribute__((force_align_arg_pointer))
void* memSearchAVX2(uint8_t* haystack, size_t hSz, uint8_t* needle, size_t nSz) {
	if((uintptr_t)haystack % 32 != 0) {
		for(; (uintptr_t)haystack % 32 != 0; hSz--, haystack++) {
			if(hSz < nSz) return NULL;
			if(0 == memcmp(haystack, needle, nSz)) return haystack;
		}
	}
	if(nSz < 2 || nSz >= hSz || hSz <= 32) return NULL;

	__m256i b1 = _mm256_set1_epi8(needle[0]);
	__m256i b2 = _mm256_set1_epi8(needle[1]);
	__m256i curr = _mm256_load_si256((__m256i const*)haystack);
	int i = 0;

	for(; i < hSz-32-1; i += 32) {
		__m256i next = _mm256_load_si256((__m256i const*)(haystack + i + 32));
		__m256i shft = _mm256_alignr_epi8(next, curr, 1);

		__m256i m1 = _mm256_cmpeq_epi8(curr, b1);
		__m256i m2 = _mm256_cmpeq_epi8(shft, b2);
		__mmask32 mask = _mm256_movemask_epi8(_mm256_and_si256(m1, m2));

		while(mask != 0) {
			uint32_t off;
			off = __builtin_ctz(mask);

			if(i + off + nSz > hSz) break;
			if(0 == memcmp(haystack + i + off+2, needle+2, nSz-2)) {
				return haystack + i + off;
			}
			mask &= mask - 1;
		}
		curr = next;
	}

	for(;i <= hSz - nSz; i++) {
		if(0 == memcmp(haystack+i, needle, nSz)) return haystack + i;
	}

	return NULL;
} 

