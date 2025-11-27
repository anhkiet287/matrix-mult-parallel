# matrix-mult-parallel
Parallel matrix multiplication using OpenMP, MPI, and hybrid approaches (naive, Strassen, proposed algorithms)

## Assignment fit (what’s covered vs. required)
- Platforms
  - ✅ Serial / shared-memory: implemented; runs on laptops.
  - ✅ MPI / distributed-memory: master–worker decomposition with `MPI_Scatterv/MPI_Gatherv`, irregular row counts handled, supports all kernels (naive, Strassen via padding, proposed via cache-blocked tiles).
  - ✅ Hybrid MPI+OpenMP: same MPI decomposition while using OpenMP kernels inside each rank (set `OMP_NUM_THREADS` to control threads). Falls back to MPI-only if OpenMP is unavailable.
- Algorithms
  - ✅ Naive: implemented (serial + OpenMP entrypoints).
  - ✅ Strassen: recursive implementation (serial + OpenMP) with automatic padding to next power-of-two; threshold currently fixed at 64.
  - ✅ Proposed: cache-blocked + transposed-B optimization available in serial + OpenMP versions.
- Correctness & testing
  - ✅ Deterministic init (fixed seeds), rank 0 verifies non-baseline runs against serial naive with tolerance.
  - ✅ Small-matrix printing for n ≤ 10.
  - ✅ Coverage for all kernels (serial + OpenMP) plus MPI/hybrid correctness/performance harnesses (see test section).
- Performance & scalability study
  - ✅ Timing + GFLOPS printed per run.
  - ⏳ Run the full 100→10 000 sweep across each group member’s shared-memory machine plus the provided distributed cluster (gateway.hpcc.vn), and add comparisons to an external BLAS (OpenBLAS/MKL/CUBLAS) in the final report.

Use this summary in the report to show which mandatory items are already met and which are pending.

## Automated testing & benchmarking

### Script overview
- `scripts/run_tests_openmp.sh` – builds + runs only the serial/OpenMP correctness and performance binaries.
- `scripts/run_tests_mpi.sh` – builds + runs the MPI-only correctness/performance suites (serial kernels per rank).
- `scripts/run_tests_hybrid.sh` – builds + runs the hybrid MPI+OpenMP correctness/performance suites.
- `scripts/run_tests.sh` – orchestration script that runs the three suites sequentially.
- `config/test_settings.sh` – shared defaults for matrix sizes, tolerances, algorithm lists, and test kernel selections. All scripts source it automatically, and the C test binaries read the same environment variables (editable in one place or via `export` overrides).

### Full regression (serial + OpenMP + MPI/hybrid)
```
cd matrix-mult-parallel
bash scripts/run_tests.sh
```
This script:
1. Runs `run_tests_openmp.sh` (builds and executes the serial/OpenMP correctness + performance binaries).
2. Runs `run_tests_mpi.sh` (MPI-only) if `mpicc`/`mpirun` are detected, otherwise prints a skip notice.
3. Runs `run_tests_hybrid.sh` (MPI+OpenMP) under the same availability checks.
Control processes via `MPI_PROCS=<p>`. When working on machines that block sockets, run just the OpenMP script to avoid MPI launcher failures, or ensure MPI jobs target a cluster login/head node.

