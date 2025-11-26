// mpi_performance_test.c
// Measure MPI execution time and GFLOPS

#include "../src/mpi_wrapper.h"
#include "../src/utility.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const int sizes[] = {128, 256, 512, 1024};
static const int nsizes = sizeof(sizes) / sizeof(sizes[0]);

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];

        double *A = NULL;
        double *B = NULL;
        double *C = NULL;

        if (rank == 0) {
            A = matrix_allocate(n);
            B = matrix_allocate(n);
            C = matrix_allocate(n);

            matrix_random_init(A, n);
            matrix_random_init(B, n);
            matrix_zero_init(C, n);
        }

        double start = MPI_Wtime();
        mpi_matmul(A, B, C, n);
        double end = MPI_Wtime();

        if (rank == 0) {
            double t = end - start;
            double gflops = (2.0 * n * n * n) / (t * 1e9);

            printf("MPI n=%d: %.4f sec, %.2f GFLOPS\n", n, t, gflops);

            matrix_free(A);
            matrix_free(B);
            matrix_free(C);
        }
    }

    MPI_Finalize();
    return 0;
}
