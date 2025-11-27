// omp_kernels.c
// OpenMP parallel implementations (WITH #pragma omp)
// Member 1 responsible

#include "kernels.h"
#include "utility.h"
#include <stdlib.h>

// Include omp.h only when OpenMP is enabled; otherwise these
// functions compile as serial fallbacks.
#ifdef _OPENMP
#include <omp.h>
#endif

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
#define STRASSEN_THRESHOLD_OMP 64
#define STRASSEN_PARALLEL_CUTOFF 256  // Only parallelize above this size

static int next_power_of_2_omp(int n) {
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

// Helper: Copy submatrix (same as serial)
static void copy_submatrix_omp(double *src, double *dst, int n, 
                               int rowSrc, int colSrc, int strideSrc,
                               int rowDst, int colDst, int strideDst) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dst[(rowDst + i) * strideDst + (colDst + j)] = 
                src[(rowSrc + i) * strideSrc + (colSrc + j)];
        }
    }
}

// Recursive Strassen with OpenMP tasks
static void strassen_recursive_omp(double *A, double *B, double *C, int n, int stride, int use_tasks) {
    // Base case: use naive multiplication
    if (n <= STRASSEN_THRESHOLD_OMP) {
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
    
    // Allocate temporary matrices
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
    
    // Copy submatrices
    copy_submatrix_omp(A, A11, half, 0, 0, stride, 0, 0, half);
    copy_submatrix_omp(A, A12, half, 0, half, stride, 0, 0, half);
    copy_submatrix_omp(A, A21, half, half, 0, stride, 0, 0, half);
    copy_submatrix_omp(A, A22, half, half, half, stride, 0, 0, half);
    
    copy_submatrix_omp(B, B11, half, 0, 0, stride, 0, 0, half);
    copy_submatrix_omp(B, B12, half, 0, half, stride, 0, 0, half);
    copy_submatrix_omp(B, B21, half, half, 0, stride, 0, 0, half);
    copy_submatrix_omp(B, B22, half, half, half, stride, 0, 0, half);
    
    // Decide whether to use tasks (only at top levels to avoid overhead)
    int use_tasks_recursive = use_tasks && (n >= STRASSEN_PARALLEL_CUTOFF);
    
    if (use_tasks_recursive) {
        // Parallelize the 7 products using OpenMP tasks
        #pragma omp task shared(M1, A11, A22, B11, B22, temp1, temp2)
        {
            matrix_add(A11, A22, temp1, half);
            matrix_add(B11, B22, temp2, half);
            matrix_zero_init(M1, half);
            strassen_recursive_omp(temp1, temp2, M1, half, half, 0);
        }
        
        #pragma omp task shared(M2, A21, A22, B11)
        {
            double *t1 = matrix_allocate(half);
            matrix_add(A21, A22, t1, half);
            matrix_zero_init(M2, half);
            strassen_recursive_omp(t1, B11, M2, half, half, 0);
            matrix_free(t1);
        }
        
        #pragma omp task shared(M3, A11, B12, B22)
        {
            double *t2 = matrix_allocate(half);
            matrix_sub(B12, B22, t2, half);
            matrix_zero_init(M3, half);
            strassen_recursive_omp(A11, t2, M3, half, half, 0);
            matrix_free(t2);
        }
        
        #pragma omp task shared(M4, A22, B21, B11)
        {
            double *t2 = matrix_allocate(half);
            matrix_sub(B21, B11, t2, half);
            matrix_zero_init(M4, half);
            strassen_recursive_omp(A22, t2, M4, half, half, 0);
            matrix_free(t2);
        }
        
        #pragma omp task shared(M5, A11, A12, B22)
        {
            double *t1 = matrix_allocate(half);
            matrix_add(A11, A12, t1, half);
            matrix_zero_init(M5, half);
            strassen_recursive_omp(t1, B22, M5, half, half, 0);
            matrix_free(t1);
        }
        
        #pragma omp task shared(M6, A21, A11, B11, B12)
        {
            double *t1 = matrix_allocate(half);
            double *t2 = matrix_allocate(half);
            matrix_sub(A21, A11, t1, half);
            matrix_add(B11, B12, t2, half);
            matrix_zero_init(M6, half);
            strassen_recursive_omp(t1, t2, M6, half, half, 0);
            matrix_free(t1);
            matrix_free(t2);
        }
        
        #pragma omp task shared(M7, A12, A22, B21, B22)
        {
            double *t1 = matrix_allocate(half);
            double *t2 = matrix_allocate(half);
            matrix_sub(A12, A22, t1, half);
            matrix_add(B21, B22, t2, half);
            matrix_zero_init(M7, half);
            strassen_recursive_omp(t1, t2, M7, half, half, 0);
            matrix_free(t1);
            matrix_free(t2);
        }
        
        #pragma omp taskwait
        
    } else {
        // Serial execution for smaller problems
        matrix_add(A11, A22, temp1, half);
        matrix_add(B11, B22, temp2, half);
        matrix_zero_init(M1, half);
        strassen_recursive_omp(temp1, temp2, M1, half, half, 0);
        
        matrix_add(A21, A22, temp1, half);
        matrix_zero_init(M2, half);
        strassen_recursive_omp(temp1, B11, M2, half, half, 0);
        
        matrix_sub(B12, B22, temp2, half);
        matrix_zero_init(M3, half);
        strassen_recursive_omp(A11, temp2, M3, half, half, 0);
        
        matrix_sub(B21, B11, temp2, half);
        matrix_zero_init(M4, half);
        strassen_recursive_omp(A22, temp2, M4, half, half, 0);
        
        matrix_add(A11, A12, temp1, half);
        matrix_zero_init(M5, half);
        strassen_recursive_omp(temp1, B22, M5, half, half, 0);
        
        matrix_sub(A21, A11, temp1, half);
        matrix_add(B11, B12, temp2, half);
        matrix_zero_init(M6, half);
        strassen_recursive_omp(temp1, temp2, M6, half, half, 0);
        
        matrix_sub(A12, A22, temp1, half);
        matrix_add(B21, B22, temp2, half);
        matrix_zero_init(M7, half);
        strassen_recursive_omp(temp1, temp2, M7, half, half, 0);
    }
    
    // Combine results into C
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < half; j++) {
            C[i * stride + j] += M1[i * half + j] + M4[i * half + j] - M5[i * half + j] + M7[i * half + j];
            C[i * stride + (j + half)] += M3[i * half + j] + M5[i * half + j];
            C[(i + half) * stride + j] += M2[i * half + j] + M4[i * half + j];
            C[(i + half) * stride + (j + half)] += M1[i * half + j] - M2[i * half + j] + M3[i * half + j] + M6[i * half + j];
        }
    }
    
    // Free temporary matrices
    matrix_free(A11); matrix_free(A12); matrix_free(A21); matrix_free(A22);
    matrix_free(B11); matrix_free(B12); matrix_free(B21); matrix_free(B22);
    matrix_free(M1); matrix_free(M2); matrix_free(M3); matrix_free(M4);
    matrix_free(M5); matrix_free(M6); matrix_free(M7);
    matrix_free(temp1); matrix_free(temp2);
}

