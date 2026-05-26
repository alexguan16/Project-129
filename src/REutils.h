#include <stdint.h>
#include <stddef.h>

void*     memSearch(const void*, size_t, const void*, size_t);
__attribute__((force_align_arg_pointer))
void* memSearchAVX2(const void*, size_t, const void*, size_t);