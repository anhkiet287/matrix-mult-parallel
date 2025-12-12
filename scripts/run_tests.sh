#!/bin/bash
# Aggregate test runner. Executes OpenMP-only, MPI-only, and Hybrid suites.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUN_TESTS_LOG_DIR="${RUN_TESTS_LOG_DIR:-}"

if [ -n "$RUN_TESTS_LOG_DIR" ]; then
    mkdir -p "$RUN_TESTS_LOG_DIR"
    LOG_FILE="$RUN_TESTS_LOG_DIR/run_tests_$(date +%Y%m%d_%H%M%S).log"
    echo "Logging output to $LOG_FILE"
    exec > >(tee -a "$LOG_FILE") 2>&1
fi

echo "=============================================="
echo "  Combined Matrix Multiplication Test Suites"
echo "=============================================="
echo ""

"$SCRIPT_DIR/run_tests_openmp.sh"
echo ""

"$SCRIPT_DIR/run_tests_mpi.sh"
echo ""

"$SCRIPT_DIR/run_tests_hybrid.sh"

echo ""
echo "=============================================="
echo "  Full test matrix complete"
echo "=============================================="
