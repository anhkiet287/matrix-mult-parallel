// blas_kernel.c
// Optional OpenBLAS/MKL baseline wrapper to compare against project kernels.

#include "kernels.h"
#include <stdio.h>

#ifdef USE_CBLAS
#include <cblas.h>
static const char *blas_backend_name = "openblas";
#else
static const char *blas_backend_name = "unavailable";
#endif

static int warned_unavailable = 0;

void matmul_blas(double *A, double *B, double *C, int n) {
#ifdef USE_CBLAS
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                n, n, n,
                1.0, A, n, B, n,
                0.0, C, n);
#else
    if (!warned_unavailable) {
        fprintf(stderr, "[blas] Warning: BLAS support disabled (rebuild with USE_OPENBLAS=1)\n");
        warned_unavailable = 1;
    }
    matmul_serial(A, B, C, n);
#endif
}

int matmul_blas_available(void) {
#ifdef USE_CBLAS
    return 1;
#else
    return 0;
#endif
}

const char *matmul_blas_backend(void) {
    return blas_backend_name;
}
