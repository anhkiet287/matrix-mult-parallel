# matrix-mult-parallel

Parallel matrix multiplication kernels (naive, Strassen, and a cache-blocked “proposed” variant) with four execution approaches: serial, OpenMP, MPI, and hybrid MPI+OpenMP. All experiments share deterministic inputs, unified configuration, and standardized logging so that performance comparisons remain fair across machines and algorithms.

## Overview

- **Kernels**
  - `naive`: classic triple-loop `O(n³)` GEMM.
  - `strassen`: recursive Strassen with automatic padding and OpenMP task parallelism above a 256 threshold.
  - `proposed`: cache-blocked multiply with a pre-transposed B tile to boost locality; OpenMP variant parallelizes across tiles.
- **Approaches**
  - `serial`: single-threaded kernels.
  - `openmp`: thread-level parallelism on one rank.
  - `mpi`: master/worker `MPI_Scatterv` + `MPI_Gatherv` with per-rank kernels.
  - `hybrid`: same MPI decomposition while each rank uses the OpenMP kernels (set `OMP_NUM_THREADS`).
- **Fair experiments**
  - Deterministic seeds (`srand(42)` for A, `srand(123)` for B) and a shared list of matrix sizes from `config/test_settings.sh`.
  - Each timed experiment runs the same number of repetitions, records the average time, recomputes GFLOPS, and compares against a serial naive reference to report `passed=true/false`.
  - Structured logs (terminal + CSV/JSON files) capture timestamps, machine IDs, algorithms, approaches, process/thread counts, GFLOPS, and optional notes.

## Repository layout

```
matrix-mult-parallel/
├── src/                # main CLI, kernels, MPI wrapper, utilities, logging helpers
├── test/               # correctness & performance suites (serial/OpenMP + MPI/hybrid)
├── scripts/            # regression runners + export_results_md.py helper
├── config/test_settings.sh  # single source of truth for matrix sizes, repetitions, tolerances
├── matmul              # built CLI (see “Building”)
└── README.md
```

## Building

Dependencies:
- C compiler with OpenMP support (GCC/Clang)
- MPI toolchain (`mpicc`, `mpirun`) for distributed and hybrid runs
- Python 3 (optional, for `scripts/export_results_md.py`)

Example builds:

```bash
# Serial + OpenMP only
gcc -O3 -fopenmp -o matmul \
  src/main.c src/kernels.c src/omp_kernels.c src/mpi_wrapper.c src/utility.c

# Full hybrid build with MPI (recommended)
mpicc -O3 -fopenmp -lm -o matmul \
  src/main.c src/kernels.c src/omp_kernels.c src/mpi_wrapper.c src/utility.c
```

If your compiler installs OpenMP headers/libraries elsewhere (e.g., Homebrew’s `libomp` on macOS), add the appropriate `-I`/`-L`/`-lomp` flags. Scripts default to `gcc`/`mpicc` but honor `CC`, `CFLAGS`, `MPICC`, `MPIRUN`, and `OMP_FLAGS` overrides.

## Running `matmul`

```
./matmul <n> <approach> <algorithm>

# Examples
./matmul 256 serial naive
OMP_NUM_THREADS=8 ./matmul 1024 openmp proposed
mpirun -np 4 ./matmul 1024 mpi strassen
OMP_NUM_THREADS=4 mpirun -np 4 ./matmul 2048 hybrid proposed
```

The program always boots MPI so the same binary can execute any approach. Rank 0 allocates matrices, seeds the random generator deterministically, and prints configuration details. After the run, rank 0 recomputes a serial naive reference (unless the run already used serial naive) and reports pass/fail with a tolerance of `1e-6`.

## Correctness checks

- All kernels (serial, OpenMP, MPI, hybrid) are exercised by dedicated test binaries in `test/`.
- Deterministic matrix initialization ensures identical inputs for every algorithm/approach pair.
- Serial/OpenMP and MPI/hybrid performance suites automatically compute a matmul_serial baseline per matrix size and flag mismatches via `matrix_compare`.

## Automated test suites

Scripts live in `scripts/` and all share `config/test_settings.sh`, so you can edit matrix sizes, repetitions, tolerances, and algorithm lists in one place. Key scripts:

