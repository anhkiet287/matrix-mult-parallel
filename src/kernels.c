// kernels.c
// Serial implementations (NO OpenMP)
// Member 2 responsible

#include "kernels.h"
#include "utility.h"
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
// Threshold for falling back to naive multiplication
#define STRASSEN_THRESHOLD 64

// Helper: Recursively pad n to next power of 2
static int next_power_of_2(int n) {
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

// Helper: Copy submatrix from src to dst
static void copy_submatrix(double *src, double *dst, int n, 
                           int rowSrc, int colSrc, int strideSrc,
                           int rowDst, int colDst, int strideDst) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dst[(rowDst + i) * strideDst + (colDst + j)] = 
                src[(rowSrc + i) * strideSrc + (colSrc + j)];
        }
    }
}

// Recursive Strassen - simplified version with copying
static void strassen_recursive(double *A, double *B, double *C, int n, int stride) {
    // Base case: use naive multiplication
    if (n <= STRASSEN_THRESHOLD) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                double sum = 0.0;
                for (int k = 0; k < n; k++) {
                    sum += A[i * stride + k] * B[k * stride + j];
                }
                C[i * stride + j] += sum;
            }
        }
        return;
    }
    
    int half = n / 2;
    
    // Allocate temporary matrices (all are half × half)
    double *A11 = matrix_allocate(half);
    double *A12 = matrix_allocate(half);
    double *A21 = matrix_allocate(half);
    double *A22 = matrix_allocate(half);
    
    double *B11 = matrix_allocate(half);
    double *B12 = matrix_allocate(half);
    double *B21 = matrix_allocate(half);
    double *B22 = matrix_allocate(half);
    
    double *M1 = matrix_allocate(half);
    double *M2 = matrix_allocate(half);
    double *M3 = matrix_allocate(half);
    double *M4 = matrix_allocate(half);
    double *M5 = matrix_allocate(half);
    double *M6 = matrix_allocate(half);
    double *M7 = matrix_allocate(half);
    
    double *temp1 = matrix_allocate(half);
    double *temp2 = matrix_allocate(half);
    
    // Copy submatrices from A and B
    copy_submatrix(A, A11, half, 0, 0, stride, 0, 0, half);
    copy_submatrix(A, A12, half, 0, half, stride, 0, 0, half);
    copy_submatrix(A, A21, half, half, 0, stride, 0, 0, half);
    copy_submatrix(A, A22, half, half, half, stride, 0, 0, half);
    
    copy_submatrix(B, B11, half, 0, 0, stride, 0, 0, half);
    copy_submatrix(B, B12, half, 0, half, stride, 0, 0, half);
    copy_submatrix(B, B21, half, half, 0, stride, 0, 0, half);
    copy_submatrix(B, B22, half, half, half, stride, 0, 0, half);
    
    // M1 = (A11 + A22) * (B11 + B22)
    matrix_add(A11, A22, temp1, half);
    matrix_add(B11, B22, temp2, half);
    matrix_zero_init(M1, half);
    strassen_recursive(temp1, temp2, M1, half, half);
    
    // M2 = (A21 + A22) * B11
    matrix_add(A21, A22, temp1, half);
    matrix_zero_init(M2, half);
    strassen_recursive(temp1, B11, M2, half, half);
    
    // M3 = A11 * (B12 - B22)
    matrix_sub(B12, B22, temp2, half);
    matrix_zero_init(M3, half);
    strassen_recursive(A11, temp2, M3, half, half);
    
    // M4 = A22 * (B21 - B11)
    matrix_sub(B21, B11, temp2, half);
    matrix_zero_init(M4, half);
    strassen_recursive(A22, temp2, M4, half, half);
    
    // M5 = (A11 + A12) * B22
    matrix_add(A11, A12, temp1, half);
    matrix_zero_init(M5, half);
    strassen_recursive(temp1, B22, M5, half, half);
    
    // M6 = (A21 - A11) * (B11 + B12)
    matrix_sub(A21, A11, temp1, half);
    matrix_add(B11, B12, temp2, half);
    matrix_zero_init(M6, half);
    strassen_recursive(temp1, temp2, M6, half, half);
    
    // M7 = (A12 - A22) * (B21 + B22)
    matrix_sub(A12, A22, temp1, half);
    matrix_add(B21, B22, temp2, half);
    matrix_zero_init(M7, half);
    strassen_recursive(temp1, temp2, M7, half, half);
    
    // Combine results into C
    // C11 = M1 + M4 - M5 + M7
    // C12 = M3 + M5
    // C21 = M2 + M4
    // C22 = M1 - M2 + M3 + M6
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < half; j++) {
            C[i * stride + j] += M1[i * half + j] + M4[i * half + j] - M5[i * half + j] + M7[i * half + j];
            C[i * stride + (j + half)] += M3[i * half + j] + M5[i * half + j];
            C[(i + half) * stride + j] += M2[i * half + j] + M4[i * half + j];
            C[(i + half) * stride + (j + half)] += M1[i * half + j] - M2[i * half + j] + M3[i * half + j] + M6[i * half + j];
        }
    }
    
    // Free all temporary matrices
    matrix_free(A11); matrix_free(A12); matrix_free(A21); matrix_free(A22);
    matrix_free(B11); matrix_free(B12); matrix_free(B21); matrix_free(B22);
    matrix_free(M1); matrix_free(M2); matrix_free(M3); matrix_free(M4);
    matrix_free(M5); matrix_free(M6); matrix_free(M7);
    matrix_free(temp1); matrix_free(temp2);
}

