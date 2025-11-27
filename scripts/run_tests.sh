#!/bin/bash
# run_tests.sh
# Automated test runner for matrix multiplication implementations

set -e  # Exit on error

echo "=============================================="
echo "  Matrix Multiplication Test Suite"
echo "=============================================="
echo ""

# Colors for output
#RED='\033[0;31m'
#GREEN='\033[0;32m'
#YELLOW='\033[1;33m'
#NC='\033[0m' # No Color
 
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
MPICC=${MPICC:-mpicc}
MPIRUN=${MPIRUN:-mpirun}
MPI_PROCS=${MPI_PROCS:-4}

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

echo -e "${YELLOW}[1/3] Building tests...${NC}"
cd "$BUILD_DIR"

# Build correctness test for Serial + OMP 
echo "  - Compiling Serial + OMP correctness_test"
"$CC" $CFLAGS $OMP_FLAGS -o correctness_test ../test/correctness_test.c ../src/kernels.c ../src/omp_kernels.c ../src/utility.c -I../src -lm

# Build performance test for Serial + OMP
echo "  - Compiling Serial + OMP performance_test"
"$CC" $CFLAGS $OMP_FLAGS -o performance_test ../test/performance_test.c ../src/kernels.c ../src/omp_kernels.c ../src/utility.c -I../src -lm

if command -v "$MPICC" >/dev/null 2>&1; then
    echo "  - Compiling MPI + Hybrid correctness_tests..."

    # # MPI correctness
    # mpicc -O2 -fopenmp \
    #     -o mpi_correctness_test \
    #     ../test/mpi_correctness_test.c \
    #     ../src/*.c -I../src -lm
    #     ../src/kernels.c \
    "$MPICC" -O2 $OMP_FLAGS \
        -o mpi_correctness_test \
        ../test/mpi_correctness_test.c \
        ../src/omp_kernels.c \
        ../src/utility.c \
        ../src/kernels.c \
        ../src/mpi_wrapper.c -I../src -lm
        

    echo "  - Compiling MPI + Hybrid performance_tests..."
    # MPI performance
    "$MPICC" -O2 $OMP_FLAGS \
        -o mpi_performance_test \
        ../test/mpi_performance_test.c \
        ../src/omp_kernels.c \
        ../src/utility.c \
        ../src/kernels.c \
        ../src/mpi_wrapper.c -I../src -lm

    echo -e "${GREEN}✓ MPI & Hybrid build OK${NC}"
    HAS_MPI=1
else
    echo -e "${RED}MPI not found. Skipping MPI builds.${NC}"
    HAS_MPI=0
fi
echo ""

echo -e "${GREEN}Build successful!${NC}"
echo ""

echo -e "${YELLOW}[2/3] Running correctness tests...${NC}"
if ./correctness_test; then
    echo -e "${GREEN}OMP + Serial correctness tests passed!${NC}"
else
    echo -e "${RED}OMP + Serial correctness tests failed!${NC}"
    exit 1
fi
if [ "$HAS_MPI" -eq 1 ]; then
    echo -e "${YELLOW}Running MPI + hybrid correctness...${NC}"
    "$MPIRUN" -np "$MPI_PROCS" ./mpi_correctness_test naive
    echo -e "${GREEN}✓ MPI correctness passed${NC}"
    echo ""
fi
echo ""

echo -e "${YELLOW}[3/3] Running performance benchmarks...${NC}"
./performance_test
if [ "$HAS_MPI" -eq 1 ]; then
    echo "${YELLOW}Running MPI performance...${NC}"
    "$MPIRUN" -np "$MPI_PROCS" ./mpi_performance_test naive mpi
    echo "${GREEN}✓ MPI performance done${NC}"
    echo ""
fi
echo ""

echo -e "${GREEN}=============================================="
echo "  All tests completed successfully!"
echo "==============================================${NC}"
