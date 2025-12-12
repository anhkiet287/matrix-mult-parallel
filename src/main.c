/**
 * main.c
 * Entry point for matrix multiplication parallel computing project
 * Supports: Serial, OpenMP, MPI, Hybrid (MPI+OpenMP)
 * Algorithms: Naive, Strassen, Proposed
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "kernels.h"
#include "mpi_wrapper.h"
#include "utility.h"

void print_usage(const char *prog_name) {
    printf("Usage: %s <size> <approach> <algorithm>\n", prog_name);
    printf("\nArguments:\n");
    printf("  size       : Matrix size (N x N)\n");
    printf("  approach   : serial | openmp | mpi | hybrid\n");
    printf("  algorithm  : naive | strassen | proposed | blas\n");
    printf("\nExamples:\n");
    printf("  %s 100 serial naive\n", prog_name);
    printf("  %s 500 openmp strassen\n", prog_name);
    printf("  mpirun -np 4 %s 1000 mpi naive\n", prog_name);
    printf("  mpirun -np 4 %s 1000 hybrid naive\n", prog_name);
}

int main(int argc, char **argv) {
    int rank = 0, size = 1;
    double start_time, end_time;
    
    // Initialize MPI (serial/openmp also run under a single MPI rank)
    mpi_init(&argc, &argv);
    rank = mpi_get_rank();
    size = mpi_get_size();
    
    // Parse arguments
    if (argc != 4) {
        if (rank == 0) {
            print_usage(argv[0]);
        }
        mpi_finalize();
        return 1;
    }
     
    int n = atoi(argv[1]); // size of matrix
    char *approach = argv[2]; // name of approach (serial, openmp, mpi, hybrid)
    char *algorithm = argv[3]; // name of algorithm (naive, strassen, proposed)
    
    // Validate input
    if (n <= 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: Matrix size must be positive\n");
        }
        mpi_finalize();
        return 1;
    }
    
    // Print configuration (rank 0 only)
    if (rank == 0) {
        printf("=================================================\n");
        printf("Matrix Multiplication - Parallel Computing\n");
        printf("=================================================\n");
        printf("Matrix size    : %d x %d\n", n, n);
        printf("Approach       : %s\n", approach);
        printf("Algorithm      : %s\n", algorithm);
        printf("MPI processes  : %d\n", size);
        printf("=================================================\n\n");
    }
    
    // Allocate matrices
    double *A = NULL, *B = NULL, *C = NULL, *C_ref = NULL;
    
    if (rank == 0) {
        A = matrix_allocate(n);
        B = matrix_allocate(n);
        C = matrix_allocate(n);
        
        // Initialize matrices with fixed seeds for reproducibility
        srand(42);
        matrix_random_init(A, n);
        srand(123);
        matrix_random_init(B, n);
        matrix_zero_init(C, n);
        
        if (rank == 0) {
            printf("Matrices initialized.\n");
            
            // Print small matrices for verification (if n <= 10)
            if (n <= 10) {
                printf("\nMatrix A:\n");
                matrix_print(A, n, n);
                printf("\nMatrix B:\n");
                matrix_print(B, n, n);
            }
        }
    } else {
        // Non-root processes need B for broadcast
        B = matrix_allocate(n);
    }
    
    // Select kernel function based on algorithm and approach
    kernel_func_t kernel = NULL;
    
    if (strcmp(approach, "serial") == 0 || strcmp(approach, "mpi") == 0) {
        // Use serial kernels for serial and MPI approaches
        if (strcmp(algorithm, "naive") == 0) {
            kernel = matmul_serial;
        } else if (strcmp(algorithm, "strassen") == 0) {
            kernel = strassen_serial;
        } else if (strcmp(algorithm, "proposed") == 0) {
            kernel = proposed_serial;
        } else if (strcmp(algorithm, "blas") == 0 && strcmp(approach, "serial") == 0) {
            if (!matmul_blas_available()) {
                if (rank == 0) {
                    fprintf(stderr, "Error: BLAS baseline unavailable (rebuild with USE_OPENBLAS=1)\n");
                }
                mpi_finalize();
                return 1;
            }
            kernel = matmul_blas;
        } else {
            if (rank == 0) {
                fprintf(stderr, "Error: Unknown algorithm '%s'\n", algorithm);
            }
            mpi_finalize();
            return 1;
        }
    } else if (strcmp(approach, "openmp") == 0 || strcmp(approach, "hybrid") == 0) {
        // Use OpenMP kernels (stubs for now, Member 1 will implement)
        if (strcmp(algorithm, "naive") == 0) {
            kernel = matmul_omp;
        } else if (strcmp(algorithm, "strassen") == 0) {
            kernel = strassen_omp;
        } else if (strcmp(algorithm, "proposed") == 0) {
            kernel = proposed_omp;
        } else if (strcmp(algorithm, "blas") == 0) {
            if (rank == 0) {
                fprintf(stderr, "Error: BLAS baseline is available only in serial mode\n");
            }
            mpi_finalize();
            return 1;
        } else {
            if (rank == 0) {
                fprintf(stderr, "Error: Unknown algorithm '%s'\n", algorithm);
            }
            mpi_finalize();
            return 1;
        }
    } else {
        if (rank == 0) {
            fprintf(stderr, "Error: Unknown approach '%s'\n", approach);
        }
        mpi_finalize();
        return 1;
    }
    
    // Synchronize before timing
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    
    // Execute based on approach
    if (strcmp(approach, "serial") == 0) {
        // Serial execution (only rank 0)
        if (rank == 0) {
            kernel(A, B, C, n);
        }
    } else if (strcmp(approach, "openmp") == 0) {
        // OpenMP execution (only rank 0, but uses threads internally)
        if (rank == 0) {
            kernel(A, B, C, n);
        }
    } else if (strcmp(approach, "mpi") == 0) {
        // MPI distributed execution
        mpi_matmul_master_worker(A, B, C, n, kernel);
    } else if (strcmp(approach, "hybrid") == 0) {
        // Hybrid MPI + OpenMP
        mpi_matmul_master_worker(A, B, C, n, kernel);
    }
    
    // Synchronize after computation
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    // Print results (rank 0 only)
    if (rank == 0) {
        double elapsed = end_time - start_time;
        
        printf("\n=================================================\n");
        printf("Computation completed!\n");
        printf("=================================================\n");
        printf("Elapsed time   : %.6f seconds\n", elapsed);
        printf("Performance    : %.2f GFLOPS\n", 
               (2.0 * n * n * n) / (elapsed * 1e9));
        printf("=================================================\n\n");
        
        // Print result matrix if small
        if (n <= 10) {
            printf("Result matrix C:\n");
            matrix_print(C, n, n);
            printf("\n");
        }
        
        // Verify correctness by comparing with serial naive
        if (strcmp(approach, "serial") == 0 && strcmp(algorithm, "naive") == 0) {
            printf("Baseline (serial naive) - no verification needed.\n");
        } else {
            printf("Computing reference result for verification...\n");
            C_ref = matrix_allocate(n);
            matrix_zero_init(C_ref, n);
            
            double ref_start = MPI_Wtime();
            matmul_serial(A, B, C_ref, n);
            double ref_end = MPI_Wtime();
            
            printf("Reference computed in %.6f seconds\n", ref_end - ref_start);
            
            // Compare results
            if (matrix_compare(C, C_ref, n, 1e-6)) {
                printf("✓ CORRECTNESS CHECK PASSED\n");
            } else {
                printf("✗ CORRECTNESS CHECK FAILED\n");
                
                // Print differences if matrix is small
                if (n <= 10) {
                    printf("\nExpected (reference):\n");
                    matrix_print(C_ref, n, n);
                    printf("\nGot (computed):\n");
                    matrix_print(C, n, n);
                }
            }
            
            matrix_free(C_ref);
        }
    }
    
    // Cleanup
    if (rank == 0) {
        if (A) matrix_free(A);
        if (C) matrix_free(C);
    }
    if (B) matrix_free(B);
    
    mpi_finalize();
    return 0;
}
