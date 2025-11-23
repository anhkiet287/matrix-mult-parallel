// omp_kernels.c
// OpenMP parallel implementations (WITH #pragma omp)
// Member 1 responsible

#include "kernels.h"
#include <omp.h>
#include <stdlib.h>

// ========== NAIVE MATRIX MULTIPLICATION (OpenMP) ==========
void matmul_omp(double *A, double *B, double *C, int n) {
    // KEY DIFFERENCE: Uses #pragma omp parallel for
    // Parallelizes the outer loop across threads
    
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i*n + k] * B[k*n + j];
            }
            C[i*n + j] = sum;
        }
    }
}

// ========== STRASSEN ALGORITHM (OpenMP) ==========
void strassen_omp(double *A, double *B, double *C, int n) {
    // TODO: Implement Strassen with OpenMP
    // Can use #pragma omp task for recursive calls
    // Or parallelize the 7 matrix multiplications
    
    // Placeholder: use OMP matmul for now
    matmul_omp(A, B, C, n);
}

// ========== PROPOSED ALGORITHM (OpenMP) ==========
void proposed_omp(double *A, double *B, double *C, int n) {
    // TODO: Implement your custom algorithm with OpenMP
    // Could add: collapse(2), schedule(dynamic), etc.
    
    // Placeholder: use OMP matmul for now
    matmul_omp(A, B, C, n);
}
