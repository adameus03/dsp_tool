#include "dac.h"
#include "math.h" // NAN, sin
#include "stdio.h" // error logging

double __dac_reconstruct_zero_order(real_signal_t* pSignal, double t) {
    double dt = 1.0 / pSignal->info.sampling_frequency;
    double tMin = pSignal->info.start_time;

    uint64_t leftNeighbourSampleIndex = (uint64_t) ((t - tMin) / dt);
    return pSignal->pValues[leftNeighbourSampleIndex];
}

double __dac_reconstruct_sinc(real_signal_t* pSignal, uint64_t numNeighCoeff, double t) {
    fprintf(stderr, "Error: sinc reconstruction is not implemented yet.");
    return NAN;
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
            sinc_reconstruction_config_t* pSincRConfig = pConfig->pReconstruction_config;
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