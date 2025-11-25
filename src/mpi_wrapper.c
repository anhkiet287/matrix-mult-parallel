// mpi_wrapper.c
// Implementation of MPI wrapper functions
// Handles distributed-memory parallelization using master-worker model

#include "mpi_wrapper.h"
#include "kernels.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

void mpi_init(int *argc, char ***argv) {
    MPI_Init(argc, argv);
}

void mpi_finalize() {
    MPI_Finalize();
}

int mpi_get_rank() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    return rank;
}

int mpi_get_size() {
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    return size;
}

void mpi_broadcast_matrix(double *matrix, int n, int root) {
    // Broadcast entire matrix (n*n elements) from root to all processes
    MPI_Bcast(matrix, n * n, MPI_DOUBLE, root, MPI_COMM_WORLD);
}

void mpi_scatter_rows(double *matrix, double *local_matrix, int n, int local_rows, int root) {
    // Scatter rows from master to workers
    // Each process receives local_rows * n elements
    MPI_Scatter(matrix, local_rows * n, MPI_DOUBLE,
                local_matrix, local_rows * n, MPI_DOUBLE,
                root, MPI_COMM_WORLD);
}

void mpi_gather_rows(double *local_matrix, double *matrix, int n, int local_rows, int root) {
    // Gather rows from workers to master
    // Master assembles complete matrix
    MPI_Gather(local_matrix, local_rows * n, MPI_DOUBLE,
               matrix, local_rows * n, MPI_DOUBLE,
               root, MPI_COMM_WORLD);
}

// Internal helper to compute a block of rows locally.
// If the block is the full matrix (np=1), defer to the chosen kernel.
// Otherwise use a simple triple loop (OMP parallelized when available).
static void compute_block(kernel_func_t kernel,
                          double *local_A,
                          double *B,
                          double *local_C,
                          int local_rows,
                          int n) {
    if (local_rows == 0) {
        return;
    }

    // If there is only one process (or no partitioning), defer to the kernel directly.
    if (local_rows == n && kernel) {
        kernel(local_A, B, local_C, n);
        return;
    }

    int use_omp = (kernel == matmul_omp ||
                   kernel == strassen_omp ||
                   kernel == proposed_omp);

#ifdef _OPENMP
    if (use_omp) {
        #pragma omp parallel for collapse(2) schedule(static)
        for (int i = 0; i < local_rows; i++) {
            for (int j = 0; j < n; j++) {
                double sum = 0.0;
                for (int k = 0; k < n; k++) {
                    sum += local_A[i * n + k] * B[k * n + j];
                }
                local_C[i * n + j] = sum;
            }
        }
        return;
    }
#endif

    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += local_A[i * n + k] * B[k * n + j];
            }
            local_C[i * n + j] = sum;
        }
    }
}

void mpi_matmul_master_worker(double *A, double *B, double *C, int n, kernel_func_t kernel) {
    int rank = mpi_get_rank();
    int size = mpi_get_size();

    // Calculate rows per process (distribute remainder among the first ranks)
    int base_rows = n / size;
    int remainder = n % size;
    int local_rows = base_rows + (rank < remainder ? 1 : 0);
    int local_elems = local_rows * n;

    // Allocate local buffers
    double *local_A = NULL;
    double *local_C = NULL;
    if (local_elems > 0) {
        local_A = (double *)malloc(local_elems * sizeof(double));
        local_C = (double *)malloc(local_elems * sizeof(double));
        if (!local_A || !local_C) {
            fprintf(stderr, "Rank %d: failed to allocate local buffers\n", rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Prepare counts/displacements on root for scatter/gather
    // Remainder rows are assigned to the earliest ranks.
    int *counts = NULL;
    int *displs = NULL;
    if (rank == 0) {
        counts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        if (!counts || !displs) {
            fprintf(stderr, "Root: failed to allocate scatter/gather metadata\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        int offset = 0;
        for (int i = 0; i < size; i++) {
            int rows = base_rows + (i < remainder ? 1 : 0);
            counts[i] = rows * n;
            displs[i] = offset;
            offset += counts[i];
        }
    }

    // Broadcast matrix B to all processes (everyone needs full B)
    MPI_Bcast(B, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Scatter rows of A to all processes
    MPI_Scatterv(A, counts, displs, MPI_DOUBLE,
                 local_A, local_elems, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    // Each process computes its portion using provided kernel
    compute_block(kernel, local_A, B, local_C, local_rows, n);

    // Gather results back to master
    MPI_Gatherv(local_C, local_elems, MPI_DOUBLE,
                C, counts, displs, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    free(local_A);
    free(local_C);
    if (rank == 0) {
        free(counts);
        free(displs);
    }
}