Important environment knobs:
- `CC`, `CFLAGS`, `OMP_FLAGS` – override compiler + OpenMP flags; script auto-detects `-fopenmp` support.
- `MPICC`, `MPIRUN`, `MPI_PROCS` – choose MPI toolchain and process count.
- `NO_COLOR=1` – disable ANSI colors in logs.
- `OMP_NUM_THREADS` – used by OpenMP kernels/tests you launch afterward.
- `MPI_ALGORITHMS`, `HYBRID_ALGORITHMS` – space/comma-separated lists of algorithms (naive|strassen|proposed) each suite will run; default is all three. `MPI_ALGORITHM` / `HYBRID_ALGORITHM` remain available for selecting a single algorithm.
- `CORRECTNESS_KERNELS`, `PERFORMANCE_KERNELS` – limit which kernels the serial/OpenMP binaries exercise (e.g., `CORRECTNESS_KERNELS="matmul_serial strassen_serial"` to test serial only).
- `TEST_CORRECTNESS_SIZE`, `TEST_PERFORMANCE_SIZES`, `MPI_TEST_SIZE`, `MPI_PERF_SIZES`, `TEST_PERFORMANCE_RUNS`, `MPI_PERF_RUNS`, `TEST_CORRECTNESS_TOLERANCE` – matrix sizes/tolerances used by the C test binaries. Edit once in `config/test_settings.sh` or export variables in your shell; all scripts/tests consume the same values.
- `RUN_TESTS_LOG_DIR=<path>` – when set, `scripts/run_tests.sh` tees all output (across sub-suites) into `path/run_tests_<timestamp>.log` while still printing to the console, making it easy to archive experiment logs.

### Targeted/manual runs
- Serial/OpenMP regression only (after one `run_tests` build): `./build/correctness_test`
- Serial/OpenMP performance sweep (sizes 128→2048, 5 runs each): `./build/performance_test`
- MPI correctness for a specific algorithm/mode: `mpirun -np 4 ./build/mpi_correctness_test naive mpi` or `mpirun -np 4 ./build/mpi_correctness_test proposed hybrid`
- MPI vs hybrid benchmark: `MPI_PERF_SIZES=256,512,1024 MPI_PERF_RUNS=3 mpirun -np 4 ./build/mpi_performance_test proposed hybrid`
- Serial-only smoke (skip OpenMP kernels): `CORRECTNESS_KERNELS="matmul_serial strassen_serial proposed_serial" PERFORMANCE_KERNELS="matmul_serial strassen_serial proposed_serial" bash scripts/run_tests_openmp.sh`

These binaries honor:
- `MPI_PERF_SIZES`, `MPI_PERF_RUNS` for MPI benchmarking grids.
- `OMP_NUM_THREADS` for OpenMP kernels (set before running hybrid tests).

### Application entry point
```
# Serial / OpenMP
./matmul 512 openmp proposed

# MPI / hybrid
OMP_NUM_THREADS=4 mpirun -np 4 ./matmul 1024 hybrid naive
```

## Recent benchmark snapshot (Nov 2023 configuration)
`config/test_settings.sh` currently specifies:
- `TEST_CORRECTNESS_SIZE=256`
- `TEST_PERFORMANCE_SIZES=128,256,512,1024,2048` with `TEST_PERFORMANCE_RUNS=5`
- `MPI_PERF_SIZES=256,512,1024,2048,4096` with `MPI_PERF_RUNS=1`

Selected highlights from the latest `bash scripts/run_tests.sh` run (4 MPI ranks, default `OMP_NUM_THREADS`):

| Mode | Algorithm | Size | Time (s) | GFLOPS | Notes |
|------|-----------|------|----------|--------|-------|
| OpenMP | `strassen_omp` | 2048 | 0.5622 | 30.56 | Fastest shared-memory kernel today. |
| OpenMP | `proposed_omp` | 2048 | 0.6720 | 25.56 | Cache-blocked kernel, close to Strassen for large n. |
| MPI | `proposed_serial` (per-rank) | 2048 | 0.9721 | 17.67 | Benefit of using the cache-blocked kernel inside each partition; still root-bound. |
| MPI | `naive` | 2048 | 9.9808 | 1.72 | Highlights the cost of scatter/gather when using the plain triple loop. |
| Hybrid | `proposed_omp` | 2048 | 0.7082 | 24.26 | Threads + MPI deliver the highest multi-process throughput today. |
| Hybrid | `naive` | 2048 | 6.6690 | 2.58 | Naive kernel still sees thread-level gains but is limited by MPI comms. |

