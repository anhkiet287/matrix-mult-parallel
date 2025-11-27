// correctness_test.c
// Test correctness of matrix multiplication implementations.
// Compares results against known-correct serial implementation.

#include "../src/kernels.h"
#include "../src/omp_kernels.h"
#include "../src/utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define DEFAULT_TEST_SIZE 256
#define DEFAULT_TOLERANCE 1e-6

static int get_env_int(const char *name, int fallback) {
    const char *value = getenv(name);
    if (!value || !*value) {
        return fallback;
    }
    char *endptr = NULL;
    long v = strtol(value, &endptr, 10);
    if (endptr == value || v <= 0) {
        return fallback;
    }
    return (int)v;
}

static double get_env_double(const char *name, double fallback) {
    const char *value = getenv(name);
    if (!value || !*value) {
        return fallback;
    }
    char *endptr = NULL;
    double v = strtod(value, &endptr);
    if (endptr == value || v <= 0.0) {
        return fallback;
    }
    return v;
}

static int kernel_enabled(const char *list, const char *name) {
    if (list == NULL || *list == '\0') {
        return 1;
    }
    char *copy = strdup(list);
    if (!copy) {
        return 1;
    }
    int enabled = 0;
    char *token = strtok(copy, ", ");
    while (token) {
        if (strcmp(token, name) == 0) {
            enabled = 1;
            break;
        }
        token = strtok(NULL, ", ");
    }
    free(copy);
    return enabled;
}

typedef struct {
    const char *name;
    void (*fn)(double*, double*, double*, int);
} kernel_entry;

static void run_single_test(const kernel_entry *entry,
                            double *A, double *B, double *baseline, int n,
                            double tol, const char *enabled_list,
                            int *total, int *passed)
{
    if (!kernel_enabled(enabled_list, entry->name)) {
        return;
    }

    printf("Testing %-20s ... ", entry->name);
    (*total)++;

    double *C = matrix_allocate(n);
    matrix_zero_init(C, n);

    entry->fn(A, B, C, n);
    int ok = matrix_compare(C, baseline, n, tol);

    if (ok) {
        printf("PASSED\n");
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
    
    int test_size = get_env_int("TEST_CORRECTNESS_SIZE", DEFAULT_TEST_SIZE);
    double tol = get_env_double("TEST_CORRECTNESS_TOLERANCE", DEFAULT_TOLERANCE);
    const char *kernel_list = getenv("CORRECTNESS_KERNELS");

    printf("Matrix size: %dx%d\n", test_size, test_size);
    printf("Tolerance  : %.2e\n\n", tol);
    
    srand(time(NULL));
    
    // Allocate matrices
    double *A = matrix_allocate(test_size);
    double *B = matrix_allocate(test_size);
    double *expected = matrix_allocate(test_size);
    
    if (!A || !B || !expected) {
        fprintf(stderr, "Error: Failed to allocate test matrices\n");
        return 1;
    }
    
    // Initialize with random values
    matrix_random_init(A, test_size);
    matrix_random_init(B, test_size);
    
    // Compute expected result using serial implementation
    printf("Computing expected result (serial)...\n");
    matmul_serial(A, B, expected, test_size);
    printf("Expected checksum: %f\n\n", matrix_checksum(expected, test_size));
    
    // Test all implementations
    int passed = 0;
    int total = 0;

    const kernel_entry kernels[] = {
        {"matmul_serial", matmul_serial},
        {"matmul_omp", matmul_omp},
        {"strassen_serial", strassen_serial},
        {"strassen_omp", strassen_omp},
        {"proposed_serial", proposed_serial},
        {"proposed_omp", proposed_omp}
    };
    const size_t kernel_count = sizeof(kernels) / sizeof(kernels[0]);

    for (size_t i = 0; i < kernel_count; i++) {
        run_single_test(&kernels[i], A, B, expected, test_size, tol,
                        kernel_list, &total, &passed);
    }

    printf("\n=== Results: %d/%d tests passed ===\n", passed, total);
    
    // Clean up
    matrix_free(A);
    matrix_free(B);
    matrix_free(expected);
    
    return (passed == total) ? 0 : 1;
}
