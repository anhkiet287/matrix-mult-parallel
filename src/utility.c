// utility.c
// Implementation of utility functions
// Helper functions for matrix operations, timing, and testing

#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

double* matrix_allocate(int n) {
    double *matrix = (double *)malloc(n * n * sizeof(double));
    if (matrix == NULL) {
        fprintf(stderr, "Error: Failed to allocate matrix of size %dx%d\n", n, n);
        return NULL;
    }
    return matrix;
}

void matrix_free(double *matrix) {
    if (matrix != NULL) {
        free(matrix);
    }
}

void matrix_random_init(double *matrix, int n) {
    for (int i = 0; i < n * n; i++) {
        matrix[i] = (double)rand() / RAND_MAX; // Random value in [0, 1]
    }
}

void matrix_zero_init(double *matrix, int n) {
    for (int i = 0; i < n * n; i++) {
        matrix[i] = 0.0;
    }
}

void matrix_identity_init(double *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i * n + j] = (i == j) ? 1.0 : 0.0;
        }
    }
}

int matrix_compare(double *A, double *B, int n, double tolerance) {
    for (int i = 0; i < n * n; i++) {
        if (fabs(A[i] - B[i]) > tolerance) {
            return 0; // Not equal
        }
    }
    return 1; // Equal within tolerance
}

void matrix_print(double *matrix, int n, int max_size) {
    int print_size = (n < max_size) ? n : max_size;
    
    printf("Matrix (%dx%d):\n", n, n);
    for (int i = 0; i < print_size; i++) {
        for (int j = 0; j < print_size; j++) {
            printf("%8.4f ", matrix[i * n + j]);
        }
        if (print_size < n) {
            printf("...");
        }
        printf("\n");
    }
    if (print_size < n) {
        printf("...\n");
    }
}

double get_wtime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

double matrix_checksum(double *matrix, int n) {
    double sum = 0.0;
    for (int i = 0; i < n * n; i++) {
        sum += matrix[i];
    }
    return sum;
}