void strassen_omp(double *A, double *B, double *C, int n) {
    int padded_n = next_power_of_2_omp(n);
    
    if (padded_n != n) {
        // Need padding
        double *A_padded = matrix_allocate(padded_n);
        double *B_padded = matrix_allocate(padded_n);
        double *C_padded = matrix_allocate(padded_n);
        
        matrix_zero_init(A_padded, padded_n);
        matrix_zero_init(B_padded, padded_n);
        matrix_zero_init(C_padded, padded_n);
        
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                A_padded[i * padded_n + j] = A[i * n + j];
                B_padded[i * padded_n + j] = B[i * n + j];
            }
        }
        
        // Create parallel region for tasks
        #pragma omp parallel
        {
            #pragma omp single
            {
                strassen_recursive_omp(A_padded, B_padded, C_padded, padded_n, padded_n, 1);
            }
        }
        
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
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                strassen_recursive_omp(A, B, C, n, n, 1);
            }
        }
    }
}

// ========== PROPOSED ALGORITHM (OpenMP) ==========
#define BLOCK_SIZE_OMP 64

void proposed_omp(double *A, double *B, double *C, int n) {
    // Step 1: Transpose B (serial is fine, not the bottleneck)
    double *B_T = matrix_allocate(n);
    matrix_transpose(B, B_T, n);
    
    // Step 2: Zero-initialize C
    matrix_zero_init(C, n);
    
    // Step 3: Cache-blocked multiplication with OpenMP parallelization over tiles
    // Parallelize over (ii, jj) - each thread gets a set of output tiles
    // Do NOT parallelize kk to avoid race conditions on C
    #pragma omp parallel for collapse(2) schedule(static)
    for (int ii = 0; ii < n; ii += BLOCK_SIZE_OMP) {
        for (int jj = 0; jj < n; jj += BLOCK_SIZE_OMP) {
            int i_end = (ii + BLOCK_SIZE_OMP < n) ? ii + BLOCK_SIZE_OMP : n;
            int j_end = (jj + BLOCK_SIZE_OMP < n) ? jj + BLOCK_SIZE_OMP : n;
            
            // Loop over k blocks (serial within each thread)
            for (int kk = 0; kk < n; kk += BLOCK_SIZE_OMP) {
                int k_end = (kk + BLOCK_SIZE_OMP < n) ? kk + BLOCK_SIZE_OMP : n;
                
                // Inner block computation
                for (int i = ii; i < i_end; i++) {
                    for (int j = jj; j < j_end; j++) {
                        double sum = 0.0;
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
