// logging.h
// Experiment logging helpers for standardized CSV / JSON output.

#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

typedef enum {
    LOG_FORMAT_NONE = 0,
    LOG_FORMAT_CSV,
    LOG_FORMAT_JSON
} log_format_t;

typedef struct {
    char timestamp[64];
    char machine_id[64];
    char algo[32];
    char approach[16];
    int n;
    int nprocs;
    int nthreads;
    int repetitions;
    double time_sec;     // median time
    double time_min;
    double time_max;
    double time_mean;
    double gflops_gemm_eq;
    double speedup_vs_naive;
    int passed;  // 1 = pass, 0 = fail
    char note[128];
} experiment_record;

typedef struct {
    FILE *fp;
    log_format_t format;
    int wrote_header;
} experiment_logger;

// experiment_logger_init
// Input:
//   logger: pointer to opaque logger state.
//   category: filename prefix (e.g., "openmp") for deterministic outputs.
// Behavior:
//   When RESULTS_DIR is set, ensures the directory exists, opens/creates
//   <RESULTS_DIR>/<category>_results.{csv|json}, and prepares the logger.
//   Falls back to NO-OP logging when RESULTS_DIR is unset.
// Returns 0 on success, -1 on failure.
int experiment_logger_init(experiment_logger *logger, const char *category);

// experiment_logger_write
// Input: logger (may be NULL) and populated experiment_record.
// Behavior: appends a CSV or JSON line and flushes immediately when logging is enabled.
void experiment_logger_write(experiment_logger *logger, const experiment_record *record);

// experiment_logger_close
// Behavior: closes the backing file handle if logging was enabled.
void experiment_logger_close(experiment_logger *logger);

// mm_make_timestamp
// Behavior: writes an ISO-8601 UTC timestamp (YYYY-MM-DDTHH:MM:SSZ) into buf.
void mm_make_timestamp(char *buf, size_t len);

// mm_get_machine_id
// Output: MACHINE_ID environment variable or "unknown" when unset.
const char *mm_get_machine_id(void);

// mm_get_results_note
// Output: RESULTS_NOTE environment variable or "" when unset.
const char *mm_get_results_note(void);

// mm_get_omp_thread_count
// Output: maximum OpenMP thread count on this binary; returns 1 when OpenMP is unavailable.
int mm_get_omp_thread_count(void);

#endif // LOGGING_H
