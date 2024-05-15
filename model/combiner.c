#include "combiner.h"
#include <stdio.h> //for logging
#include <math.h>
#include <stdlib.h> // for exit

/**
 * @verify
*/
void __signal_domain_adjust_start_time(real_signal_t* pSignal, double newStartTime) {
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
    }
}

/** @verify */
void __signal_domain_adjust_end_time(real_signal_t* pSignal, double oldEndTime, double newEndTime) {
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
    }
}

/**
 * @returns 0 on success, 1 on failure
*/
int __prepare_real_signals_for_combination(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {
    fprintf(stdout, "Info: Preparing signals for combination\n");
    
    if (pAccumulatorSignal->info.sampling_frequency != pOtherSignal->info.sampling_frequency) {
        fprintf(stdout, "Warning: Refusing to combine signals sampled differently\n");
        return 1;
    }
    double accumulatorDt = 1.0 / pAccumulatorSignal->info.sampling_frequency;
    double otherDt = 1.0 / pOtherSignal->info.sampling_frequency;
    
    if (fmod(pAccumulatorSignal->info.start_time, accumulatorDt) != fmod(pOtherSignal->info.start_time, otherDt)) {
        /** @attention */
        fprintf(stdout, "Warning: Refusing to combine signals with unsynchronized samples\n");
        return 1;
    }

    double accumulatorStartTime = pAccumulatorSignal->info.start_time;
    double accumulatorEndTime = accumulatorStartTime + accumulatorDt * pAccumulatorSignal->info.num_samples;
    double otherStartTime = pOtherSignal->info.start_time;
    double otherEndTime = otherStartTime + otherDt * pOtherSignal->info.num_samples;
    
    double newStartTime = accumulatorStartTime < otherStartTime ? accumulatorStartTime : otherStartTime;
    double newEndTime = accumulatorEndTime > otherEndTime ? accumulatorEndTime : otherEndTime;

    __signal_domain_adjust_start_time (pAccumulatorSignal, newStartTime);
    __signal_domain_adjust_start_time (pOtherSignal, newStartTime);
    __signal_domain_adjust_end_time (pAccumulatorSignal, accumulatorEndTime, newEndTime);
    __signal_domain_adjust_end_time (pOtherSignal, otherEndTime, newEndTime);
    return 0;
}


/**
 * @optimize Expand & Narrow down instead of backup & expand & restore ?
 * @optimize DRY
*/
void add_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {
    fprintf(stdout, "Info: add_signal called\n");
    //Backup
    real_signal_t otherSignalCopy = {
        .info = {
            .num_samples = pOtherSignal->info.num_samples,
            .sampling_frequency = pOtherSignal->info.sampling_frequency,
            .start_time = pOtherSignal->info.start_time
        },
        .pValues = 0
    };
    real_signal_alloc_values (&otherSignalCopy);
    for (uint64_t i = 0; i < otherSignalCopy.info.num_samples; i++) {
        otherSignalCopy.pValues[i] = pOtherSignal->pValues[i];
    }

    //Combine
    if (__prepare_real_signals_for_combination(pAccumulatorSignal, pOtherSignal) == 1) {
        // Failed to prepare for combination
        real_signal_free_values (&otherSignalCopy);
        return;
    }
    
    fprintf(stdout, "Info: Performing signal addition\n");
    for (uint64_t i = 0; i < pAccumulatorSignal->info.num_samples; i++) {
        pAccumulatorSignal->pValues[i] += pOtherSignal->pValues[i];
    }


    //Restore
    fprintf(stdout, "Info: Freeing redundant signal data\n");
    real_signal_free_values (pOtherSignal);

    fprintf(stdout, "Info: Restoring the non-accumulator signal values\n");
    pOtherSignal->pValues = otherSignalCopy.pValues;
    fprintf(stdout, "Info: Restoring the non-accumulator signal number of samples\n");
    pOtherSignal->info.num_samples = otherSignalCopy.info.num_samples;
    fprintf(stdout, "Info: Restoring the non-accumulator signal sampling frequency\n");
    pOtherSignal->info.sampling_frequency = otherSignalCopy.info.sampling_frequency;
    fprintf(stdout, "Info: Restoring the non-accumulator signal start time\n");
    pOtherSignal->info.start_time = otherSignalCopy.info.start_time;
}

void substract_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {
    fprintf(stdout, "Info: substract_signal called\n");
    //Backup
    real_signal_t otherSignalCopy = {
        .info = {
            .num_samples = pOtherSignal->info.num_samples,
            .sampling_frequency = pOtherSignal->info.sampling_frequency,
            .start_time = pOtherSignal->info.start_time
        },
        .pValues = 0
    };
    real_signal_alloc_values (&otherSignalCopy);
    for (uint64_t i = 0; i < otherSignalCopy.info.num_samples; i++) {
        otherSignalCopy.pValues[i] = pOtherSignal->pValues[i];
    }

    //Combine
    if (__prepare_real_signals_for_combination(pAccumulatorSignal, pOtherSignal) == 1) {
        // Failed to prepare for combination
        real_signal_free_values (&otherSignalCopy);
        return;
    }
    
    for (uint64_t i = 0; i < pAccumulatorSignal->info.num_samples; i++) {
        pAccumulatorSignal->pValues[i] -= pOtherSignal->pValues[i];
    }

    //Restore
    real_signal_free_values (pOtherSignal);
    pOtherSignal->pValues = otherSignalCopy.pValues;
    pOtherSignal->info.num_samples = otherSignalCopy.info.num_samples;
    pOtherSignal->info.sampling_frequency = otherSignalCopy.info.sampling_frequency;
    pOtherSignal->info.start_time = otherSignalCopy.info.start_time;
}

void multiply_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {
    fprintf(stdout, "Info: multiply_signal called\n");
    //Backup
    real_signal_t otherSignalCopy = {
        .info = {
            .num_samples = pOtherSignal->info.num_samples,
            .sampling_frequency = pOtherSignal->info.sampling_frequency,
            .start_time = pOtherSignal->info.start_time
        },
        .pValues = 0
    };
    real_signal_alloc_values (&otherSignalCopy);
    for (uint64_t i = 0; i < otherSignalCopy.info.num_samples; i++) {
        otherSignalCopy.pValues[i] = pOtherSignal->pValues[i];
    }

    //Combine
    if (__prepare_real_signals_for_combination(pAccumulatorSignal, pOtherSignal) == 1) {
        // Failed to prepare for combination
        real_signal_free_values (&otherSignalCopy);
        return;
    }
    
    for (uint64_t i = 0; i < pAccumulatorSignal->info.num_samples; i++) {
        pAccumulatorSignal->pValues[i] *= pOtherSignal->pValues[i];
    }

    //Restore
    real_signal_free_values (pOtherSignal);
    pOtherSignal->pValues = otherSignalCopy.pValues;
    pOtherSignal->info.num_samples = otherSignalCopy.info.num_samples;
    pOtherSignal->info.sampling_frequency = otherSignalCopy.info.sampling_frequency;
    pOtherSignal->info.start_time = otherSignalCopy.info.start_time;
}

void divide_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {
    fprintf(stdout, "Info: divide_signal called\n");
    //Backup
    real_signal_t otherSignalCopy = {
        .info = {
            .num_samples = pOtherSignal->info.num_samples,
            .sampling_frequency = pOtherSignal->info.sampling_frequency,
            .start_time = pOtherSignal->info.start_time
        },
        .pValues = 0
    };
    real_signal_alloc_values (&otherSignalCopy);
    for (uint64_t i = 0; i < otherSignalCopy.info.num_samples; i++) {
        otherSignalCopy.pValues[i] = pOtherSignal->pValues[i];
    }

    //Combine
    if (__prepare_real_signals_for_combination(pAccumulatorSignal, pOtherSignal) == 1) {
        // Failed to prepare for combination
        real_signal_free_values (&otherSignalCopy);
        return;
    }
    
    for (uint64_t i = 0; i < pAccumulatorSignal->info.num_samples; i++) {
        pAccumulatorSignal->pValues[i] /= pOtherSignal->pValues[i];
    }

    //Restore
    real_signal_free_values (pOtherSignal);
    pOtherSignal->pValues = otherSignalCopy.pValues;
    pOtherSignal->info.num_samples = otherSignalCopy.info.num_samples;
    pOtherSignal->info.sampling_frequency = otherSignalCopy.info.sampling_frequency;
    pOtherSignal->info.start_time = otherSignalCopy.info.start_time;
}

void convolve_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {
    fprintf(stderr, "Error: Not implemented"); exit(EXIT_FAILURE);
}

void cross_correlate_signal_1(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {
    fprintf(stderr, "Error: Not implemented"); exit(EXIT_FAILURE);
}

void cross_correlate_signal_2(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {
    fprintf(stderr, "Error: Not implemented"); exit(EXIT_FAILURE);
}