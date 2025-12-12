#!/bin/bash
# Run MPI-only correctness and performance tests (serial kernels per rank).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# shellcheck source=/dev/null
source "$SCRIPT_DIR/run_tests_common.sh"

setup_colors
echo "=============================================="
echo "  ${YELLOW}MPI Test Suite${NC}"
echo "=============================================="

if ! command -v "$MPICC" >/dev/null 2>&1; then
    echo "${YELLOW}MPI compiler '$MPICC' not found. Skipping MPI tests.${NC}"
    exit 0
fi

if ! command -v "$MPIRUN" >/dev/null 2>&1; then
    echo "${YELLOW}MPI runner '$MPIRUN' not found. Skipping MPI tests.${NC}"
    exit 0
fi

build_mpi_binaries

mpi_algorithms=$(get_algorithm_list "${MPI_ALGORITHMS}" "${MPI_ALGORITHM}" "naive strassen proposed")
mpi_proc_list=$(normalize_algorithm_list "${MPI_PROC_LIST:-}")
if [ -z "$mpi_proc_list" ]; then
    mpi_proc_list="$MPI_PROCS"
fi

echo ""
echo "${YELLOW}[1/2] Running MPI correctness sweep...${NC}"
for procs in $mpi_proc_list; do
    echo "  + MPI ranks = ${procs}"
    for algorithm in $mpi_algorithms; do
        echo "    - ${algorithm}"
        run_mpi_correctness "$algorithm" mpi "$procs"
    done
done
echo "${GREEN}✓ MPI correctness passed (${mpi_algorithms}) [ranks: ${mpi_proc_list}]${NC}"

echo ""
echo "${YELLOW}[2/2] Running MPI performance sweep...${NC}"
for procs in $mpi_proc_list; do
    echo "  + MPI ranks = ${procs}"
    for algorithm in $mpi_algorithms; do
        echo "    - ${algorithm}"
        run_mpi_performance "$algorithm" mpi "$procs"
    done
done
echo "${GREEN}✓ MPI benchmarks complete (${mpi_algorithms}) [ranks: ${mpi_proc_list}]${NC}"

echo ""
echo "=============================================="
echo "  ${GREEN}MPI suite finished successfully${NC}"
echo "=============================================="
