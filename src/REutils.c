#include <stdint.h>
#include <stddef.h>
#include <immintrin.h>
#include <string.h>


inline void* memSearch(uint8_t* haystack, size_t hSz, uint8_t* needle, size_t nSz) {
	size_t i;
	for(i = 0; i <= hSz - nSz; i++) {
		if(0 == memcmp(haystack + i, needle, nSz)) return haystack + i;
	}

	return NULL;
}

__attribute__((force_align_arg_pointer))
void* memSearchAVX2(uint8_t* haystack, size_t hSz, uint8_t* needle, size_t nSz) {
	if(hSz + (uintptr_t)haystack % 32 < 64
		|| nSz < 2) return memSearch(haystack, hSz, needle, nSz);

	if((uintptr_t)haystack % 32 != 0) {
		for(; (uintptr_t)haystack % 32 != 0; hSz--, haystack++) {
			if(hSz < nSz) return NULL;
			if(0 == memcmp(haystack, needle, nSz)) return haystack;
		}
	}

	__m256i b1 = _mm256_set1_epi8(needle[0]);
	__m256i b2 = _mm256_set1_epi8(needle[1]);
	__m256i curr = _mm256_load_si256((__m256i const*)haystack);
	uint8_t lastByte = needle[nSz -1];
	size_t i = 0;

	for(; i + 32+32 <= hSz; i += 32) {
		__m256i next = _mm256_load_si256((__m256i const*)(haystack + i + 32));
		__m256i comb = _mm256_permute2x128_si256(curr, next, 0x21);
		__m256i shft = _mm256_alignr_epi8(comb, curr, 1);

		__m256i m1 = _mm256_cmpeq_epi8(curr, b1);
		__m256i m2 = _mm256_cmpeq_epi8(shft, b2);
		uint32_t mask = _mm256_movemask_epi8(_mm256_and_si256(m1, m2));

		while(mask != 0) {
			uint32_t off = _tzcnt_u32(mask);

			if(i + off + nSz > hSz) break;
			if(haystack[i + off + nSz-1] == lastByte
				&& 0 == memcmp(haystack + i + off+2, needle+2, nSz-2)) {
				return haystack + i + off;
			}
			mask = _blsr_u32(mask);
		}
		curr = next;
	}

	for(;i + nSz <= hSz; i++) {
		if(0 == memcmp(haystack+i, needle, nSz)) return haystack + i;
	}

	return NULL;
} 