| Script | What it does |
| --- | --- |
| `scripts/run_tests_openmp.sh` | Builds + runs `build/correctness_test` and `build/performance_test`. Covers serial and OpenMP kernels only. |
| `scripts/run_tests_mpi.sh` | Builds + runs `build/mpi_correctness_test` and `build/mpi_performance_test` using MPI kernels per rank. Skips automatically if `mpicc`/`mpirun` are missing. |
| `scripts/run_tests_hybrid.sh` | Same as MPI script but launches the hybrid (OpenMP-inside) variations. |
| `scripts/run_three_approach_size.sh` | Convenience wrapper that runs OpenMP, MPI, and hybrid benchmarks for one matrix size so you can compare the approaches directly. |
| `scripts/run_tests.sh` | Orchestrates openmp + mpi + hybrid suites sequentially; set `RUN_TESTS_LOG_DIR=/path` to tee combined output into a timestamped log. |

Common environment knobs (all loaded from `config/test_settings.sh` unless you override them):

- `TEST_CORRECTNESS_SIZE`, `MPI_TEST_SIZE` – matrices for correctness smoke tests.
- `TEST_PERFORMANCE_SIZES`, `MPI_PERF_SIZES` – comma/space separated sizes for shared-memory vs distributed benchmarks. MPI falls back to `TEST_PERFORMANCE_SIZES` when its list is unset to keep experiments aligned.
- `TEST_PERFORMANCE_RUNS` – number of timed repetitions per configuration (used everywhere; default 5).
- `MPI_PERF_RUNS` – legacy MPI-specific repetition count (falls back to `TEST_PERFORMANCE_RUNS` when unset).
- `WARMUP_RUNS` – warm-up iterations to discard before timing (default 1).
- `CORRECTNESS_KERNELS`, `PERFORMANCE_KERNELS`, `MPI_ALGORITHMS`, `HYBRID_ALGORITHMS` – restrict which kernels/algorithms are exercised.
- `OMP_THREAD_LIST` – thread counts to sweep for OpenMP tests (e.g., `1,2,4,8`).
- `MPI_PROC_LIST` – MPI ranks to sweep (e.g., `1,2,4,8`) when running `scripts/run_tests_mpi.sh`.
- `MPIRUN_FLAGS` – extra launcher flags appended before `-np` (defaults to `--bind-to core`; override with `--bind-to none --oversubscribe` on laptops or custom pinning rules).
- `HYBRID_GRID` – comma-separated list of `<procs>x<threads>` pairs (e.g., `2x8,4x4,4x6`) for hybrid sweeps.
- `MPI_PROCS` / `OMP_NUM_THREADS` – defaults when no sweep list is provided.
- `USE_OPENBLAS=1`, `OPENBLAS_DIR=/path` – opt-in BLAS baseline support (adds `-DUSE_CBLAS` and links OpenBLAS when building).
- `BLAS_ALLOW_THREADS=1` – let vendor BLAS manage its own threading (default forces BLAS baselines to one thread).
- `ENABLE_STRESS_10K=1` – append `n=10000` to both shared-memory and MPI performance sweeps (leave unset/0 for day-to-day runs).

Manual entry points if you want to run binaries directly after one build:

```bash
./build/correctness_test
./build/performance_test
mpirun -np 4 ./build/mpi_correctness_test proposed hybrid
MPI_PERF_SIZES=512,1024 mpirun -np 4 ./build/mpi_performance_test proposed mpi
```

## Experiment logging & metrics

Set `RESULTS_DIR=<path>` to enable file logging. Each test binary writes to a deterministic file (or to `RESULTS_DIR/<RESULTS_FILE_BASENAME>.{csv,json}` when `RESULTS_FILE_BASENAME` is set):

- `openmp_results.*` for `performance_test`
- `mpi_results.*` for `mpi_performance_test` (MPI mode)
- `hybrid_results.*` for hybrid mode

Format defaults to CSV; override with `RESULTS_FORMAT=json`. Each line/record follows the unified schema:

```
timestamp,machine_id,algo,approach,n,nprocs,nthreads,repetitions,
time_sec,time_min,time_max,time_mean,gflops_gemm_eq,passed,speedup_vs_naive,note
```

Key metrics:
- `time_sec` is the median of `TEST_PERFORMANCE_RUNS` iterations; `time_min/time_max/time_mean` capture variability.
- `gflops_gemm_eq` always uses the GEMM-equivalent `2n^3 / time` formula, even for Strassen (treat it as a relative throughput metric).
- `speedup_vs_naive` compares each configuration against the serial naive baseline for the same `n` (when available).

Environment helpers:
- `MACHINE_ID` – free-form string describing the host (default `unknown`).
- `RESULTS_NOTE` – optional note appended to each record (e.g., `hpcc node01` or `warmup excluded`).
- `RESULTS_FILE_BASENAME` – override the default filename when aggregating multiple suites into one log.

Use `python3 scripts/export_results_md.py -i results/openmp_results.csv` to turn CSV output into Markdown tables for reports.

### Scalability sweeps

The performance scripts understand several sweep lists:

- **OpenMP threads:** `OMP_THREAD_LIST="1,2,4,8" bash scripts/run_tests_openmp.sh`
- **MPI ranks:** `MPI_PROC_LIST="1,2,4,8" bash scripts/run_tests_mpi.sh`
- **Hybrid grids:** `HYBRID_GRID="2x4,4x4,4x6" bash scripts/run_tests_hybrid.sh`

Each configuration logs a single row with median/min/max/mean time and GEMM-equivalent GFLOPS, making it trivial to plot scaling curves.
MPI sweeps on laptops/desktops can be noisy; for graded scalability studies we recommend running the same commands on the provided cluster or via `scripts/hpcc_job_slurm.sh`.

### Single-size comparison across approaches

To exercise all three approaches (OpenMP, MPI, and hybrid) on the same matrix size, use:

```bash
cd ~/matrix-mult-parallel   # or cd /path/to/your/clone
mkdir -p results logs
export MACHINE_ID=$(hostname)
export RESULTS_DIR=$PWD/results
export MPIRUN_FLAGS="--bind-to core"
bash scripts/run_three_approach_size.sh 2048 logs/size2048
```

The script pins `TEST_PERFORMANCE_SIZES`/`MPI_PERF_SIZES` to the value you pass (2048 in the example) and then calls the standard OpenMP, MPI, and hybrid test suites. Customize `OMP_THREAD_LIST`, `MPI_PROC_LIST`, and `HYBRID_GRID` beforehand to match your hardware. Results land in `results/openmp_results.csv`, `results/mpi_results.csv`, and `results/hybrid_results.csv`, all filtered to the requested matrix size for easy comparison.

### Best-config template (side-by-side comparison)

When you want one “best” configuration per approach—for example, on an 8-thread workstation—run:

```bash
cd /path/to/your/matrix-mult-parallel
mkdir -p results logs

export MACHINE_ID=$(hostname)         # customize per platform
export RESULTS_DIR=$PWD/results
export RESULTS_NOTE="best-config sweep"
export MPIRUN_FLAGS="--bind-to core"  # override if you need different pinning

# OpenMP: pick a single thread count
TEST_PERFORMANCE_SIZES=2048 OMP_THREAD_LIST="8" \
RUN_TESTS_LOG_DIR=logs bash scripts/run_tests_openmp.sh

# MPI: pick the rank count that fits your cores
MPI_PERF_SIZES=2048 MPI_PROC_LIST="8" \
RUN_TESTS_LOG_DIR=logs bash scripts/run_tests_mpi.sh

# Hybrid: choose a grid (procs x threads) that multiplies to your cores
MPI_PERF_SIZES=2048 HYBRID_GRID="4x2" \
RUN_TESTS_LOG_DIR=logs bash scripts/run_tests_hybrid.sh
```

Tune these environment variables for other hardware:
- `TEST_PERFORMANCE_SIZES` / `MPI_PERF_SIZES`: matrix sizes (comma-separated) to benchmark.
- `OMP_THREAD_LIST`: OpenMP thread counts (set to one value to test only your “best” option).
- `MPI_PROC_LIST`: MPI ranks per run; avoid exceeding the physical core count unless you add `--oversubscribe`.
- `HYBRID_GRID`: comma-separated list of `procsxthreads` pairs (e.g., `2x4,4x2`).
- `MPIRUN_FLAGS`: launcher flags for pinning or oversubscription.

This template yields one row per approach in the CSV logs so you can compare their speedups side by side without re-running full sweeps.

### Cluster automation

On Slurm-based clusters run:

```bash
bash scripts/hpcc_job_slurm.sh
```

The helper script sets `RESULTS_DIR`, `MACHINE_ID`, `RESULTS_NOTE`, submits an `sbatch` job, and runs both MPI (`MPI_PROC_LIST`) and hybrid (`HYBRID_GRID`) sweeps via `srun`. When `sbatch` is unavailable the script prints manual instructions instead.

### Optional BLAS baseline

To include a single-node BLAS DGEMM reference (algo=`blas`, approach=`serial`):

1. Install OpenBLAS (or another CBLAS-compatible library).
2. Build/run with BLAS enabled, for example:
   ```bash
   USE_OPENBLAS=1 OPENBLAS_DIR=/opt/openblas \
   RESULTS_DIR=results \
   PERFORMANCE_KERNELS="matmul_serial matmul_blas" \
   OMP_THREAD_LIST="1,2,4" \
   bash scripts/run_tests_openmp.sh
   ```
