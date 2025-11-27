#!/bin/bash
# Run serial + OpenMP correctness and performance tests only.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# shellcheck source=/dev/null
source "$SCRIPT_DIR/run_tests_common.sh"

setup_colors
echo "=============================================="
echo "  ${YELLOW}Serial + OpenMP Test Suite${NC}"
echo "=============================================="

build_serial_omp_binaries

echo ""
echo "${YELLOW}[1/2] Running correctness tests...${NC}"
run_serial_omp_correctness
echo "${GREEN}✓ Serial/OpenMP correctness passed${NC}"

echo ""
echo "${YELLOW}[2/2] Running performance benchmarks...${NC}"
run_serial_omp_performance
echo "${GREEN}✓ Serial/OpenMP benchmarks complete${NC}"

echo ""
echo "=============================================="
echo "  ${GREEN}OpenMP suite finished successfully${NC}"
echo "=============================================="
