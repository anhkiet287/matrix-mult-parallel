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

echo ""
echo "${YELLOW}[1/2] Running hybrid correctness sweep...${NC}"
for algorithm in $hybrid_algorithms; do
    echo "  - ${algorithm}"
    run_mpi_correctness "$algorithm" hybrid
done
echo "${GREEN}✓ Hybrid correctness passed (${hybrid_algorithms})${NC}"

echo ""
echo "${YELLOW}[2/2] Running hybrid performance sweep...${NC}"
for algorithm in $hybrid_algorithms; do
    echo "  - ${algorithm}"
    run_mpi_performance "$algorithm" hybrid
done
echo "${GREEN}✓ Hybrid benchmarks complete (${hybrid_algorithms})${NC}"

echo ""
echo "=============================================="
echo "  ${GREEN}Hybrid suite finished successfully${NC}"
echo "=============================================="
