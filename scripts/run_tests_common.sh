#!/bin/bash
# Shared helpers for the run_tests_*.sh scripts.
# Provides compiler detection, build helpers, and wrappers around test binaries.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

if [ -f "$PROJECT_ROOT/config/test_settings.sh" ]; then
    # shellcheck source=/dev/null
    source "$PROJECT_ROOT/config/test_settings.sh"
fi

export TEST_CORRECTNESS_SIZE
export TEST_CORRECTNESS_TOLERANCE
export CORRECTNESS_KERNELS
export TEST_PERFORMANCE_SIZES
export TEST_PERFORMANCE_RUNS
export PERFORMANCE_KERNELS
export MPI_TEST_SIZE
export MPI_PERF_SIZES
export MPI_PERF_RUNS

: "${BUILD_DIR:=$PROJECT_ROOT/build}"
: "${CC:=gcc}"
: "${CFLAGS:=-O2 -Wall}"
DEFAULT_OMP_FLAGS="-fopenmp"
: "${MPICC:=mpicc}"
: "${MPIRUN:=mpirun}"
: "${MPI_PROCS:=4}"
: "${MPI_ALGORITHM:=}"
: "${HYBRID_ALGORITHM:=}"
: "${MPI_ALGORITHMS:=}"
: "${HYBRID_ALGORITHMS:=}"
: "${USE_OPENBLAS:=0}"
: "${OPENBLAS_DIR:=}"

CBLAS_CFLAGS=""
CBLAS_LIBS=""
if [ "$USE_OPENBLAS" = "1" ]; then
    CBLAS_CFLAGS="-DUSE_CBLAS"
    if [ -n "$OPENBLAS_DIR" ]; then
        CBLAS_CFLAGS="$CBLAS_CFLAGS -I${OPENBLAS_DIR}/include"
        CBLAS_LIBS="-L${OPENBLAS_DIR}/lib -lopenblas"
    else
        CBLAS_LIBS="-lopenblas"
    fi
fi

RED=""; GREEN=""; YELLOW=""; NC=""

setup_colors() {
    if [ -t 1 ] && [ -z "${NO_COLOR+x}" ]; then
        RED=$'\033[0;31m'
        GREEN=$'\033[0;32m'
        YELLOW=$'\033[1;33m'
        NC=$'\033[0m'
    fi
}

detect_openmp_flags() {
    if [ -n "${OMP_FLAGS:-}" ]; then
        return
    fi
    if [ -n "${_OMP_FLAGS_SET:-}" ]; then
        return
    fi

    local tmp_source="int main(){return 0;}"
    local tmp_bin
    tmp_bin="$(mktemp)"
    if echo "$tmp_source" | "$CC" $CFLAGS $DEFAULT_OMP_FLAGS -x c - -o "$tmp_bin" >/dev/null 2>&1; then
        OMP_FLAGS=$DEFAULT_OMP_FLAGS
    else
        echo "${YELLOW}Warning: compiler '$CC' lacks OpenMP support, continuing without -fopenmp${NC}"
        OMP_FLAGS=""
    fi
    rm -f "$tmp_bin"
    _OMP_FLAGS_SET=1
}

ensure_build_dir() {
    mkdir -p "$BUILD_DIR"
}

build_serial_omp_binaries() {
    detect_openmp_flags
    ensure_build_dir
    echo "${YELLOW}[build] Serial/OpenMP correctness + performance${NC}"
    pushd "$BUILD_DIR" >/dev/null
    "$CC" $CFLAGS ${OMP_FLAGS:-} $CBLAS_CFLAGS -o correctness_test \
        "$PROJECT_ROOT/test/correctness_test.c" \
        "$PROJECT_ROOT/src/kernels.c" \
        "$PROJECT_ROOT/src/blas_kernel.c" \
        "$PROJECT_ROOT/src/logging.c" \
        "$PROJECT_ROOT/src/omp_kernels.c" \
        "$PROJECT_ROOT/src/utility.c" -I"$PROJECT_ROOT/src" -lm $CBLAS_LIBS
    "$CC" $CFLAGS ${OMP_FLAGS:-} $CBLAS_CFLAGS -o performance_test \
        "$PROJECT_ROOT/test/performance_test.c" \
        "$PROJECT_ROOT/src/kernels.c" \
        "$PROJECT_ROOT/src/blas_kernel.c" \
        "$PROJECT_ROOT/src/logging.c" \
        "$PROJECT_ROOT/src/omp_kernels.c" \
        "$PROJECT_ROOT/src/utility.c" -I"$PROJECT_ROOT/src" -lm $CBLAS_LIBS
    popd >/dev/null
}

