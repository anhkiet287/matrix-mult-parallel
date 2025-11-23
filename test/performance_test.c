// performance_test.c
// Benchmark performance of matrix multiplication implementations
// Measures execution time and calculates GFLOPS for different matrix sizes

#include "../src/kernels.h"
#include "../src/omp_kernels.h"
#include "../src/utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_RUNS 5

static const int test_sizes[] = {128, 256, 512, 1024, 2048};
static const int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);

void benchmark_kernel(const char *name, void (*kernel)(double*, double*, double*, int), int n) {
    double *A = matrix_allocate(n);
    double *B = matrix_allocate(n);
    double *C = matrix_allocate(n);
    
    if (!A || !B || !C) {
        fprintf(stderr, "Error: Failed to allocate matrices\n");
        goto cleanup;
    }
    
    matrix_random_init(A, n);
    matrix_random_init(B, n);
    matrix_zero_init(C, n);
    
    // Warmup run
    kernel(A, B, C, n);
    
    // Benchmark runs
    double total_time = 0.0;
    for (int run = 0; run < NUM_RUNS; run++) {
        matrix_zero_init(C, n);
        
        double start = get_wtime();
        kernel(A, B, C, n);
        double end = get_wtime();
        
        total_time += (end - start);
    }
    
    double avg_time = total_time / NUM_RUNS;
    double gflops = (2.0 * n * n * n) / (avg_time * 1e9); // 2n^3 operations
    
    printf("  %-20s: %8.4f sec, %8.2f GFLOPS\n", name, avg_time, gflops);
    
cleanup:
    matrix_free(A);
    matrix_free(B);
    matrix_free(C);
}

int main() {
    printf("=== Matrix Multiplication Performance Benchmark ===\n\n");
    
    srand(time(NULL));
    
    for (int i = 0; i < num_sizes; i++) {
        int n = test_sizes[i];
        printf("Matrix size: %dx%d\n", n, n);
        
        benchmark_kernel("matmul_serial", matmul_serial, n);
        benchmark_kernel("matmul_omp", matmul_omp, n);
        // Add more benchmarks when implemented
        
        printf("\n");
    }
    
    printf("Benchmark complete.\n");
    return 0;
}
