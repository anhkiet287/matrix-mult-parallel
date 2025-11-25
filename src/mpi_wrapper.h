// mpi_wrapper.h
// Header for MPI wrapper functions
// Handles distributed-memory parallelization

#ifndef MPI_WRAPPER_H
#define MPI_WRAPPER_H

#include <mpi.h>

// Type definition for kernel function pointer
// Allows pluggable kernel selection (serial/omp/strassen/proposed)
typedef void (*kernel_func_t)(double *A, double *B, double *C, int n);

// Initialize MPI environment
// Call this before any MPI operations
void mpi_init(int *argc, char ***argv);

// Finalize MPI environment
// Call this before program exits
void mpi_finalize();

// Get current MPI rank (process ID)
int mpi_get_rank();

// Get total number of MPI processes
int mpi_get_size();

// Master-worker distributed matrix multiplication
// Master (rank 0) distributes work to all workers
// Each worker computes partial result using provided kernel
// Note: Partial blocks currently use the naive triple-loop; if only one
// process participates (np=1), the provided kernel is used directly.
// Master collects and assembles final result
// Parameters:
//   A, B, C: matrices (only master needs full A, B; all need C buffer)
//   n: matrix dimension
//   kernel: function pointer to computation kernel
void mpi_matmul_master_worker(double *A, double *B, double *C, int n, kernel_func_t kernel);

// Broadcast matrix from master to all processes
void mpi_broadcast_matrix(double *matrix, int n, int root);

// Scatter matrix rows from master to workers
// Each process receives a subset of rows
void mpi_scatter_rows(double *matrix, double *local_matrix, int n, int local_rows, int root);

// Gather matrix rows from workers to master
// Master assembles complete matrix from all processes
void mpi_gather_rows(double *local_matrix, double *matrix, int n, int local_rows, int root);

#endif // MPI_WRAPPER_H
