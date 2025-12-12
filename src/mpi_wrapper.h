// mpi_wrapper.h
// Header for MPI wrapper functions
// Handles distributed-memory parallelization

#ifndef MPI_WRAPPER_H
#define MPI_WRAPPER_H

#include <mpi.h>

// Type definition for kernel function pointer
// Allows pluggable kernel selection (serial/omp/strassen/proposed)
typedef void (*kernel_func_t)(double *A, double *B, double *C, int n);

// mpi_init
// Input: pointers to argc/argv from main.
// Behavior: wraps MPI_Init so all approaches share one entry point.
// Constraints: must be called exactly once before any MPI usage.
void mpi_init(int *argc, char ***argv);

// mpi_finalize
// Behavior: wraps MPI_Finalize; call once all MPI work is done.
void mpi_finalize();

// mpi_get_rank
// Output: integer rank in MPI_COMM_WORLD.
int mpi_get_rank();

// mpi_get_size
// Output: total process count participating in MPI_COMM_WORLD.
int mpi_get_size();

// mpi_matmul_master_worker
// Input:
//   A, B, C: rank-0 owns full matrices (row-major n x n); other ranks need B buffer.
//   n:      matrix dimension.
//   kernel: computation kernel to apply on local partitions (serial or OMP).
// Behavior:
//   Implements master-worker matrix multiplication:
//     * Rank 0 scatters rows of A via MPI_Scatterv (handles uneven row counts).
//     * All ranks receive full B via MPI_Bcast.
//     * Each rank multiplies its rows using the selected kernel.
//     * Partial C rows are gathered back on rank 0.
// Constraints:
//   MPI must be initialized; pointers on rank 0 must be valid buffers of size n*n.
// Complexity:
//   Communication O(n^2) per scatter/gather; computation cost depends on kernel.
void mpi_matmul_master_worker(double *A, double *B, double *C, int n, kernel_func_t kernel);

// mpi_broadcast_matrix
// Convenience wrapper around MPI_Bcast for entire n x n matrices.
void mpi_broadcast_matrix(double *matrix, int n, int root);

// mpi_scatter_rows
// Scatter fixed row blocks from root to all ranks (legacy helper).
void mpi_scatter_rows(double *matrix, double *local_matrix, int n, int local_rows, int root);

// mpi_gather_rows
// Gather fixed row blocks back to the root process (legacy helper).
void mpi_gather_rows(double *local_matrix, double *matrix, int n, int local_rows, int root);

#endif // MPI_WRAPPER_H