Update these numbers (or link to CSV logs) when you reconfigure `test_settings.sh` or run on different machines.

## Status checklist
- [x] Serial naive, Strassen (auto padding), and cache-blocked “proposed” kernels implemented.
- [x] OpenMP counterparts for all kernels (task-based Strassen, tiled proposed, naive baseline).
- [x] MPI master–worker wrapper with load-balanced `MPI_Scatterv/MPI_Gatherv` and per-partition kernel dispatch (proposed uses cache-blocked multiply; Strassen pads to square subproblems for correctness).
- [x] Automated correctness + performance harnesses, including MPI and hybrid variants.
- [ ] Large-scale performance/scalability study (100→10 000) across all required platforms (each group member’s shared-memory machine + the provided cluster) with logged results.
- [ ] External baseline comparison (e.g., OpenBLAS/MKL/CUBLAS) documented in the report.

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

## Optimization opportunities / known gaps
- `src/mpi_wrapper.c`: Strassen partitions are padded to square matrices locally so correctness holds, but this introduces extra memory copies and diminishes the algorithm’s advantage. A 2D block distribution that keeps subtiles square (or recursively applies Strassen across ranks) would reduce padding overhead and let Strassen scale better.
- `src/omp_kernels.c` & `src/kernels.c`: Strassen implementation copies and allocates fresh submatrices at every recursion level, driving memory usage to O(n² log n) and adding a lot of malloc/free pressure. Reusing buffers, iterating in-place, or switching to an iterative schedule at larger thresholds (≥256) would improve performance.
- `test/correctness_test.c` / MPI tests: currently run a single matrix size per invocation (configured via `config/test_settings.sh`). Consider scripting sweeps across multiple sizes (including non-powers-of-two) and logging pass/fail plus checksum deltas to CSV for the report.
- `src/proposed_*`: `BLOCK_SIZE` is fixed at 64; expose it as a parameter or auto-tune per architecture (e.g., environment variable or config entry) to meet the “proposed algorithm” performance expectations on different caches.
- MPI scaling: the current master–worker design performs all scatter/gather on rank 0, so communication becomes the bottleneck and MPI throughput never surpasses OpenMP on a single machine. Implement a 2‑D block decomposition (SUMMA/Cannon) with non-blocking collectives to overlap communication/computation, and run on more ranks/nodes to showcase distributed-memory gains.

## Notes
- Matrices are initialized with fixed seeds for reproducibility.
- Verification runs a serial naive reference on rank 0 (for non-baseline modes).
- When `np=1`, the chosen kernel runs directly; when `np>1`, partial blocks use a simple triple loop (OMP-parallel if available).

## Remaining work to reach final deliverable
- Redesign distributed tiling (or add a 2D block-cyclic layout) so Strassen can operate on square subproblems without local padding, reducing memory copies and improving scaling.
- Add tunable knobs (or auto-tuning scripts) for Strassen thresholds and the proposed-algorithm block sizes, and document the tuning procedure in the report.
- Extend correctness/performance sweeps to span the 100→10 000 size range across all available machines, exporting CSV/plots for the final report and comparing the fastest configurations against a vendor BLAS (OpenBLAS/MKL/CUBLAS).
- Provide automation for cluster jobs (e.g., SLURM scripts) that iterate over matrix sizes, MPI ranks, and `OMP_NUM_THREADS`, storing results per run for reproducibility.
- Run the testing matrix on each required platform: (1) every group member’s laptop/desktop (OpenMP-only runs via `scripts/run_tests_openmp.sh`), (2) the provided distributed cluster via `gateway.hpcc.vn` (MPI + hybrid runs via `RUN_TESTS_LOG_DIR=... bash scripts/run_tests.sh` after `module load gcc openmpi`), and (3) a combined MPI+OpenMP configuration (hybrid mode) to satisfy the “marriage” requirement.
