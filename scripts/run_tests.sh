#!/bin/bash
# run_tests.sh
# Automated test runner for matrix multiplication implementations

set -e  # Exit on error

echo "=============================================="
echo "  Matrix Multiplication Test Suite"
echo "=============================================="
echo ""

# Colors for output (use real escape chars; disable if not a TTY)
if [ -t 1 ] && [ -z "${NO_COLOR+x}" ]; then
    RED=$'\033[0;31m'
    GREEN=$'\033[0;32m'
    YELLOW=$'\033[1;33m'
    NC=$'\033[0m' # No Color
else
    RED=""; GREEN=""; YELLOW=""; NC=""
fi

# Compiler and flags
CC=${CC:-gcc}
CFLAGS=${CFLAGS:-"-O2 -Wall"}
DEFAULT_OMP_FLAGS="-fopenmp"

# Detect OpenMP support (Apple Clang lacks -fopenmp unless libomp/GCC installed)
if [ -z "${OMP_FLAGS+x}" ]; then
    if echo "int main(){return 0;}" | $CC $CFLAGS $DEFAULT_OMP_FLAGS -x c - -o /tmp/omp_check 2>/dev/null; then
        OMP_FLAGS=$DEFAULT_OMP_FLAGS
    else
        echo "${YELLOW}OpenMP flags not supported by '$CC'; building without OpenMP (serial fallback).${NC}"
        echo "Set CC=gcc-13 (Homebrew) or install libomp to enable OpenMP."
        OMP_FLAGS=""
    fi
    rm -f /tmp/omp_check
fi

# Build directory
BUILD_DIR="build"

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

echo "${YELLOW}[1/3] Building tests...${NC}"
cd "$BUILD_DIR"

# Build correctness test
echo "  - Compiling correctness_test"
"$CC" $CFLAGS $OMP_FLAGS -o correctness_test ../test/correctness_test.c ../src/kernels.c ../src/omp_kernels.c ../src/utility.c -I../src -lm

# Build performance test
echo "  - Compiling performance_test"
"$CC" $CFLAGS $OMP_FLAGS -o performance_test ../test/performance_test.c ../src/kernels.c ../src/omp_kernels.c ../src/utility.c -I../src -lm

echo "${GREEN}Build successful!${NC}"
echo ""

echo "${YELLOW}[2/3] Running correctness tests...${NC}"
if ./correctness_test; then
    echo "${GREEN}Correctness tests passed!${NC}"
else
    echo "${RED}Correctness tests failed!${NC}"
    exit 1
fi
echo ""

echo "${YELLOW}[3/3] Running performance benchmarks...${NC}"
./performance_test
echo ""

echo "${GREEN}=============================================="
echo "  All tests completed successfully!"
echo "==============================================${NC}"
