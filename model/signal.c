#include "signal.h"
#include <stdlib.h>
#include <memory.h>

void real_signal_free_values(real_signal_t* pSignal) {
    free(pSignal->pValues);
}
void complex_signal_free_values(complex_signal_t* pSignal) {
    free(pSignal->pValues);
}

void real_signal_alloc_values(real_signal_t* pSignal) {
    pSignal->pValues = malloc(pSignal->info.num_samples * sizeof(double));
}
void complex_signal_alloc_values(complex_signal_t* pSignal) {
    pSignal->pValues = malloc(pSignal->info.num_samples * sizeof(complex_number_t));
}

/**
 * @verify
*/
void signal_domain_adjust_start_time(real_signal_t* pSignal, double newStartTime) {
    double t1Diff = pSignal->info.start_time - newStartTime;
    if (t1Diff > 0) {
        uint64_t num_samples_to_prepend = (uint64_t)(t1Diff * pSignal->info.sampling_frequency);
        real_signal_t tempSignal = { 
            .info = {
                .num_samples = pSignal->info.num_samples + num_samples_to_prepend,
                .sampling_frequency = pSignal->info.sampling_frequency,
                .start_time = newStartTime
            },
            .pValues = 0
        };
        real_signal_alloc_values (&tempSignal);
        for (uint64_t i = 0; i < num_samples_to_prepend; i++) {
            tempSignal.pValues[i] = 0;
        }
        for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
            tempSignal.pValues[i + num_samples_to_prepend] = pSignal->pValues[i];
        }
        real_signal_free_values (pSignal);
        pSignal->pValues = tempSignal.pValues;
        pSignal->info.num_samples = tempSignal.info.num_samples;
        pSignal->info.start_time = tempSignal.info.start_time;
    } else if (t1Diff < 0) {
        uint64_t num_samples_to_delete = (uint64_t)((-t1Diff) * pSignal->info.sampling_frequency);
        uint64_t new_num_samples = pSignal->info.num_samples - num_samples_to_delete;
        memmove(pSignal->pValues, pSignal->pValues + num_samples_to_delete, sizeof(double) * new_num_samples);
        pSignal->pValues = (double*) realloc(pSignal->pValues, sizeof(double) * new_num_samples);

        pSignal->info.num_samples = new_num_samples;
        pSignal->info.start_time = newStartTime;
    }
}

/** @verify */
void signal_domain_adjust_end_time(real_signal_t* pSignal, double oldEndTime, double newEndTime) {
    double t2Diff = newEndTime - oldEndTime;
    if (t2Diff > 0) {
        uint64_t num_samples_to_append = (uint64_t)(t2Diff * pSignal->info.sampling_frequency);
        real_signal_t tempSignal = {
            .info = {
                .num_samples = pSignal->info.num_samples + num_samples_to_append,
                .sampling_frequency = pSignal->info.sampling_frequency,
                .start_time = pSignal->info.start_time
            },
            .pValues = 0
        };
        real_signal_alloc_values (&tempSignal);
        for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
            tempSignal.pValues[i] = pSignal->pValues[i];
        }
        for (uint64_t i = 0; i < num_samples_to_append; i++) {
            tempSignal.pValues[i + pSignal->info.num_samples] = 0;
        }
        real_signal_free_values (pSignal);
        pSignal->pValues = tempSignal.pValues;
        pSignal->info.num_samples = tempSignal.info.num_samples;
    } else if (t2Diff < 0) {
        uint64_t num_samples_to_delete = (uint64_t)((-t2Diff) * pSignal->info.sampling_frequency);
        uint64_t new_num_samples = pSignal->info.num_samples - num_samples_to_delete;
        pSignal->pValues = (double*) realloc(pSignal->pValues, sizeof(double) * new_num_samples);
        
        pSignal->info.num_samples = new_num_samples;
    }
}

void real_signal_timeshift(real_signal_t* pSignal, double timeshiftValue) {
    pSignal->info.start_time += timeshiftValue;
}

void real_signal_collapse_signals_tdomains(real_signal_t* pSignal_1, real_signal_t* pSignal_2) {
    double sig1_dt = 1.0 / pSignal_1->info.sampling_frequency;
    double sig2_dt = 1.0 / pSignal_2->info.sampling_frequency;

    double sig1_startTime = pSignal_1->info.start_time;
    double sig1_endTime = sig1_startTime + sig1_dt * pSignal_1->info.num_samples;
    double sig2_startTime = pSignal_2->info.start_time;
    double sig2_endTime = sig2_startTime + sig2_dt * pSignal_2->info.num_samples;
    
    double newStartTime = sig1_startTime < sig2_startTime ? sig2_startTime : sig1_startTime;
    double newEndTime = sig1_endTime > sig2_endTime ? sig2_endTime : sig1_endTime;
    
    signal_domain_adjust_start_time(pSignal_1, newStartTime);
    signal_domain_adjust_start_time(pSignal_2, newStartTime);
    signal_domain_adjust_end_time(pSignal_1, sig1_endTime, newEndTime);
    signal_domain_adjust_end_time(pSignal_2, sig2_endTime, newEndTime);
    
}
