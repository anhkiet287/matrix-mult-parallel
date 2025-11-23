// mpi_wrapper.c
// Implementation of MPI wrapper functions
// Handles distributed-memory parallelization using master-worker model

#include "mpi_wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void mpi_matmul_master_worker(double *A, double *B, double *C, int n, kernel_func_t kernel) {
    int rank = mpi_get_rank();
    int size = mpi_get_size();
    
    // Calculate rows per process
    int rows_per_proc = n / size;
    int remainder = n % size;
    
    // Allocate local buffers
    double *local_A = (double *)malloc(rows_per_proc * n * sizeof(double));
    double *local_C = (double *)malloc(rows_per_proc * n * sizeof(double));
    
    // Master broadcasts matrix B to all processes
    // All processes need full B matrix for computation
    if (rank == 0) {
        mpi_broadcast_matrix(B, n, 0);
    } else {
        mPI_Bcast(B, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    
    // Master scatters rows of A to all processes
    mpi_scatter_rows(A, local_A, n, rows_per_proc, 0);
    
    // Each process computes its portion using provided kernel
    // kernel can be: matmul_serial, matmul_omp, strassen_serial, etc.
    kernel(local_A, B, local_C, rows_per_proc);
    
    // Gather results back to master
    mpi_gather_rows(local_C, C, n, rows_per_proc, 0);
    
    // TODO: Handle remainder rows (when n is not evenly divisible by size)
    // Master should compute remaining rows separately
    
    free(local_A);
    free(local_C);
}
