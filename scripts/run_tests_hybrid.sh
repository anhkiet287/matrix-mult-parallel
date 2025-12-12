#!/bin/bash
# Run Hybrid (MPI + OpenMP) correctness and performance tests.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# shellcheck source=/dev/null
source "$SCRIPT_DIR/run_tests_common.sh"

setup_colors
echo "=============================================="
echo "  ${YELLOW}Hybrid (MPI + OpenMP) Test Suite${NC}"
echo "=============================================="

if ! command -v "$MPICC" >/dev/null 2>&1; then
    echo "${YELLOW}MPI compiler '$MPICC' not found. Skipping hybrid tests.${NC}"
    exit 0
fi

if ! command -v "$MPIRUN" >/dev/null 2>&1; then
    echo "${YELLOW}MPI runner '$MPIRUN' not found. Skipping hybrid tests.${NC}"
    exit 0
fi

build_mpi_binaries

hybrid_algorithms=$(get_algorithm_list "${HYBRID_ALGORITHMS}" "${HYBRID_ALGORITHM}" "naive strassen proposed")
hybrid_grid_raw="${HYBRID_GRID:-}"
if [ -z "$hybrid_grid_raw" ]; then
    default_threads="${OMP_NUM_THREADS:-1}"
    hybrid_grid_raw="${MPI_PROCS}x${default_threads}"
fi
hybrid_grid="${hybrid_grid_raw//,/ }"

echo ""
echo "${YELLOW}[1/2] Running hybrid correctness sweep...${NC}"
for entry in $hybrid_grid; do
    procs="${entry%x*}"
    threads="${entry#*x}"
    if [ -z "$procs" ] || [ -z "$threads" ] || [ "$entry" = "$threads" ]; then
        echo "${YELLOW}Skipping malformed HYBRID_GRID entry '${entry}' (expected PxT).${NC}"
        continue
    fi
    echo "  + Grid ${procs} ranks × ${threads} threads"
    export OMP_NUM_THREADS="$threads"
    for algorithm in $hybrid_algorithms; do
        echo "    - ${algorithm}"
        run_mpi_correctness "$algorithm" hybrid "$procs"
    done
done
echo "${GREEN}✓ Hybrid correctness passed (${hybrid_algorithms}) [grid: ${hybrid_grid_raw}]${NC}"

echo ""
echo "${YELLOW}[2/2] Running hybrid performance sweep...${NC}"
for entry in $hybrid_grid; do
    procs="${entry%x*}"
    threads="${entry#*x}"
    if [ -z "$procs" ] || [ -z "$threads" ] || [ "$entry" = "$threads" ]; then
        continue
    fi
    echo "  + Grid ${procs} ranks × ${threads} threads"
    export OMP_NUM_THREADS="$threads"
    for algorithm in $hybrid_algorithms; do
        echo "    - ${algorithm}"
        run_mpi_performance "$algorithm" hybrid "$procs"
    done
done
echo "${GREEN}✓ Hybrid benchmarks complete (${hybrid_algorithms}) [grid: ${hybrid_grid_raw}]${NC}"

echo ""
echo "=============================================="
echo "  ${GREEN}Hybrid suite finished successfully${NC}"
echo "=============================================="
