// mpi_correctness_test.c
// Correctness test for MPI and Hybrid matrix multiplication
// Must be run with: mpirun -np <P> ./mpi_correctness_test <algorithm>

#include "../src/kernels.h"
#include "../src/mpi_wrapper.h"
#include "../src/utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_SIZE 256
#define TOL 1e-6

int main(int argc, char **argv) {
    mpi_init(&argc, &argv);
    int rank = mpi_get_rank();
    int size = mpi_get_size();

    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: mpirun -np <P> ./mpi_correctness_test <algorithm>\n");
            printf("Algorithms: naive | strassen | proposed\n");
        }
        mpi_finalize();
        return 1;
    }

    char *algorithm = argv[1];

    // Select kernel (serial or OMP worked inside wrapper)
    kernel_func_t kernel = NULL;
    if (strcmp(algorithm, "naive") == 0)      kernel = matmul_serial;
    else if (strcmp(algorithm, "strassen") == 0) kernel = strassen_serial;
    else if (strcmp(algorithm, "proposed") == 0) kernel = proposed_serial;
    else {
        if (rank == 0) fprintf(stderr, "Unknown algorithm: %s\n", algorithm);
        mpi_finalize();
        return 1;
    }

    double *A = NULL;
    double *B = NULL;
    double *C = NULL;

    // Rank 0 allocates full matrices
    if (rank == 0) {
        A = matrix_allocate(TEST_SIZE);
        B = matrix_allocate(TEST_SIZE);
        C = matrix_allocate(TEST_SIZE);

        srand(42);
        matrix_random_init(A, TEST_SIZE);
        srand(123);
        matrix_random_init(B, TEST_SIZE);
        matrix_zero_init(C, TEST_SIZE);
    } else {
        // Non-root processes allocate only B for broadcast
        B = matrix_allocate(TEST_SIZE);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Perform distributed multiplication
    mpi_matmul_master_worker(A, B, C, TEST_SIZE, kernel);

    // Only rank 0 evaluates correctness
    if (rank == 0) {
        double *reference = matrix_allocate(TEST_SIZE);
        matrix_zero_init(reference, TEST_SIZE);

        matmul_serial(A, B, reference, TEST_SIZE);

        int ok = matrix_compare(C, reference, TEST_SIZE, TOL);

        printf("\n=== MPI Correctness Test (%s) ===\n", algorithm);
        printf("Processes: %d\n", size);

        if (ok) {
            printf("Result: PASSED ✓\n");
        } else {
            printf("Result: FAILED ✗\n");
            printf("Reference checksum: %f\n", matrix_checksum(reference, TEST_SIZE));
            printf("MPI result checksum: %f\n", matrix_checksum(C, TEST_SIZE));
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