build_mpi_binaries() {
    detect_openmp_flags
    ensure_build_dir

    if ! command -v "$MPICC" >/dev/null 2>&1; then
        echo "${RED}Error: MPICC ('$MPICC') not found in PATH${NC}"
        return 1
    fi

    echo "${YELLOW}[build] MPI/Hybrid correctness + performance${NC}"
    pushd "$BUILD_DIR" >/dev/null
    "$MPICC" -O2 ${OMP_FLAGS:-} $CBLAS_CFLAGS -o mpi_correctness_test \
        "$PROJECT_ROOT/test/mpi_correctness_test.c" \
        "$PROJECT_ROOT/src/blas_kernel.c" \
        "$PROJECT_ROOT/src/logging.c" \
        "$PROJECT_ROOT/src/omp_kernels.c" \
        "$PROJECT_ROOT/src/utility.c" \
        "$PROJECT_ROOT/src/kernels.c" \
        "$PROJECT_ROOT/src/mpi_wrapper.c" -I"$PROJECT_ROOT/src" -lm $CBLAS_LIBS
    "$MPICC" -O2 ${OMP_FLAGS:-} $CBLAS_CFLAGS -o mpi_performance_test \
        "$PROJECT_ROOT/test/mpi_performance_test.c" \
        "$PROJECT_ROOT/src/blas_kernel.c" \
        "$PROJECT_ROOT/src/logging.c" \
        "$PROJECT_ROOT/src/omp_kernels.c" \
        "$PROJECT_ROOT/src/utility.c" \
        "$PROJECT_ROOT/src/kernels.c" \
        "$PROJECT_ROOT/src/mpi_wrapper.c" -I"$PROJECT_ROOT/src" -lm $CBLAS_LIBS
    popd >/dev/null
}

require_mpi_runtime() {
    if ! command -v "$MPIRUN" >/dev/null 2>&1; then
        echo "${RED}Error: MPIRUN ('$MPIRUN') not found in PATH${NC}"
        return 1
    fi
    return 0
}

run_serial_omp_correctness() {
    pushd "$BUILD_DIR" >/dev/null
    ./correctness_test
    popd >/dev/null
}

run_serial_omp_performance() {
    pushd "$BUILD_DIR" >/dev/null
    ./performance_test
    popd >/dev/null
}

run_mpi_correctness() {
    local algorithm=$1
    local mode=${2:-mpi}
    local procs=${3:-$MPI_PROCS}
    pushd "$BUILD_DIR" >/dev/null
    "$MPIRUN" -np "$procs" ./mpi_correctness_test "$algorithm" "$mode"
    popd >/dev/null
}

run_mpi_performance() {
    local algorithm=$1
    local mode=$2
    local procs=${3:-$MPI_PROCS}
    pushd "$BUILD_DIR" >/dev/null
    "$MPIRUN" -np "$procs" ./mpi_performance_test "$algorithm" "$mode"
    popd >/dev/null
}

normalize_algorithm_list() {
    local value="$1"
    value="${value//,/ }"
    echo "$value"
}

get_algorithm_list() {
    local multi="$1"
    local single="$2"
    local fallback="$3"

    if [ -n "$multi" ]; then
        normalize_algorithm_list "$multi"
    elif [ -n "$single" ]; then
        normalize_algorithm_list "$single"
    else
        echo "$fallback"
    fi
}
