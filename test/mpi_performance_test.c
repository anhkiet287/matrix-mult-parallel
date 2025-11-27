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

static const int test_sizes[] = {256, 512, 1024};
static const int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);

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

    for (int t = 0; t < num_sizes; t++) {
        int n = test_sizes[t];

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
        double start = MPI_Wtime();

        mpi_matmul_master_worker(A, B, C, n, kernel);

        MPI_Barrier(MPI_COMM_WORLD);
        double end = MPI_Wtime();

        if (rank == 0) {
            double sec = end - start;
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

    mpi_finalize();
    return 0;
}
