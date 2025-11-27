// mpi_correctness_test.c
// Correctness test for MPI and Hybrid matrix multiplication
// Usage: mpirun -np <P> ./mpi_correctness_test <algorithm> [mpi|hybrid]

#include "../src/kernels.h"
#include "../src/mpi_wrapper.h"
#include "../src/utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_MPI_TEST_SIZE 256
#define TOL 1e-6

static int get_env_int(const char *name, int fallback) {
    const char *value = getenv(name);
    if (!value || !*value) return fallback;
    char *endptr = NULL;
    long v = strtol(value, &endptr, 10);
    if (endptr == value || v <= 0) return fallback;
    return (int)v;
}

int main(int argc, char **argv) {
    mpi_init(&argc, &argv);
    int rank = mpi_get_rank();
    int size = mpi_get_size();

    if (argc < 2 || argc > 3) {
        if (rank == 0) {
            printf("Usage: mpirun -np <P> ./mpi_correctness_test <algorithm> [mpi|hybrid]\n");
            printf("Algorithms: naive | strassen | proposed\n");
            printf("Mode (optional, default mpi): mpi | hybrid\n");
        }
        mpi_finalize();
        return 1;
    }

    char *algorithm = argv[1];
    const char *mode = (argc == 3) ? argv[2] : "mpi";
    int hybrid = 0;

    if (strcmp(mode, "mpi") == 0) {
        hybrid = 0;
    } else if (strcmp(mode, "hybrid") == 0) {
        hybrid = 1;
    } else {
        if (rank == 0) fprintf(stderr, "Unknown mode: %s (use mpi|hybrid)\n", mode);
        mpi_finalize();
        return 1;
    }

    // Select kernel (serial or OMP worked inside wrapper)
    kernel_func_t kernel = NULL;
    if (strcmp(algorithm, "naive") == 0) {
        kernel = hybrid ? matmul_omp : matmul_serial;
    } else if (strcmp(algorithm, "strassen") == 0) {
        kernel = hybrid ? strassen_omp : strassen_serial;
    } else if (strcmp(algorithm, "proposed") == 0) {
        kernel = hybrid ? proposed_omp : proposed_serial;
    }
    else {
        if (rank == 0) fprintf(stderr, "Unknown algorithm: %s\n", algorithm);
        mpi_finalize();
        return 1;
    }

    int test_size = get_env_int("MPI_TEST_SIZE", DEFAULT_MPI_TEST_SIZE);

    double *A = NULL;
    double *B = NULL;
    double *C = NULL;

    // Rank 0 allocates full matrices
    if (rank == 0) {
        A = matrix_allocate(test_size);
        B = matrix_allocate(test_size);
        C = matrix_allocate(test_size);

        srand(42);
        matrix_random_init(A, test_size);
        srand(123);
        matrix_random_init(B, test_size);
        matrix_zero_init(C, test_size);
    } else {
        // Non-root processes allocate only B for broadcast
        B = matrix_allocate(test_size);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Perform distributed multiplication
    mpi_matmul_master_worker(A, B, C, test_size, kernel);

    // Only rank 0 evaluates correctness
    if (rank == 0) {
        double *reference = matrix_allocate(test_size);
        matrix_zero_init(reference, test_size);

        matmul_serial(A, B, reference, test_size);

        int ok = matrix_compare(C, reference, test_size, TOL);

        printf("\n=== MPI Correctness Test (%s, %s mode) ===\n", algorithm,
               hybrid ? "hybrid" : "mpi");
        printf("Processes: %d\n", size);

        if (ok) {
            printf("Result: PASSED ✓\n");
        } else {
            printf("Result: FAILED ✗\n");
            printf("Reference checksum: %f\n", matrix_checksum(reference, test_size));
            printf("MPI result checksum: %f\n", matrix_checksum(C, test_size));
        }

        matrix_free(reference);
    }

    if (rank == 0) {
        matrix_free(A);
        matrix_free(C);
    }
    matrix_free(B);

    mpi_finalize();
    return 0;
}
