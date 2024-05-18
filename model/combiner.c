#include "combiner.h"
#include <stdio.h> //for logging
#include <math.h>
#include <stdlib.h> // for exit



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

    signal_domain_adjust_start_time (pAccumulatorSignal, newStartTime);
    signal_domain_adjust_start_time (pOtherSignal, newStartTime);
    signal_domain_adjust_end_time (pAccumulatorSignal, accumulatorEndTime, newEndTime);
    signal_domain_adjust_end_time (pOtherSignal, otherEndTime, newEndTime);
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