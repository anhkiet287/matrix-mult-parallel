# Design Notes - Matrix Multiplication Parallel Computing

## Repository Overview
Parallel matrix multiplication implementation using OpenMP, MPI, and hybrid approaches.
Supports naive, Strassen, and proposed algorithms.

## Repository Structure

```
matrix-mult-parallel/
├── README.md
├── src/
│   ├── main.c           # Entry point
│   ├── kernels.c/h      # Core algorithms (serial + OpenMP)
│   ├── omp_kernels.c/h  # OpenMP implementations
│   ├── mpi_wrapper.c/h  # MPI scatter/gather/wrapper
│   └── utility.c/h      # Helper functions
├── test/
│   ├── correctness_test.c
│   └── performance_test.c
├── scripts/
│   ├── run_tests.sh
│   └── generate_matrix.py
└── docs/
    ├── flowcharts/
    └── design_notes.md (this file)
```

## Member Responsibilities

### Member 1: OpenMP & Kernel Core
- Implement: `matmul_omp()`, `strassen_omp()`, `proposed_omp()`
- File: `src/omp_kernels.c`
- Update: `src/kernels.h` with prototypes

### Member 2: MPI & Hybrid Wrapper
- Implement: MPI wrapper (scatter/bcast/gather)
- Implement serial kernels: `matmul_serial()`, `strassen_serial()`, `proposed_serial()`
- File: `src/mpi_wrapper.c`, `src/kernels.c`
- Integrate Member 1's OpenMP kernels via function pointers

### Member 3: Testing & Utilities
- Implement: `src/utility.c` (matrix init, compare, print, timing)
- Write: `test/correctness_test.c`, `test/performance_test.c`
- Create: `scripts/run_tests.sh` automation
- Collect performance data and prepare report

## Key Interfaces

All kernel functions follow this signature:
```c
void function_name(double *A, double *B, double *C, int n);
```

MPI wrapper uses function pointers:
```c
void mpi_wrapper(double *A, double *B, double *C, int n,
                 void (*kernel)(double*, double*, double*, int));
```

## Workflow

1. **Day 1**: Implement serial kernels + MPI skeleton + OpenMP kernels separately
2. **Day 2**: Integrate MPI + OpenMP for hybrid, test correctness
3. **Day 3**: Performance testing on cluster, comparison with libs, finalize report

## Build & Run

```bash
# Serial/OpenMP
gcc -O2 -fopenmp main.c kernels.c utility.c -o matmul

# MPI
mpicc -O2 main.c mpi_wrapper.c kernels.c utility.c -o matmul_mpi

# Hybrid
mpicc -O2 -fopenmp main.c mpi_wrapper.c kernels.c omp_kernels.c utility.c -o matmul_hybrid

# Run
export OMP_NUM_THREADS=4
mpirun -np 4 ./matmul_hybrid
```

## Notes
- Matrix format: 1D row-major, double precision
- Strassen requires n = power of 2 (pad if needed)
- Use MPI_Wtime() for timing
- Test on multiple platforms as required
