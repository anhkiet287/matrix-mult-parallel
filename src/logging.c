// logging.c
// Implements experiment logging helpers for CSV / JSON output files.

#include "logging.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifdef _OPENMP
#include <omp.h>
#endif

static log_format_t detect_format(const char *fmt) {
    if (!fmt || !*fmt) {
        return LOG_FORMAT_CSV;
    }
    if (strcmp(fmt, "csv") == 0) {
        return LOG_FORMAT_CSV;
    }
    if (strcmp(fmt, "json") == 0) {
        return LOG_FORMAT_JSON;
    }
    return LOG_FORMAT_CSV;
}

static void write_csv_header(FILE *fp) {
    fprintf(fp,
            "timestamp,machine_id,algo,approach,n,nprocs,nthreads,repetitions,"
            "time_sec,time_min,time_max,time_mean,gflops_gemm_eq,passed,"
            "speedup_vs_naive,note\n");
    fflush(fp);
}

static int ensure_dir(const char *path) {
    if (!path || !*path) {
        return -1;
    }

    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if (len == 0) return -1;

    for (size_t i = 1; i < len; i++) {
        if (tmp[i] == '/' || tmp[i] == '\\') {
            char saved = tmp[i];
            tmp[i] = '\0';
            if (tmp[0]) {
                if (mkdir(tmp, 0777) != 0 && errno != EEXIST) {
                    return -1;
                }
            }
            tmp[i] = saved;
        }
    }

    if (mkdir(tmp, 0777) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

int experiment_logger_init(experiment_logger *logger, const char *category) {
    if (!logger) {
        return -1;
    }
    memset(logger, 0, sizeof(*logger));

    const char *results_dir = getenv("RESULTS_DIR");
    if (!results_dir || !*results_dir) {
        logger->format = LOG_FORMAT_NONE;
        return 0;
    }

    if (ensure_dir(results_dir) != 0) {
        logger->format = LOG_FORMAT_NONE;
        return -1;
    }

    logger->format = detect_format(getenv("RESULTS_FORMAT"));
    const char *basename = getenv("RESULTS_FILE_BASENAME");
    if (!basename || !*basename) {
        basename = category ? category : "results";
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/%s_results.%s",
             results_dir,
             basename,
             (logger->format == LOG_FORMAT_JSON) ? "json" : "csv");

    logger->fp = fopen(path, "a");
    if (!logger->fp) {
        logger->format = LOG_FORMAT_NONE;
        return -1;
    }

    if (logger->format == LOG_FORMAT_CSV) {
        long pos = ftell(logger->fp);
        if (pos == 0) {
            write_csv_header(logger->fp);
            logger->wrote_header = 1;
        }
    }

    return 0;
}

void experiment_logger_write(experiment_logger *logger, const experiment_record *record) {
    if (!logger || !record || logger->format == LOG_FORMAT_NONE || !logger->fp) {
        return;
    }

    if (logger->format == LOG_FORMAT_CSV) {
        fprintf(logger->fp,
                "%s,%s,%s,%s,%d,%d,%d,%d,%.6f,%.6f,%.6f,%.6f,%.4f,%s,%.4f,%s\n",
                record->timestamp,
                record->machine_id,
                record->algo,
                record->approach,
                record->n,
                record->nprocs,
                record->nthreads,
                record->repetitions,
                record->time_sec,
                record->time_min,
                record->time_max,
                record->time_mean,
                record->gflops_gemm_eq,
                record->passed ? "true" : "false",
                record->speedup_vs_naive,
                record->note);
    } else if (logger->format == LOG_FORMAT_JSON) {
        fprintf(logger->fp,
                "{\"timestamp\":\"%s\",\"machine_id\":\"%s\",\"algo\":\"%s\",\"approach\":\"%s\","
                "\"n\":%d,\"nprocs\":%d,\"nthreads\":%d,\"repetitions\":%d,"
                "\"time_sec\":%.6f,\"time_min\":%.6f,\"time_max\":%.6f,\"time_mean\":%.6f,"
                "\"gflops_gemm_eq\":%.4f,\"passed\":%s,\"speedup_vs_naive\":%.4f,"
                "\"note\":\"%s\"}\n",
                record->timestamp,
                record->machine_id,
                record->algo,
                record->approach,
                record->n,
                record->nprocs,
                record->nthreads,
                record->repetitions,
                record->time_sec,
                record->time_min,
                record->time_max,
                record->time_mean,
                record->gflops_gemm_eq,
                record->passed ? "true" : "false",
                record->speedup_vs_naive,
                record->note);
    }
    fflush(logger->fp);
}

void experiment_logger_close(experiment_logger *logger) {
    if (!logger) return;
    if (logger->fp) {
        fclose(logger->fp);
    }
    logger->fp = NULL;
    logger->format = LOG_FORMAT_NONE;
}

void mm_make_timestamp(char *buf, size_t len) {
    if (!buf || len == 0) return;
    time_t t = time(NULL);
    struct tm tm_info;
#if defined(_WIN32)
    gmtime_s(&tm_info, &t);
#else
    gmtime_r(&t, &tm_info);
#endif
    strftime(buf, len, "%Y-%m-%dT%H:%M:%SZ", &tm_info);
}

const char *mm_get_machine_id(void) {
    const char *id = getenv("MACHINE_ID");
    return (id && *id) ? id : "unknown";
}

const char *mm_get_results_note(void) {
    const char *note = getenv("RESULTS_NOTE");
    return (note && *note) ? note : "";
}

int mm_get_omp_thread_count(void) {
#ifdef _OPENMP
    return omp_get_max_threads();
#else
    return 1;
#endif
}
