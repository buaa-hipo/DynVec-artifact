#include  "mode.h"
#include <string.h>
#include <assert.h>
#ifdef USE_DOUBLE
    #define FLOAT double
    #else
    #define FLOAT float
#endif
void reduce_pure(const FLOAT* src, FLOAT* dst, void*_idx, int nnz) {
    memset(dst, 0, nnz * sizeof(FLOAT));
    int* idx = (int*)_idx;
    for (int i = 0; i < nnz; ++i) {
        dst[idx[i]] += src[i];
    }
}