3. By default the benchmark forces `OPENBLAS_NUM_THREADS=1` (and likewise for MKL/BLIS) so the baseline matches the single-threaded naive reference. Set `BLAS_ALLOW_THREADS=1` to let BLAS manage its own threading.
4. Logs will contain rows such as `algo=blas`, `approach=serial`, `nthreads=1`, and the `note` field automatically appends `blas=<backend>` (e.g., `blas=openblas`). Convert to Markdown for the report via:
   ```bash
   python3 scripts/export_results_md.py -i results/openmp_results.csv -o results/openmp_results.md
   ```

When BLAS support is disabled (default build), the benchmark prints a single warning and skips the `blas` configuration gracefully.

## Reproducing experiments

1. **Configure the machine**  
   Install a compiler with OpenMP, MPI runtime, and Python 3. Set `MACHINE_ID`, `OMP_NUM_THREADS`, and `MPI_PROCS` as needed.
2. **Clone & build**  
   `git clone ... && cd matrix-mult-parallel && bash scripts/run_tests_openmp.sh` (or the full `run_tests.sh` if MPI networking is available).
3. **Set experiment knobs**  
   Edit `config/test_settings.sh` or export env vars (`TEST_PERFORMANCE_SIZES`, `MPI_PERF_SIZES`, `TEST_PERFORMANCE_RUNS`, `WARMUP_RUNS`, `OMP_THREAD_LIST`, `MPI_PROC_LIST`, `HYBRID_GRID`, `RESULTS_DIR`, `RESULTS_FILE_BASENAME`, etc.).
4. **Run suites**  
   `RESULTS_DIR=results RUN_TESTS_LOG_DIR=logs bash scripts/run_tests.sh`
5. **Inspect results**  
   - Terminal summaries show algo/approach dimensions, med/min/max/mean runtimes, GEMM-equivalent GFLOPS, and pass/fail.
   - CSV/JSON logs live under `results/`.
   - Optional Markdown export via `scripts/export_results_md.py`.
   - Attach logs (and scripts/test_settings snapshot) to your report for reproducibility.

### Full sweep recipe (copy & paste)

```bash
cd ~/matrix-mult-parallel   # or cd /path/to/your/clone
mkdir -p results logs

export MACHINE_ID=$(hostname)
export RESULTS_DIR=$PWD/results
export RESULTS_NOTE="report sweep"
export TEST_PERFORMANCE_RUNS=7
export WARMUP_RUNS=2

# 1) Shared-memory sweep + BLAS baseline
USE_OPENBLAS=1 OPENBLAS_DIR=/opt/openblas \
PERFORMANCE_KERNELS="matmul_serial matmul_blas matmul_omp strassen_omp proposed_omp" \
OMP_THREAD_LIST="1,2,4,8" \
RUN_TESTS_LOG_DIR=logs \
bash scripts/run_tests_openmp.sh

# 2) MPI ranks (override MPIRUN_FLAGS if you need different pinning)
MPI_PROC_LIST="1,2,4,8" \
RUN_TESTS_LOG_DIR=logs \
bash scripts/run_tests_mpi.sh

# 3) Hybrid grids (processes x threads, pick combos that fit your machine)
HYBRID_GRID="2x4,4x2,4x1" \
RUN_TESTS_LOG_DIR=logs \
bash scripts/run_tests_hybrid.sh

# Export CSV -> Markdown for reporting
python3 scripts/export_results_md.py -i results/openmp_results.csv -o results/openmp_results.md
python3 scripts/export_results_md.py -i results/mpi_results.csv -o results/mpi_results.md
python3 scripts/export_results_md.py -i results/hybrid_results.csv -o results/hybrid_results.md
```

Tùy chỉnh `OPENBLAS_DIR`, `MPI_PROC_LIST`, `HYBRID_GRID` theo phần cứng từng người. Sau khi chạy xong sẽ có dữ liệu cho cả shared-memory, MPI và hybrid cùng với bảng Markdown tiện đưa vào báo cáo.

## Status & next steps

- ✅ All required kernels (naive, Strassen, proposed) implemented and available in serial, OpenMP, MPI, and hybrid forms.
- ✅ Deterministic seeding, shared configuration, and unified logging across serial/OpenMP/MPI experiments.
- ✅ Correctness harnesses compare every run against the serial naive reference with tolerance `1e-6`.
- ✅ Optional BLAS baseline (OpenBLAS/CBLAS) integrated into the shared-memory suite (`USE_OPENBLAS=1` + `algo=blas`).
- 🔄 Pending work: large-scale (100→10 000) sweeps on every required platform and more advanced MPI tilings to reduce root bottlenecks.

Use the TODOs in `docs/design_notes.md` and the “Optimization opportunities” section of the prior README if you plan further improvements (2‑D decompositions, auto-tuning block sizes, BLAS comparisons, etc.).
