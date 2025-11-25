# matrix-mult-parallel
Parallel matrix multiplication using OpenMP, MPI, and hybrid approaches (naive, Strassen, proposed algorithms)

## Current implementation status
- Algorithms: `naive` is implemented; `strassen` and `proposed` currently fall back to naive (placeholders).
- Parallel modes: Serial, MPI, and Hybrid (MPI + OpenMP). Hybrid gains extra speed only when compiled with OpenMP.
- MPI splitting: uses `MPI_Scatterv/MPI_Gatherv` and handles sizes not divisible by `np`.

## Build & run (minimal, no OpenMP required)
If OpenMP toolchain is not available, you can still build and run MPI/hybrid (hybrid will behave like MPI-only):
```bash
mpicc -O2 src/main.c src/mpi_wrapper.c src/kernels.c src/omp_kernels.c src/utility.c -o matmul

./matmul 64 serial naive
mpirun -np 2 ./matmul 64 mpi naive
mpirun -np 2 ./matmul 64 hybrid naive   # runs but without thread-level parallelism
```

## Build with OpenMP (for real hybrid/OpenMP speedup)
- macOS (Homebrew): install GCC/libomp (`brew install gcc libomp openmpi`), then:
```bash
mpicc -O2 -fopenmp \
  src/main.c src/mpi_wrapper.c src/kernels.c src/omp_kernels.c src/utility.c \
  -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp \
  -o matmul
```
- Linux (Ubuntu): `sudo apt install build-essential libomp-dev openmpi-bin libopenmpi-dev`, then:
```bash
mpicc -O2 -fopenmp src/main.c src/mpi_wrapper.c src/kernels.c src/omp_kernels.c src/utility.c -o matmul
```

## Run examples
```bash
# Serial baseline
./matmul 100 serial naive

# OpenMP on one rank (needs OpenMP build)
OMP_NUM_THREADS=4 ./matmul 500 openmp naive

# MPI
mpirun -np 4 ./matmul 1000 mpi naive

# Hybrid MPI + OpenMP (needs OpenMP build)
OMP_NUM_THREADS=4 mpirun -np 4 ./matmul 1000 hybrid naive
```

## Notes
- Matrices are initialized with fixed seeds for reproducibility.
- Verification runs a serial naive reference on rank 0 (for non-baseline modes).
- When `np=1`, the chosen kernel runs directly; when `np>1`, partial blocks use a simple triple loop (OMP-parallel if available).
