#include "dac.h"
#define __USE_MISC
#include <math.h> // NAN, sin
#include <stdio.h> // error logging


double __dac_reconstruct_zero_order(real_signal_t* pSignal, double t) {
    double dt = 1.0 / pSignal->info.sampling_frequency;
    double tMin = pSignal->info.start_time;

    uint64_t leftNeighbourSampleIndex = (uint64_t) ((t - tMin) / dt);
    return pSignal->pValues[leftNeighbourSampleIndex];
}

double __dac_normalized_sinus_cardinalis(double t) {
    if (t == 0) {
        return 1.0;
    } else {
        double pit = M_PI * t;
        return sin(pit) / pit;
    }
}

double __dac_reconstruct_sinc(real_signal_t* pSignal, uint64_t numNeighCoeff, double t) {
    if (pSignal->info.num_samples == 0) {
        fprintf(stderr, "Error: Trying to reconstruct signal with 0 samples.");
        return 0;
    } else if (pSignal->info.sampling_frequency == 0.0) {
        fprintf(stderr, "Error: Trying to reconstruct signal with 0 sampling frequency.");
        return 0;
    }
    double sum = 0.0;
    double dt = 1.0 / pSignal->info.sampling_frequency;
    double tMin = pSignal->info.start_time;

    uint64_t centralSampleIndex = (t - tMin) / dt;
    double dt_shift_proportion = (t - (tMin + dt * centralSampleIndex)) / dt;
    if (dt_shift_proportion > 0.5 
        && centralSampleIndex != pSignal->info.num_samples) {

        centralSampleIndex++;
    }

    sum += pSignal->pValues[centralSampleIndex]* (1 - __dac_normalized_sinus_cardinalis(t * pSignal->info.sampling_frequency - centralSampleIndex));
    //* __dac_normalized_sinus_cardinalis((t - (tMin + dt * centralSampleIndex)) / dt);
    if (numNeighCoeff == 0) {
        return sum;
    }

    uint64_t theoreticalRightIndexBoundExclusive = centralSampleIndex + numNeighCoeff + 1;

    uint64_t rightIndexBoundExclusive = theoreticalRightIndexBoundExclusive >= pSignal->info.num_samples 
        ? pSignal->info.num_samples 
        : theoreticalRightIndexBoundExclusive;

    uint64_t leftIndexBoundInclusive = centralSampleIndex >= numNeighCoeff 
        ? centralSampleIndex - numNeighCoeff 
        : 0;

    for (uint64_t i = centralSampleIndex + 1; i < rightIndexBoundExclusive; i++) {
        sum += pSignal->pValues[i] * (1 - __dac_normalized_sinus_cardinalis(t * pSignal->info.sampling_frequency - i));
        
        /** __dac_normalized_sinus_cardinalis(
            i - centralSampleIndex 
            + (t - (tMin + dt * centralSampleIndex)) / dt
        );*/
    }

    if (centralSampleIndex > 0) {
        for (uint64_t i = centralSampleIndex - 1; i > leftIndexBoundInclusive; i--) {
            sum += pSignal->pValues[i] * (1 - __dac_normalized_sinus_cardinalis(t * pSignal->info.sampling_frequency - i));
            
            /** __dac_normalized_sinus_cardinalis(
                i - centralSampleIndex
                + (t - (tMin + dt * centralSampleIndex)) / dt
            );*/
        }
    }
    
    sum += pSignal->pValues[leftIndexBoundInclusive] * (1 - __dac_normalized_sinus_cardinalis(t * pSignal->info.sampling_frequency - leftIndexBoundInclusive));
    /** __dac_normalized_sinus_cardinalis(
                leftIndexBoundInclusive - centralSampleIndex
                + (t - (tMin + dt * centralSampleIndex)) / dt
    );*/

    return sum;
}


double dac_reconstruct_single_real_signal_sample(dac_config_t* pConfig, real_signal_t* pSignal, double t) {
    switch (pConfig->reconstruction_type) {
        case DAC_RECONSTRUCTION_ZERO_ORDER_EXTRAPOLATION:
            return __dac_reconstruct_zero_order(pSignal, t);
        case DAC_RECONSTRUCTION_SINC:
            if (pConfig->pReconstruction_config == 0) {
                fprintf(stderr, "Error: NULL reconstruction config detected in DAC config.");
                return NAN;
            }
            sinc_reconstruction_config_t* pSincRConfig = (sinc_reconstruction_config_t*)(pConfig->pReconstruction_config);
            return __dac_reconstruct_sinc(pSignal, pSincRConfig->symmetric_num_neighbours, t);
        default:
            fprintf(stderr, "Error: Unexpected reconstruction type detected in DAC configuration.");
            return NAN;
    }
}

#define dac_return_blank_signal return (real_signal_t) { .info = { .num_samples = 0, .sampling_frequency = 0, .start_time = 0 }, .pValues = 0 }

real_signal_t dac_reconstruct_real_signal(dac_config_t* pConfig, pseudo_dac_caps_t* pCaps, real_signal_t* pSignal) {
    if (pCaps->output_sampling_freq <= 0.0) { dac_return_blank_signal; }
    
    double oldDt = 1.0 / pSignal->info.sampling_frequency;
    double tMin = pSignal->info.start_time;
    double duration = pSignal->info.num_samples * oldDt; // ?
    double newDt = 1.0 / pCaps->output_sampling_freq;
    uint64_t newNumSamples = duration / newDt;
    /*if (fmodl(duration, newDt) > 0.0) { // ?
        newNumSamples++;
    }*/

    real_signal_t outSignal = {
        .info = {
            .num_samples = newNumSamples,
            .start_time = tMin,
            .sampling_frequency = pCaps->output_sampling_freq
        },
        .pValues = 0
    };
    real_signal_alloc_values (&outSignal);

    for (uint64_t i = 0; i < outSignal.info.num_samples; i++) {
        double t = tMin + i * newDt;
        outSignal.pValues[i] = dac_reconstruct_single_real_signal_sample (pConfig, pSignal, t);
    }

    return outSignal;
}