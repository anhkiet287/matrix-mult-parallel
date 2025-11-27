# Default test configuration shared by scripts and C test binaries.
# Override by exporting the variables before running scripts, or edit this file.

: "${TEST_CORRECTNESS_SIZE:=256}"
: "${TEST_CORRECTNESS_TOLERANCE:=1e-6}"
: "${CORRECTNESS_KERNELS:=matmul_serial matmul_omp strassen_serial strassen_omp proposed_serial proposed_omp}"

: "${TEST_PERFORMANCE_SIZES:=128,256,512,1024,2048}"
: "${TEST_PERFORMANCE_RUNS:=5}"
: "${PERFORMANCE_KERNELS:=matmul_serial matmul_omp strassen_serial strassen_omp proposed_serial proposed_omp}"

: "${MPI_TEST_SIZE:=256}"
: "${MPI_PERF_SIZES:=128,256,512,1024,2048}"
: "${MPI_PERF_RUNS:=1}"
: "${MPI_PROCS:=4}"

# MPI + Hybrid algorithm sweeps (space or comma separated)
: "${MPI_ALGORITHMS:=naive strassen proposed}"
: "${HYBRID_ALGORITHMS:=naive strassen proposed}"
