// kernels.h
// Function prototypes for matrix multiplication kernels

#ifndef KERNELS_H
#define KERNELS_H

// ========== Naive Matrix Multiplication ==========

// matmul_serial
// Input:
//   A, B: pointers to n x n matrices stored in row-major layout.
//   C:    pointer to an n x n output matrix buffer (may be uninitialized).
//   n:    matrix dimension (>0).
// Behavior:
//   Computes C = A * B using the classic triple-loop GEMM.
// Constraints:
//   All pointers must be non-NULL and point to valid buffers of size n*n.
// Complexity:
//   Time O(n^3), space O(1) extra.
void matmul_serial(double *A, double *B, double *C, int n);

// matmul_omp
// Same contract as matmul_serial but parallelizes the outer loop with OpenMP.
// Complexity:
//   Time O(n^3) work divided across threads, space O(1) extra.
void matmul_omp(double *A, double *B, double *C, int n);

// ========== Strassen Algorithm ==========

// strassen_serial
// Input:
//   A, B, C: n x n matrices (row-major).
//   n:      matrix dimension; non powers of two are padded internally.
// Behavior:
//   Computes C = A * B using Strassen recursion with a naive base case (n<=64).
// Constraints:
//   Matrices must be square; padding is handled internally.
// Complexity:
//   Time O(n^log2(7)), space O(n^2) for temporaries.
void strassen_serial(double *A, double *B, double *C, int n);

// strassen_omp
// Parallel Strassen implementation that uses OpenMP tasks on large subproblems.
// Follows the same semantics and constraints as strassen_serial.
void strassen_omp(double *A, double *B, double *C, int n);

// ========== Proposed Algorithm ==========

// proposed_serial
// Input/Behavior:
//   Performs cache-blocked GEMM with a transposed copy of B to improve locality.
// Constraints:
//   Matrices are n x n in row-major layout; n > 0.
// Complexity:
//   Time O(n^3) with improved constants; extra space O(n^2) for B^T.
void proposed_serial(double *A, double *B, double *C, int n);

// proposed_omp
// OpenMP variant of proposed_serial that parallelizes across output tiles.
void proposed_omp(double *A, double *B, double *C, int n);

// BLAS baseline (optional; requires USE_CBLAS to link against CBLAS).
void matmul_blas(double *A, double *B, double *C, int n);
int matmul_blas_available(void);
const char *matmul_blas_backend(void);

#endif // KERNELS_H
