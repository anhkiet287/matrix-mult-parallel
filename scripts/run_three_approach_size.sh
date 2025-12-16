#!/bin/bash
# Convenience wrapper: run OpenMP, MPI, and Hybrid performance tests
# for a single matrix size so you can compare the three approaches head-to-head.

set -euo pipefail

if [ $# -lt 1 ]; then
    echo "Usage: $0 <matrix_size> [log_dir]"
    echo "Example: $0 2048 logs/size2048"
    exit 1
fi

MATRIX_SIZE="$1"
LOG_DIR="${2:-logs}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

export TEST_PERFORMANCE_SIZES="$MATRIX_SIZE"
export MPI_PERF_SIZES="$MATRIX_SIZE"

mkdir -p "$PROJECT_ROOT/$LOG_DIR"
export RUN_TESTS_LOG_DIR="$PROJECT_ROOT/$LOG_DIR"

echo "================================================="
echo " Matrix size sweep: n = ${MATRIX_SIZE}"
echo " OpenMP threads : ${OMP_THREAD_LIST:-default from config}"
echo " MPI ranks      : ${MPI_PROC_LIST:-${MPI_PROCS:-4}}"
echo " Hybrid grids   : ${HYBRID_GRID:-${MPI_PROCS:-4}x${OMP_NUM_THREADS:-1}}"
echo " Log directory  : ${RUN_TESTS_LOG_DIR}"
echo "================================================="

pushd "$PROJECT_ROOT" >/dev/null

echo ""
echo ">>> Running OpenMP benchmarks (n=${MATRIX_SIZE})"
bash "$SCRIPT_DIR/run_tests_openmp.sh"

echo ""
echo ">>> Running MPI benchmarks (n=${MATRIX_SIZE})"
bash "$SCRIPT_DIR/run_tests_mpi.sh"

echo ""
echo ">>> Running Hybrid benchmarks (n=${MATRIX_SIZE})"
bash "$SCRIPT_DIR/run_tests_hybrid.sh"

popd >/dev/null

echo ""
echo "All three approaches completed for n=${MATRIX_SIZE}."
echo "Check ${RESULTS_DIR:-results}/*_results.csv for the metrics."
