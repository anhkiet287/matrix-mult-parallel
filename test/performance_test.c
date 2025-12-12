// performance_test.c
// Benchmark serial/OpenMP kernels with unified logging, warmups, and repetitions.

#include "../src/kernels.h"
#include "../src/logging.h"
#include "../src/omp_kernels.h"
#include "../src/utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define DEFAULT_PERF_RUNS 5
#define DEFAULT_WARMUP_RUNS 1
#define DEFAULT_TOLERANCE 1e-6

typedef struct {
    const char *name;
    const char *algo;
    const char *approach;
    void (*fn)(double*, double*, double*, int);
} kernel_entry;

typedef struct {
    double median;
    double min;
    double max;
    double mean;
} run_stats;

static int get_env_int(const char *name, int fallback) {
    const char *val = getenv(name);
    if (!val || !*val) return fallback;
    char *end = NULL;
    long v = strtol(val, &end, 10);
    if (end == val || v <= 0) return fallback;
    return (int)v;
}

static double get_env_double(const char *name, double fallback) {
    const char *val = getenv(name);
    if (!val || !*val) return fallback;
    char *end = NULL;
    double v = strtod(val, &end);
    if (end == val || v <= 0.0) return fallback;
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

static int *parse_int_list_string(const char *val, int *count) {
    if (!val || !*val) {
        *count = 0;
        return NULL;
    }

    int separators = 0;
    for (const char *p = val; *p; ++p) {
        if (*p == ',' || *p == ' ' || *p == ';') separators++;
    }
    int max = separators + 1;
    int *values = (int *)malloc((size_t)max * sizeof(int));
    if (!values) {
        *count = 0;
        return NULL;
    }

    int idx = 0;
    const char *start = val;
    char *end = NULL;
    while (*start && idx < max) {
        long v = strtol(start, &end, 10);
        if (start == end) break;
        if (v > 0) values[idx++] = (int)v;
        if (*end == ',' || *end == ' ' || *end == ';') end++;
        start = end;
    }

    if (idx == 0) {
        free(values);
        *count = 0;
        return NULL;
    }

    *count = idx;
    return values;
}

static int *parse_sizes(const char *env_var, int *count) {
    const char *val = getenv(env_var);
    if (!val || !*val) {
        *count = 0;
        return NULL;
    }
    return parse_int_list_string(val, count);
}

static int kernel_enabled(const char *list, const char *name) {
    if (!list || !*list) return 1;
    char *copy = strdup(list);
    if (!copy) return 1;
    int enabled = 0;
    char *token = strtok(copy, ", ");
    while (token) {
        if (strcmp(token, name) == 0) {
            enabled = 1;
            break;
        }
        token = strtok(NULL, ", ");
    }
    free(copy);
    return enabled;
}

static void maybe_force_blas_threads(void) {
    static int applied = 0;
    if (applied) return;
    const char *allow = getenv("BLAS_ALLOW_THREADS");
    if (allow && strcmp(allow, "1") == 0) {
        printf("[blas] BLAS_ALLOW_THREADS=1 -> using external BLAS thread settings\n");
        applied = 1;
        return;
    }
    setenv("OPENBLAS_NUM_THREADS", "1", 1);
    setenv("MKL_NUM_THREADS", "1", 1);
    setenv("BLIS_NUM_THREADS", "1", 1);
    printf("[blas] forcing BLAS threads to 1 (set BLAS_ALLOW_THREADS=1 to override)\n");
    applied = 1;
}

static void append_blas_note(experiment_record *rec) {
    const char *backend = matmul_blas_backend();
    if (!backend || !*backend) {
        backend = "unknown";
    }
    char extra[64];
    snprintf(extra, sizeof(extra), "blas=%s", backend);
    if (rec->note[0] != '\0') {
        strncat(rec->note, ";", sizeof(rec->note) - strlen(rec->note) - 1);
    }
    strncat(rec->note, extra, sizeof(rec->note) - strlen(rec->note) - 1);
}

static run_stats measure_kernel(const kernel_entry *entry,
                                double *A, double *B, double *C, int n,
                                int repetitions, int warmup_runs) {
    run_stats stats = {0.0, 0.0, 0.0, 0.0};
    if (repetitions <= 0) repetitions = 1;
    if (warmup_runs < 0) warmup_runs = 0;

    double *times = (double *)malloc((size_t)repetitions * sizeof(double));
    if (!times) {
        fprintf(stderr, "Warning: failed to allocate timing buffer; falling back to single run.\n");
        repetitions = 1;
        times = (double *)malloc(sizeof(double));
        if (!times) {
            return stats;
        }
    }

    for (int w = 0; w < warmup_runs; ++w) {
        matrix_zero_init(C, n);
        entry->fn(A, B, C, n);
    }

    for (int run = 0; run < repetitions; ++run) {
        matrix_zero_init(C, n);
        double start = get_wtime();
        entry->fn(A, B, C, n);
        double end = get_wtime();
        times[run] = end - start;
    }

    stats = compute_stats(times, repetitions);
    free(times);
    return stats;
}

static void print_result_line(const experiment_record *rec) {
    printf("algo=%-8s approach=%-6s n=%5d nthreads=%2d "
           "time_med=%8.4fs (min=%8.4fs mean=%8.4fs max=%8.4fs) "
           "gemm_eq_GF/s=%8.2f",
           rec->algo,
           rec->approach,
           rec->n,
           rec->nthreads,
           rec->time_sec,
           rec->time_min,
           rec->time_mean,
           rec->time_max,
           rec->gflops_gemm_eq);
    if (rec->speedup_vs_naive > 0.0) {
        printf(" speedup=%.2fx", rec->speedup_vs_naive);
    } else {
        printf(" speedup=--");
    }
    printf(" passed=%s\n", rec->passed ? "true" : "false");
}

int main() {
    printf("=== Matrix Multiplication Performance Benchmark (Serial/OpenMP) ===\n\n");

    int num_sizes = 0;
    int *sizes = parse_sizes("TEST_PERFORMANCE_SIZES", &num_sizes);
    if (num_sizes == 0 || !sizes) {
        fprintf(stderr, "Error: TEST_PERFORMANCE_SIZES is not set or invalid.\n");
        free(sizes);
        return 1;
    }

    int repetitions = get_env_int("TEST_PERFORMANCE_RUNS", DEFAULT_PERF_RUNS);
    int warmup_runs = get_env_int("WARMUP_RUNS", DEFAULT_WARMUP_RUNS);
    double tolerance = get_env_double("TEST_CORRECTNESS_TOLERANCE", DEFAULT_TOLERANCE);
    const char *kernel_list = getenv("PERFORMANCE_KERNELS");

    const kernel_entry kernels[] = {
        {"matmul_serial",   "naive",    "serial", matmul_serial},
        {"matmul_omp",      "naive",    "openmp", matmul_omp},
        {"strassen_serial", "strassen", "serial", strassen_serial},
        {"strassen_omp",    "strassen", "openmp", strassen_omp},
        {"proposed_serial", "proposed", "serial", proposed_serial},
        {"proposed_omp",    "proposed", "openmp", proposed_omp},
        {"matmul_blas",     "blas",     "serial", matmul_blas}
    };
    const size_t kernel_count = sizeof(kernels) / sizeof(kernels[0]);

    int thread_list_count = 0;
    int *thread_list_values = parse_int_list_string(getenv("OMP_THREAD_LIST"), &thread_list_count);
    int default_openmp_threads = mm_get_omp_thread_count();
    if (default_openmp_threads <= 0) default_openmp_threads = 1;
    int default_thread_array[1] = { default_openmp_threads };
    int thread_list_owned = 1;
    if (thread_list_count == 0 || !thread_list_values) {
        thread_list_values = default_thread_array;
        thread_list_count = 1;
        thread_list_owned = 0;
    }
    const int serial_thread_value = 1;

    experiment_logger logger;
    experiment_logger_init(&logger, "openmp");

    int blas_unavailable_warned = 0;

    for (int i = 0; i < num_sizes; i++) {
        int n = sizes[i];
        printf("Matrix size: %dx%d\n", n, n);

        double *A = matrix_allocate(n);
        double *B = matrix_allocate(n);
        double *baseline = matrix_allocate(n);
        double *C = matrix_allocate(n);
        if (!A || !B || !baseline || !C) {
            fprintf(stderr, "Error: Failed to allocate matrices for n=%d\n", n);
            matrix_free(A); matrix_free(B); matrix_free(baseline); matrix_free(C);
            free(sizes);
            experiment_logger_close(&logger);
            return 1;
        }

        srand(42);
        matrix_random_init(A, n);
        srand(123);
        matrix_random_init(B, n);
        matrix_zero_init(baseline, n);
        matmul_serial(A, B, baseline, n);

        double naive_serial_baseline = -1.0;

        for (size_t k = 0; k < kernel_count; k++) {
            if (!kernel_enabled(kernel_list, kernels[k].name)) {
                continue;
            }
            if (strcmp(kernels[k].name, "matmul_blas") == 0 &&
                !matmul_blas_available()) {
                if (!blas_unavailable_warned) {
                    printf("[blas] Skipping BLAS baseline (rebuild with USE_OPENBLAS=1)\n");
                    blas_unavailable_warned = 1;
                }
                continue;
            }

            int pass_thread_counts = 1;
            const int *thread_values = &serial_thread_value;
            if (strcmp(kernels[k].approach, "openmp") == 0) {
                thread_values = thread_list_values;
                pass_thread_counts = thread_list_count;
            } else if (strcmp(kernels[k].algo, "blas") == 0) {
                thread_values = &serial_thread_value;
                pass_thread_counts = 1;
                maybe_force_blas_threads();
            }

            for (int t_idx = 0; t_idx < pass_thread_counts; ++t_idx) {
                int current_threads = thread_values[t_idx];
                if (current_threads <= 0) continue;
#ifdef _OPENMP
                if (strcmp(kernels[k].approach, "openmp") == 0) {
                    omp_set_num_threads(current_threads);
                }
#endif

                run_stats stats = measure_kernel(&kernels[k], A, B, C, n, repetitions, warmup_runs);

                experiment_record rec;
                memset(&rec, 0, sizeof(rec));
                mm_make_timestamp(rec.timestamp, sizeof(rec.timestamp));
                snprintf(rec.machine_id, sizeof(rec.machine_id), "%s", mm_get_machine_id());
                snprintf(rec.note, sizeof(rec.note), "%s", mm_get_results_note());
                snprintf(rec.algo, sizeof(rec.algo), "%s", kernels[k].algo);
                snprintf(rec.approach, sizeof(rec.approach), "%s", kernels[k].approach);
                rec.n = n;
                rec.nprocs = 1;
                rec.nthreads = strcmp(kernels[k].approach, "openmp") == 0 ? current_threads : 1;
                rec.repetitions = repetitions;
                rec.time_sec = stats.median;
                rec.time_min = stats.min;
                rec.time_max = stats.max;
                rec.time_mean = stats.mean;

                double denom = (stats.median > 0.0) ? stats.median : stats.mean;
                if (denom <= 0.0) denom = 1.0;
                double ops = 2.0 * n * (double)n * (double)n;
                rec.gflops_gemm_eq = ops / (denom * 1e9);

                rec.passed = matrix_compare(C, baseline, n, tolerance);

                if (strcmp(kernels[k].algo, "blas") == 0) {
                    append_blas_note(&rec);
                }

                if (strcmp(kernels[k].algo, "naive") == 0 &&
                    strcmp(kernels[k].approach, "serial") == 0) {
                    naive_serial_baseline = rec.time_sec;
                    rec.speedup_vs_naive = 1.0;
                } else if (naive_serial_baseline > 0.0 && rec.time_sec > 0.0) {
                    rec.speedup_vs_naive = naive_serial_baseline / rec.time_sec;
                } else {
                    rec.speedup_vs_naive = 0.0;
                }

                print_result_line(&rec);
                experiment_logger_write(&logger, &rec);
            }
        }

        printf("\n");
        matrix_free(A);
        matrix_free(B);
        matrix_free(baseline);
        matrix_free(C);
    }

    free(sizes);
    if (thread_list_owned) {
        free(thread_list_values);
    }
    experiment_logger_close(&logger);
    printf("Benchmark complete.\n");
    return 0;
}
