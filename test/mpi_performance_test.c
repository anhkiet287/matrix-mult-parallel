// mpi_performance_test.c
// Benchmark MPI and Hybrid matrix multiplication with standardized logging.

#include "../src/kernels.h"
#include "../src/logging.h"
#include "../src/mpi_wrapper.h"
#include "../src/omp_kernels.h"
#include "../src/utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PERF_RUNS 5
#define DEFAULT_WARMUP_RUNS 1
#define DEFAULT_TOLERANCE 1e-6

typedef struct {
    double median;
    double min;
    double max;
    double mean;
} run_stats;

static int get_env_int(const char *name, int fallback) {
    const char *value = getenv(name);
    if (!value || !*value) return fallback;
    char *endptr = NULL;
    long v = strtol(value, &endptr, 10);
    if (endptr == value || v <= 0) return fallback;
    return (int)v;
}

static double get_env_double(const char *name, double fallback) {
    const char *value = getenv(name);
    if (!value || !*value) return fallback;
    char *endptr = NULL;
    double v = strtod(value, &endptr);
    if (endptr == value || v <= 0.0) return fallback;
    return v;
}

static int double_compare(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    return (da > db) - (da < db);
}

static run_stats compute_stats(double *values, int count) {
    run_stats stats = {0.0, 0.0, 0.0, 0.0};
    if (count <= 0 || !values) {
        return stats;
    }

    double min_v = values[0];
    double max_v = values[0];
    double sum = 0.0;
    for (int i = 0; i < count; ++i) {
        if (values[i] < min_v) min_v = values[i];
        if (values[i] > max_v) max_v = values[i];
        sum += values[i];
    }

    qsort(values, count, sizeof(double), double_compare);
    double median = values[count / 2];
    if (count % 2 == 0 && count > 1) {
        median = 0.5 * (values[count / 2 - 1] + values[count / 2]);
    }

    stats.median = median;
    stats.min = min_v;
    stats.max = max_v;
    stats.mean = sum / (double)count;
    return stats;
}

static int *parse_sizes(const char *primary_env, const char *fallback_env, int *count) {
    const char *val = getenv(primary_env);
    if (!val || !*val) {
        val = getenv(fallback_env);
    }
    if (!val || !*val) {
        *count = 0;
        return NULL;
    }

    int separators = 0;
    for (const char *p = val; *p; ++p) {
        if (*p == ',' || *p == ' ') separators++;
    }
    int max = separators + 1;
    int *sizes = (int *)malloc(max * sizeof(int));
    if (!sizes) {
        *count = 0;
        return NULL;
    }

    int idx = 0;
    const char *start = val;
    char *end = NULL;
    while (*start && idx < max) {
        long v = strtol(start, &end, 10);
        if (start == end) break;
        if (v > 0) sizes[idx++] = (int)v;
        if (*end == ',' || *end == ' ') end++;
        start = end;
    }

    if (idx == 0) {
        free(sizes);
        *count = 0;
        return NULL;
    }

    *count = idx;
    return sizes;
}

static void print_result_line(const experiment_record *rec) {
    printf("algo=%-8s approach=%-6s n=%5d nprocs=%2d nthreads=%2d "
           "time_med=%8.4fs (min=%8.4fs mean=%8.4fs max=%8.4fs) "
           "gemm_eq_GF/s=%8.2f passed=%s\n",
           rec->algo,
           rec->approach,
           rec->n,
           rec->nprocs,
           rec->nthreads,
           rec->time_sec,
           rec->time_min,
           rec->time_mean,
           rec->time_max,
           rec->gflops_gemm_eq,
           rec->passed ? "true" : "false");
}

