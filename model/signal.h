#include "complex.h"
#include <stdint.h>

#ifndef __GUARD_SIGNAL_H
#define __GUARD_SIGNAL_H

/* Signal metadata struct */
typedef struct {
    double start_time;
    double sampling_frequency;
    uint64_t num_samples;
} signal_info_t;

typedef struct {
    signal_info_t info;
    double* pValues;
} real_signal_t;

typedef struct {
    signal_info_t info;
    complex_number_t* pValues;
} complex_signal_t;

typedef union {
    real_signal_t real_signal;
    complex_signal_t complex_signal;
} signal_t;

void real_signal_free_values(real_signal_t* pSignal);
void complex_signal_free_values(complex_signal_t* pSignal);
void real_signal_alloc_values(real_signal_t* pSignal);
void complex_signal_alloc_values(complex_signal_t* pSignal);

void signal_domain_adjust_start_time(real_signal_t* pSignal, double newStartTime);
void signal_domain_adjust_end_time(real_signal_t* pSignal, double oldEndTime, double newEndTime);

void signal_reverse(real_signal_t* pSignal);

void real_signal_timeshift(real_signal_t* pSignal, double timeshiftValue);
void real_signal_collapse_signals_tdomains(real_signal_t* pSignal_1, real_signal_t* pSignal_2);

#endif