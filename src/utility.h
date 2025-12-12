// utility.h
// Header for utility functions
// Helper functions for matrix operations, timing, and testing

#ifndef UTILITY_H
#define UTILITY_H

#include <stdlib.h>

// matrix_allocate
// Input: n (matrix dimension > 0).
// Output: pointer to a newly allocated n*n double buffer (row-major) or NULL on failure.
double* matrix_allocate(int n);

// matrix_free
// Input: pointer previously returned by matrix_allocate (may be NULL).
// Behavior: releases heap memory.
void matrix_free(double *matrix);

// matrix_random_init
// Input: matrix pointer, dimension n.
// Behavior: fills the n*n matrix with rand()/RAND_MAX values using the current RNG seed.
void matrix_random_init(double *matrix, int n);

// matrix_zero_init
// Input: matrix pointer, dimension n.
// Behavior: sets all elements to 0.0.
void matrix_zero_init(double *matrix, int n);

// matrix_identity_init
// Behavior: writes the n x n identity matrix (1.0 on the diagonal, 0 elsewhere).
void matrix_identity_init(double *matrix, int n);

// matrix_compare
// Input: matrices A, B, dimension n, absolute tolerance.
// Behavior: returns 1 if every entry differs by <= tolerance, otherwise 0.
// Complexity: O(n^2).
int matrix_compare(double *A, double *B, int n, double tolerance);

// matrix_print
// Input: matrix pointer, dimensions n, max_size cap for printing.
// Behavior: prints up to max_size x max_size entries for debugging.
void matrix_print(double *matrix, int n, int max_size);

// get_wtime
// Output: double precision wall-clock timestamp in seconds (monotonic best-effort).
double get_wtime();

// matrix_checksum
// Behavior: returns the sum of all n*n elements (useful for quick fingerprints).
double matrix_checksum(double *matrix, int n);

// matrix_add
// Behavior: performs elementwise C = A + B over n x n matrices.
void matrix_add(double *A, double *B, double *C, int n);

// matrix_sub
// Behavior: performs elementwise C = A - B over n x n matrices.
void matrix_sub(double *A, double *B, double *C, int n);

// matrix_transpose
// Behavior: writes dst[j*n + i] = src[i*n + j] for an n x n matrix.
void matrix_transpose(double *src, double *dst, int n);

#endif // UTILITY_H
