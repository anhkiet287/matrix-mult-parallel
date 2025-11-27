// correctness_test.c
// Test correctness of matrix multiplication implementations
// Compares results against known-correct serial implementation

#include "../src/kernels.h"
#include "../src/omp_kernels.h"
#include "../src/utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define TEST_SIZE 128
#define TOLERANCE 1e-6
int is_placeholder(double *C_test, double *C_ref, int n) {
    return matrix_compare(C_test, C_ref, n, 1e-12); 
}

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
void run_single_test(const char *name,
                     void (*kernel)(double*, double*, double*, int),
                     double *A, double *B, double *baseline, int n,
                     int *total, int *passed)
{
    printf("Testing %-20s ... ", name);
    (*total)++;

    double *C = matrix_allocate(n);
    matrix_zero_init(C, n);

    kernel(A, B, C, n);
    int ok = matrix_compare(C, baseline, n, TOLERANCE);

    if (ok) {
        printf("PASSED");

        // Detect placeholder (Strassen/proposed not implemented)
        if (strcmp(name, "strassen_serial") == 0 ||
            strcmp(name, "strassen_omp") == 0 ||
            strcmp(name, "proposed_serial") == 0 ||
            strcmp(name, "proposed_omp") == 0)
        {
            if (is_placeholder(C, baseline, n)) {
                printf("  ⚠ WARNING: placeholder implementation detected\n");
            } else {
                printf("\n");
            }
        } else {
            printf("\n");
        }

        (*passed)++;
    } 
    else {
        printf("FAILED ❌\n");
        printf("  Expected checksum: %f\n", matrix_checksum(baseline, n));
        printf("  Got checksum     : %f\n", matrix_checksum(C, n));
    }

    matrix_free(C);
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
    
    // total++;
    // passed += test_kernel("matmul_serial", matmul_serial, A, B, expected, TEST_SIZE);
    
    // total++;
    // passed += test_kernel("matmul_omp", matmul_omp, A, B, expected, TEST_SIZE);
    
    // TODO: Add tests for strassen_serial/strassen_omp and proposed_* when implemented
     // === NAIVE ===
    run_single_test("matmul_serial",       matmul_serial,  A, B, expected, TEST_SIZE, &total, &passed);
    run_single_test("matmul_omp",          matmul_omp,     A, B, expected, TEST_SIZE, &total, &passed);

    // === STRASSEN ===
    run_single_test("strassen_serial",     strassen_serial, A, B, expected, TEST_SIZE, &total, &passed);
    run_single_test("strassen_omp",        strassen_omp,    A, B, expected, TEST_SIZE, &total, &passed);

    // === PROPOSED ===
    run_single_test("proposed_serial",     proposed_serial, A, B, expected, TEST_SIZE, &total, &passed);
    run_single_test("proposed_omp",        proposed_omp,    A, B, expected, TEST_SIZE, &total, &passed);

    printf("\n=== Results: %d/%d tests passed ===\n", passed, total);
    
    // Clean up
    matrix_free(A);
    matrix_free(B);
    matrix_free(expected);
    
    return (passed == total) ? 0 : 1;
}
