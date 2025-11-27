// performance_test.c
// Benchmark performance of matrix multiplication implementations
// Measures execution time and calculates GFLOPS for different matrix sizes

#include "../src/kernels.h"
#include "../src/omp_kernels.h"
#include "../src/utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct {
    const char *name;
    void (*fn)(double*, double*, double*, int);
} kernel_entry;

static int get_env_int(const char *name, int fallback) {
    const char *val = getenv(name);
    if (!val || !*val) return fallback;
    char *end = NULL;
    long v = strtol(val, &end, 10);
    if (end == val || v <= 0) return fallback;
    return (int)v;
}

static int parse_kernel_enabled(const char *list, const char *name) {
    if (!list || !*list) return 1;
    char *copy = strdup(list);
    if (!copy) return 1;
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

static int *parse_sizes(const char *env_var, int *count) {
    const char *val = getenv(env_var);
    if (!val || !*val) {
        *count = 0;
        return NULL;
    }

    int seps = 0;
    for (const char *p = val; *p; p++) {
        if (*p == ',' || *p == ' ') seps++;
    }

    int max = seps + 1;
    int *sizes = (int *)malloc(max * sizeof(int));
    if (!sizes) {
        *count = 0;
        return NULL;
    }

    int idx = 0;
    const char *start = val;
    char *end = NULL;
    while (*start && idx < max) {
        long v = strtol(start, &end, 10);
        if (start == end) break;
        if (v > 0) sizes[idx++] = (int)v;
        if (*end == ',' || *end == ' ') end++;
        start = end;
    }

    if (idx == 0) {
        free(sizes);
        *count = 0;
        return NULL;
    }

    *count = idx;
    return sizes;
}

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
    
    int num_runs = get_env_int("TEST_PERFORMANCE_RUNS", 5);
    double total_time = 0.0;
    for (int run = 0; run < num_runs; run++) {
        matrix_zero_init(C, n);
        
        double start = get_wtime();
        kernel(A, B, C, n);
        double end = get_wtime();
        
        total_time += (end - start);
    }
    
    double avg_time = total_time / num_runs;
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

    int num_sizes = 0;
    int *sizes = parse_sizes("TEST_PERFORMANCE_SIZES", &num_sizes);
    if (num_sizes == 0 || !sizes) {
        fprintf(stderr, "Error: TEST_PERFORMANCE_SIZES is not set or invalid.\n");
        free(sizes);
        return 1;
    }

    const char *kernel_list = getenv("PERFORMANCE_KERNELS");
    const kernel_entry kernels[] = {
        {"matmul_serial", matmul_serial},
        {"matmul_omp", matmul_omp},
        {"strassen_serial", strassen_serial},
        {"strassen_omp", strassen_omp},
        {"proposed_serial", proposed_serial},
        {"proposed_omp", proposed_omp}
    };
    const size_t kernel_count = sizeof(kernels) / sizeof(kernels[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        int n = sizes[i];
        printf("Matrix size: %dx%d\n", n, n);
        
        for (size_t k = 0; k < kernel_count; k++) {
            if (parse_kernel_enabled(kernel_list, kernels[k].name)) {
                benchmark_kernel(kernels[k].name, kernels[k].fn, n);
            }
        }

        printf("\n");
    }

    free(sizes);
    
    printf("Benchmark complete.\n");
    return 0;
}
