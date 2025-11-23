// kernels.c
// Serial implementations (NO OpenMP)
// Member 2 responsible

#include "kernels.h"
#include <stdlib.h>

// ========== NAIVE MATRIX MULTIPLICATION (SERIAL) ==========
void matmul_serial(double *A, double *B, double *C, int n) {
    // Classic triple-loop matrix multiplication
    // C[i][j] = sum(A[i][k] * B[k][j])
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

// ========== STRASSEN ALGORITHM (SERIAL) ==========
void strassen_serial(double *A, double *B, double *C, int n) {
    // TODO: Implement Strassen algorithm
    // For now, fallback to naive
    // Note: Strassen requires n to be power of 2
    // Will implement: divide matrix into 4 blocks, compute 7 products
    
    // Placeholder: use naive for now
    matmul_serial(A, B, C, n);
}

// ========== PROPOSED ALGORITHM (SERIAL) ==========
void proposed_serial(double *A, double *B, double *C, int n) {
    // TODO: Implement your custom algorithm
    // Could be: block-tiled, cache-optimized, hybrid naive+Strassen, etc.
    
    // Placeholder: use naive for now
    matmul_serial(A, B, C, n);
}
