// omp_kernels.h
// Header for OpenMP kernel implementations
// NOTE: This header is OPTIONAL if using kernels.h directly
// Can be used if you want to separate OpenMP-specific declarations

#ifndef OMP_KERNELS_H
#define OMP_KERNELS_H

// Include base kernels header (which already has these prototypes)
#include "kernels.h"

// If you want OMP-specific helper functions, declare here
// For example:
// void omp_set_threads(int num_threads);
// int omp_get_max_threads_available();

#endif // OMP_KERNELS_H
