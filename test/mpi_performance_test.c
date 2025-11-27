// mpi_performance_test.c
// Benchmark MPI and Hybrid matrix multiplication
// Run with: mpirun -np <P> ./mpi_performance_test <algorithm> <mode>
// mode: mpi | hybrid

#include "../src/kernels.h"
#include "../src/mpi_wrapper.h"
#include "../src/utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Defaults; override via env:
// MPI_PERF_SIZES="256,512,1024", MPI_PERF_RUNS=1
static const int default_sizes[] = {128, 256, 512, 1024};
static const int default_num_sizes = sizeof(default_sizes) / sizeof(default_sizes[0]);
static int num_runs = 1;

// Parse comma-separated sizes from env. Caller must free *out if non-NULL.
static void load_sizes_from_env(const char *env_var, int **out, int *count) {
    const char *val = getenv(env_var);
    if (!val || !*val) {
        *out = NULL;
        *count = 0;
        return;
    }

    int commas = 0;
    for (const char *p = val; *p; p++) if (*p == ',') commas++;
    int max_sizes = commas + 1;
    int *sizes = (int *)malloc(max_sizes * sizeof(int));
    if (!sizes) {
        fprintf(stderr, "Warning: failed to allocate sizes array, using defaults.\n");
        *out = NULL;
        *count = 0;
        return;
    }

    int idx = 0;
    const char *start = val;
    char *end = NULL;
    while (*start && idx < max_sizes) {
        long v = strtol(start, &end, 10);
        if (start == end) break;
        if (v > 0) sizes[idx++] = (int)v;
        if (*end == ',') end++;
        start = end;
    }

    if (idx == 0) {
        free(sizes);
        sizes = NULL;
    }

    *out = sizes;
    *count = idx;
}

int main(int argc, char **argv) {
    mpi_init(&argc, &argv);
    int rank = mpi_get_rank();
    int size = mpi_get_size();

    if (argc != 3) {
        if (rank == 0) {
            printf("Usage: mpirun -np <P> ./mpi_performance_test <algorithm> <mode>\n");
            printf("Algorithms: naive | strassen | proposed\n");
            printf("Mode: mpi | hybrid\n");
        }
        mpi_finalize();
        return 1;
    }

    char *algorithm = argv[1];
    char *mode = argv[2];

    // Choose kernel: if hybrid → use OMP version
    kernel_func_t kernel = NULL;

    if (strcmp(mode, "mpi") == 0) {
        if      (strcmp(algorithm, "naive") == 0)      kernel = matmul_serial;
        else if (strcmp(algorithm, "strassen") == 0)    kernel = strassen_serial;
        else if (strcmp(algorithm, "proposed") == 0)    kernel = proposed_serial;
    }
    else if (strcmp(mode, "hybrid") == 0) {
        if      (strcmp(algorithm, "naive") == 0)      kernel = matmul_omp;
        else if (strcmp(algorithm, "strassen") == 0)    kernel = strassen_omp;
        else if (strcmp(algorithm, "proposed") == 0)    kernel = proposed_omp;
    }
    else {
        if (rank == 0) fprintf(stderr, "Unknown mode: %s\n", mode);
        mpi_finalize();
        return 1;
    }

    double *A = NULL;
    double *B = NULL;
    double *C = NULL;

    if (rank == 0) {
        printf("=== MPI Performance Benchmark (%s, %s) ===\n", algorithm, mode);
        printf("Processes: %d\n\n", size);
    }

    const char *runs_env = getenv("MPI_PERF_RUNS");
    if (runs_env && *runs_env) {
        int r = atoi(runs_env);
        if (r > 0) num_runs = r;
    }

    int *sizes = NULL;
    int num_sizes = 0;
    load_sizes_from_env("MPI_PERF_SIZES", &sizes, &num_sizes);
    const int *sizes_to_use = sizes ? sizes : default_sizes;
    int sizes_count = sizes ? num_sizes : default_num_sizes;

    for (int t = 0; t < sizes_count; t++) {
        int n = sizes_to_use[t];

        if (rank == 0) {
            A = matrix_allocate(n);
            B = matrix_allocate(n);
            C = matrix_allocate(n);

            srand(42);
            matrix_random_init(A, n);
            srand(123);
            matrix_random_init(B, n);
            matrix_zero_init(C, n);
        } else {
            B = matrix_allocate(n);
        }

        MPI_Barrier(MPI_COMM_WORLD);

        double total = 0.0;
        for (int run = 0; run < num_runs; run++) {
            double start = MPI_Wtime();
            mpi_matmul_master_worker(A, B, C, n, kernel);
            double end = MPI_Wtime();
            total += (end - start);
        }

        double sec = total / num_runs;

        if (rank == 0) {
            double gflops = (2.0 * n * n * n) / (sec * 1e9);

            printf("Matrix %4dx%-4d: %8.4f sec, %8.2f GFLOPS\n", n, n, sec, gflops);

            matrix_free(A);
            matrix_free(B);
            matrix_free(C);
        } else {
            matrix_free(B);
        }
    }

    if (rank == 0) printf("\nBenchmark done.\n");

    free(sizes);
    mpi_finalize();
    return 0;
}
