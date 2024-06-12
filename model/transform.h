/**
 * @file transform.h
 * @attention As gnuplot can handle signal data conversion to prepare histogram,
 * transform.h/transform.c has become redundant as for ZAD1
 * @attention As gnuplot can handle signal data conversion to prepare histogram, the following functions became redundant:
 * rsignal_to_histogram_transform(), histogram_data_aloc_codomain(), histogram_data_free_codomain()
*/

#include <stdint.h>
#include "signal.h"

#define HISTOGRAM_TRANSFORM_USE_LINEAR_SEARCH
//#define HISTOGRAM_TRANSFORM_USE_BINARY_SEARCH

typedef struct {
    double domain_min; // Minimum signal-for-histogram value
    double domain_max; // Maximum signal-for-histogram value
    ////double interval_length;
    uint64_t num_intervals;
    uint64_t* codomain_values;
} histogram_data_t;

typedef struct {
    // Determines the number of steps when constructing the Walsh-Hadamard matrix
    uint64_t m; 
} walsh_hadamard_config_t;

typedef histogram_data_t* pHistogram_data_t;

void histogram_data_aloc_codomain (pHistogram_data_t pHistogramData);
void histogram_data_free_codomain (pHistogram_data_t pHistogramData);

/**
 * Transforms real signal to histogram data by switching from the time domain to the amplitude domain
 * @attention You need to call `histogram_data_free_codomain()` after the output struct is no longer used
*/
histogram_data_t rsignal_to_histogram_transform(real_signal_t* pRealSignal, uint64_t numIntervals);


double* transform_generate_matrix_walsh_hadamard_recursive(uint64_t m);
double* transform_generate_matrix_walsh_hadamard_normalized_recursive(uint64_t m);

complex_signal_t transform_dft_real_naive(real_signal_t* pRealSignal);
real_signal_t transform_walsh_hadamard_real_naive(real_signal_t* pRealSignal, walsh_hadamard_config_t* pConfig);
