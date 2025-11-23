#!/bin/bash
# run_tests.sh
# Automated test runner for matrix multiplication implementations

set -e  # Exit on error

echo "=============================================="
echo "  Matrix Multiplication Test Suite"
echo "=============================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

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
gcc -o correctness_test ../test/correctness_test.c ../src/kernels.c ../src/omp_kernels.c ../src/utility.c -I../src -lm -fopenmp

# Build performance test
echo "  - Compiling performance_test"
gcc -o performance_test ../test/performance_test.c ../src/kernels.c ../src/omp_kernels.c ../src/utility.c -I../src -lm -fopenmp

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
