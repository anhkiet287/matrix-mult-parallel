// mpi_correctness_test.c
// Correctness verification for MPI matrix multiplication
// Compares MPI result with known-correct serial output

#include "../src/mpi_wrapper.h"
#include "../src/kernels.h"
#include "../src/omp_kernels.h"
#include "../src/utility.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_SIZE 128
#define TOLERANCE 1e-6

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double *A = NULL;
    double *B = NULL;
    double *expected = NULL;
    double *C_mpi = NULL;

    if (rank == 0) {
        A = matrix_allocate(TEST_SIZE);
        B = matrix_allocate(TEST_SIZE);
        expected = matrix_allocate(TEST_SIZE);
        C_mpi = matrix_allocate(TEST_SIZE);

        matrix_random_init(A, TEST_SIZE);
        matrix_random_init(B, TEST_SIZE);

        // Baseline serial
        matmul_serial(A, B, expected, TEST_SIZE);

        printf("Baseline checksum: %f\n", matrix_checksum(expected, TEST_SIZE));
    }

    // Run MPI version
    mpi_matmul(A, B, C_mpi, TEST_SIZE);

    if (rank == 0) {
        int correct = matrix_compare(expected, C_mpi, TEST_SIZE, TOLERANCE);

        printf("MPI correctness: %s\n", correct ? "PASSED" : "FAILED");

        matrix_free(A);
        matrix_free(B);
        matrix_free(expected);
        matrix_free(C_mpi);
    }

    MPI_Finalize();
    return 0;
}
