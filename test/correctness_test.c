// correctness_test.c
// Test correctness of matrix multiplication implementations
// Compares results against known-correct serial implementation

#include "../src/kernels.h"
#include "../src/omp_kernels.h"
#include "../src/utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TEST_SIZE 128
#define TOLERANCE 1e-6

int test_kernel(const char *name, void (*kernel)(double*, double*, double*, int), 
                double *A, double *B, double *expected, int n) {
    printf("Testing %s... ", name);
    
    double *C = matrix_allocate(n);
    if (C == NULL) {
        printf("FAILED (allocation)\n");
        return 0;
    }
    
    matrix_zero_init(C, n);
    kernel(A, B, C, n);
    
    int result = matrix_compare(C, expected, n, TOLERANCE);
    
    if (result) {
        printf("PASSED\n");
    } else {
        printf("FAILED (incorrect result)\n");
        printf("Expected checksum: %f\n", matrix_checksum(expected, n));
        printf("Got checksum:      %f\n", matrix_checksum(C, n));
    }
    
    matrix_free(C);
    return result;
}

int main() {
    printf("=== Matrix Multiplication Correctness Test ===\n");
    printf("Matrix size: %dx%d\n\n", TEST_SIZE, TEST_SIZE);
    
    srand(time(NULL));
    
    // Allocate matrices
    double *A = matrix_allocate(TEST_SIZE);
    double *B = matrix_allocate(TEST_SIZE);
    double *expected = matrix_allocate(TEST_SIZE);
    
    if (!A || !B || !expected) {
        fprintf(stderr, "Error: Failed to allocate test matrices\n");
        return 1;
    }
    
    // Initialize with random values
    matrix_random_init(A, TEST_SIZE);
    matrix_random_init(B, TEST_SIZE);
    
    // Compute expected result using serial implementation
    printf("Computing expected result (serial)...\n");
    matmul_serial(A, B, expected, TEST_SIZE);
    printf("Expected checksum: %f\n\n", matrix_checksum(expected, TEST_SIZE));
    
    // Test all implementations
    int passed = 0;
    int total = 0;
    
    total++;
    passed += test_kernel("matmul_serial", matmul_serial, A, B, expected, TEST_SIZE);
    
    total++;
    passed += test_kernel("matmul_omp", matmul_omp, A, B, expected, TEST_SIZE);
    
    // TODO: Add tests for strassen_serial/strassen_omp and proposed_* when implemented
    
    printf("\n=== Results: %d/%d tests passed ===\n", passed, total);
    
    // Clean up
    matrix_free(A);
    matrix_free(B);
    matrix_free(expected);
    
    return (passed == total) ? 0 : 1;
}
