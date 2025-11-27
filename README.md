# matrix-mult-parallel
Parallel matrix multiplication using OpenMP, MPI, and hybrid approaches (naive, Strassen, proposed algorithms)

## Assignment fit (what’s covered vs. required)
- Platforms
  - ✅ Serial / shared-memory: implemented; runs on laptops.
  - ✅ MPI / distributed-memory: implemented master–worker with `MPI_Scatterv/MPI_Gatherv`; handles n not divisible by np; verified on cluster.
  - ✅ Hybrid MPI+OpenMP: wiring is present; gains speed when compiled with `-fopenmp`. Fallback is MPI-only if OpenMP is unavailable.
- Algorithms
  - ✅ Naive: implemented (serial + OpenMP entrypoints).
  - ⏳ Strassen: stub, currently calls naive (needs real implementation + padding strategy).
  - ⏳ Proposed: stub, currently calls naive (needs your chosen optimized/blocking method, serial + OpenMP).
- Correctness & testing
  - ✅ Deterministic init (fixed seeds), rank 0 verifies non-baseline runs against serial naive with tolerance.
  - ✅ Small-matrix printing for n ≤ 10.
  - ⏳ Extend tests when Strassen/proposed are added; run correctness/perf suites on all platforms.
- Performance & scalability study
  - ✅ Timing + GFLOPS printed per run.
  - ⏳ Collect data across sizes (100–10,000) and multiple machines; compare against a library baseline (e.g., OpenBLAS) for report.

Use this summary in the report to show which mandatory items are already met and which are pending.

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
mpicc -O3 -fopenmp -o matmul src/main.c src/kernels.c src/omp_kernels.c src/mpi_wrapper.c src/utility.c -lm
```
- Cluster (gateway.hpcc.vn): modules may already provide OpenMPI/GCC. Typical: `module load gcc openmpi` then the Linux command above.

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

## Remaining work to reach final deliverable
- Implement real `strassen_serial/strassen_omp` (and padding to power-of-two if needed).
- Implement the `proposed` algorithm (serial + OpenMP), e.g., tiled/blocking or hybrid naive/Strassen.
- Optimize hybrid path to use block-aware kernels per partition (optional) instead of fallback triple-loop.
- Add/extend tests to cover new algorithms and hybrid correctness; run and record performance benchmarks across sizes (100–10,000) and platforms.
- Prepare scripts for automated runs on the cluster (SLURM batch if available) and collect/report GFLOPS comparisons vs library baselines.