void strassen_serial(double *A, double *B, double *C, int n) {
    // Pad to next power of 2 if needed
    int padded_n = next_power_of_2(n);
    
    if (padded_n != n) {
        // Need padding
        double *A_padded = matrix_allocate(padded_n);
        double *B_padded = matrix_allocate(padded_n);
        double *C_padded = matrix_allocate(padded_n);
        
        matrix_zero_init(A_padded, padded_n);
        matrix_zero_init(B_padded, padded_n);
        matrix_zero_init(C_padded, padded_n);
        
        // Copy original matrices
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                A_padded[i * padded_n + j] = A[i * n + j];
                B_padded[i * padded_n + j] = B[i * n + j];
            }
        }
        
        strassen_recursive(A_padded, B_padded, C_padded, padded_n, padded_n);
        
        // Copy result back
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                C[i * n + j] = C_padded[i * padded_n + j];
            }
        }
        
        matrix_free(A_padded);
        matrix_free(B_padded);
        matrix_free(C_padded);
    } else {
        // Already power of 2
        matrix_zero_init(C, n);
        strassen_recursive(A, B, C, n, n);
    }
}

// ========== PROPOSED ALGORITHM (SERIAL) ==========
// Block size for cache optimization (tune this based on cache size)
// Typical L1 cache: 32KB, so BS=64 means 64*64*8 = 32KB per block
#define BLOCK_SIZE 64

void proposed_serial(double *A, double *B, double *C, int n) {
    // Step 1: Transpose B for better cache locality
    double *B_T = matrix_allocate(n);
    matrix_transpose(B, B_T, n);
    
    // Step 2: Zero-initialize C
    matrix_zero_init(C, n);
    
    // Step 3: Cache-blocked multiplication with transposed B
    // Now we compute C[i][j] = sum_k A[i][k] * B_T[j][k]
    // Both A[i][*] and B_T[j][*] are row-contiguous = happy cache
    for (int ii = 0; ii < n; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < n; jj += BLOCK_SIZE) {
            for (int kk = 0; kk < n; kk += BLOCK_SIZE) {
                // Compute block boundaries
                int i_end = (ii + BLOCK_SIZE < n) ? ii + BLOCK_SIZE : n;
                int j_end = (jj + BLOCK_SIZE < n) ? jj + BLOCK_SIZE : n;
                int k_end = (kk + BLOCK_SIZE < n) ? kk + BLOCK_SIZE : n;
                
                // Inner block computation
                for (int i = ii; i < i_end; i++) {
                    for (int j = jj; j < j_end; j++) {
                        double sum = 0.0;
                        // Dot product of A[i][k] and B_T[j][k] over k
                        for (int k = kk; k < k_end; k++) {
                            sum += A[i * n + k] * B_T[j * n + k];
                        }
                        C[i * n + j] += sum;
                    }
                }
            }
        }
    }
    
    // Step 4: Clean up
    matrix_free(B_T);
}