int main(int argc, char **argv) {
    mpi_init(&argc, &argv);
    int rank = mpi_get_rank();
    int world_size = mpi_get_size();

    if (argc != 3) {
        if (rank == 0) {
            printf("Usage: mpirun -np <P> ./mpi_performance_test <algorithm> <mode>\n");
            printf("Algorithms: naive | strassen | proposed\n");
            printf("Mode: mpi | hybrid\n");
        }
        mpi_finalize();
        return 1;
    }

    const char *algorithm = argv[1];
    const char *mode = argv[2];
    kernel_func_t kernel = NULL;

    if (strcmp(mode, "mpi") == 0) {
        if      (strcmp(algorithm, "naive") == 0)    kernel = matmul_serial;
        else if (strcmp(algorithm, "strassen") == 0) kernel = strassen_serial;
        else if (strcmp(algorithm, "proposed") == 0) kernel = proposed_serial;
    } else if (strcmp(mode, "hybrid") == 0) {
        if      (strcmp(algorithm, "naive") == 0)    kernel = matmul_omp;
        else if (strcmp(algorithm, "strassen") == 0) kernel = strassen_omp;
        else if (strcmp(algorithm, "proposed") == 0) kernel = proposed_omp;
    }

    if (!kernel) {
        if (rank == 0) fprintf(stderr, "Unknown algorithm or mode combination.\n");
        mpi_finalize();
        return 1;
    }

    int repetitions = get_env_int("MPI_PERF_RUNS", -1);
    if (repetitions <= 0) {
        repetitions = get_env_int("TEST_PERFORMANCE_RUNS", DEFAULT_PERF_RUNS);
    }
    int warmup_runs = get_env_int("WARMUP_RUNS", DEFAULT_WARMUP_RUNS);
    double tolerance = get_env_double("TEST_CORRECTNESS_TOLERANCE", DEFAULT_TOLERANCE);
    int num_sizes = 0;
    int *sizes = parse_sizes("MPI_PERF_SIZES", "TEST_PERFORMANCE_SIZES", &num_sizes);
    if (num_sizes == 0 || !sizes) {
        if (rank == 0) {
            fprintf(stderr, "Error: MPI_PERF_SIZES or TEST_PERFORMANCE_SIZES must be set.\n");
        }
        mpi_finalize();
        free(sizes);
        return 1;
    }

    if (rank == 0) {
        printf("=== MPI Performance Benchmark (%s, %s) ===\n", algorithm, mode);
        printf("Processes: %d\n\n", world_size);
    }

    experiment_logger logger;
    experiment_logger *logger_ptr = NULL;
    if (rank == 0) {
        experiment_logger_init(&logger, (strcmp(mode, "hybrid") == 0) ? "hybrid" : "mpi");
        logger_ptr = &logger;
    }

    for (int idx = 0; idx < num_sizes; ++idx) {
        int n = sizes[idx];
        double *A = NULL;
        double *B = NULL;
        double *C = NULL;
        double *baseline = NULL;

        if (rank == 0) {
            A = matrix_allocate(n);
            B = matrix_allocate(n);
            C = matrix_allocate(n);
            baseline = matrix_allocate(n);
            if (!A || !B || !C || !baseline) {
                fprintf(stderr, "Rank 0: failed to allocate matrices for n=%d\n", n);
                mpi_finalize();
                free(sizes);
                return 1;
            }

            srand(42);
            matrix_random_init(A, n);
            srand(123);
            matrix_random_init(B, n);
            matrix_zero_init(C, n);
            matrix_zero_init(baseline, n);
            matmul_serial(A, B, baseline, n);
        } else {
            B = matrix_allocate(n);
            if (!B) {
                fprintf(stderr, "Rank %d: failed to allocate matrix B\n", rank);
                mpi_finalize();
                free(sizes);
                return 1;
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);

        for (int w = 0; w < warmup_runs; ++w) {
            if (rank == 0) matrix_zero_init(C, n);
            MPI_Barrier(MPI_COMM_WORLD);
            mpi_matmul_master_worker(A, B, C, n, kernel);
        }

        double *times = NULL;
        if (rank == 0) {
            times = (double *)malloc((size_t)repetitions * sizeof(double));
            if (!times) {
                fprintf(stderr, "Rank 0: failed to allocate timing buffer\n");
            }
        }

        for (int run = 0; run < repetitions; ++run) {
            if (rank == 0) {
                matrix_zero_init(C, n);
            }
            MPI_Barrier(MPI_COMM_WORLD);
            double start = MPI_Wtime();
            mpi_matmul_master_worker(A, B, C, n, kernel);
            double end = MPI_Wtime();
            if (rank == 0 && times) {
                times[run] = end - start;
            }
        }

        if (rank == 0) {
            run_stats stats = compute_stats(times, repetitions);
            free(times);

            double denom = (stats.median > 0.0) ? stats.median : stats.mean;
            if (denom <= 0.0) denom = 1.0;
            double gflops = (2.0 * n * (double)n * (double)n) / (denom * 1e9);
            int passed = matrix_compare(C, baseline, n, tolerance);

            experiment_record rec;
            memset(&rec, 0, sizeof(rec));
            mm_make_timestamp(rec.timestamp, sizeof(rec.timestamp));
            snprintf(rec.machine_id, sizeof(rec.machine_id), "%s", mm_get_machine_id());
            snprintf(rec.note, sizeof(rec.note), "%s", mm_get_results_note());
            snprintf(rec.algo, sizeof(rec.algo), "%s", algorithm);
            snprintf(rec.approach, sizeof(rec.approach), "%s", mode);
            rec.n = n;
            rec.nprocs = world_size;
            rec.nthreads = (strcmp(mode, "hybrid") == 0) ? mm_get_omp_thread_count() : 1;
            rec.repetitions = repetitions;
            rec.time_sec = stats.median;
            rec.time_min = stats.min;
            rec.time_max = stats.max;
            rec.time_mean = stats.mean;
            rec.gflops_gemm_eq = gflops;
            rec.passed = passed;
            rec.speedup_vs_naive = 0.0;

            print_result_line(&rec);
            experiment_logger_write(logger_ptr, &rec);

            matrix_free(A);
            matrix_free(B);
            matrix_free(C);
            matrix_free(baseline);
        } else {
            matrix_free(B);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    free(sizes);
    if (rank == 0) {
        experiment_logger_close(logger_ptr);
        printf("\nBenchmark done.\n");
    }
    mpi_finalize();
    return 0;
}
