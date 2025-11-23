// kernels.h
// Function prototypes for matrix multiplication kernels

#ifndef KERNELS_H
#define KERNELS_H

// ========== Naive Matrix Multiplication ==========

// Naive matrix multiply (serial)
// Arguments: double *A, *B, *C - pointers to matrices (row-major, size n*n)
//            int n - matrix dimension
// Output: Result stored in C
void matmul_serial(double *A, double *B, double *C, int n);

// Naive matrix multiply (OpenMP)
void matmul_omp(double *A, double *B, double *C, int n);

// ========== Strassen Algorithm ==========

// Strassen matrix multiply (serial)
// Note: n should be power of 2, or padded
void strassen_serial(double *A, double *B, double *C, int n);

// Strassen matrix multiply (OpenMP)
void strassen_omp(double *A, double *B, double *C, int n);

// ========== Proposed Algorithm ==========

// Proposed algorithm (serial)
void proposed_serial(double *A, double *B, double *C, int n);

// Proposed algorithm (OpenMP)
void proposed_omp(double *A, double *B, double *C, int n);

#endif // KERNELS_H
