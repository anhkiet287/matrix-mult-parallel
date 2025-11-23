// utility.h
// Header for utility functions
// Helper functions for matrix operations, timing, and testing

#ifndef UTILITY_H
#define UTILITY_H

#include <stdlib.h>

// Matrix memory allocation
// Allocates n*n matrix in row-major order
// Returns pointer to allocated matrix, or NULL on failure
double* matrix_allocate(int n);

// Matrix memory deallocation
void matrix_free(double *matrix);

// Initialize matrix with random values in range [0, 1]
void matrix_random_init(double *matrix, int n);

// Initialize matrix with zeros
void matrix_zero_init(double *matrix, int n);

// Initialize identity matrix
void matrix_identity_init(double *matrix, int n);

// Compare two matrices for equality within tolerance
// Returns 1 if equal, 0 otherwise
// tolerance: acceptable absolute difference (e.g., 1e-6)
int matrix_compare(double *A, double *B, int n, double tolerance);

// Print matrix to stdout (for debugging)
// max_size: maximum rows/cols to print (for large matrices)
void matrix_print(double *matrix, int n, int max_size);

// Get current wall-clock time in seconds
// For performance benchmarking
double get_wtime();

// Calculate matrix checksum for verification
// Sum of all elements (simple hash)
double matrix_checksum(double *matrix, int n);

#endif // UTILITY_H
