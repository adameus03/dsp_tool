#include "similarity.h"
#include <math.h>
#include <stdio.h> // for error logging
#include <assert.h>

#include "../aggregator.h"



/**
 * @returns 0 if check completed successfully, 1 if check failed
*/
uint8_t __check_signals(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal) {
    uint8_t status = 1U;
    if (pSignalOriginal->info.num_samples == 0) {
        fprintf(stderr, "Error: Original signal has 0 samples.");
    } else if (pSignalOriginal->info.sampling_frequency <= 0.0) {
        fprintf(stderr, "Error: Original signal has forbidden sampling frequency.");
    } else if (pSignalImitated->info.num_samples == 0) {
        fprintf(stderr, "Error: Imitated signal has 0 samples.");
    } else if (pSignalImitated->info.sampling_frequency <= 0.0) {
        fprintf(stderr, "Error: Imitated signal has forbidden sampling frequency.");
    } else if (pSignalImitated->info.start_time != pSignalImitated->info.start_time) {
        fprintf(stderr, "Error: Original and imitated signal should start at the same time.");
    } else {
        status = 0U;
    }
    return status;
}

double signal_mean_squared_error(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal) {
    if (__check_signals(pSignalImitated, pSignalOriginal)) {
        return NAN;
    }

    double t0 = pSignalOriginal->info.start_time;
    double original_dt = 1.0 / pSignalOriginal->info.sampling_frequency;
    double imitated_dt = 1.0 / pSignalImitated->info.sampling_frequency;
    
    double sum = 0.0;

    for (uint64_t i = 0; i < pSignalOriginal->info.num_samples; i++) {
        double* pOriginalValue = pSignalOriginal->pValues + i;
        double t = t0 + i * original_dt;
        uint64_t imitatedNearestLeftNeighbourSampleIndex = (uint64_t) ((t - t0) / imitated_dt);
        assert(imitatedNearestLeftNeighbourSampleIndex < pSignalImitated->info.num_samples);
        double* pImitatedValue = pSignalImitated->pValues + imitatedNearestLeftNeighbourSampleIndex;

        double diff = *pImitatedValue - *pOriginalValue;
        sum += diff * diff;
    }

    return sum / (double)pSignalOriginal->info.num_samples;
}

double signal_max_difference(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal) {
    if (__check_signals(pSignalImitated, pSignalOriginal)) {
        return NAN;
    }

    double t0 = pSignalOriginal->info.start_time;
    double original_dt = 1.0 / pSignalOriginal->info.sampling_frequency;
    double imitated_dt = 1.0 / pSignalImitated->info.sampling_frequency;
    
    double max_adiff = -__DBL_MAX__;

    for (uint64_t i = 0; i < pSignalOriginal->info.num_samples; i++) {
        double* pOriginalValue = pSignalOriginal->pValues + i;
        double t = t0 + i * original_dt;
        uint64_t imitatedNearestLeftNeighbourSampleIndex = (uint64_t) ((t - t0) / imitated_dt);
        assert(imitatedNearestLeftNeighbourSampleIndex < pSignalImitated->info.num_samples);
        double* pImitatedValue = pSignalImitated->pValues + imitatedNearestLeftNeighbourSampleIndex;

        double adiff = fabs(*pImitatedValue - *pOriginalValue);
        if (adiff > max_adiff) {
            max_adiff = adiff;
        }
    }

    return max_adiff;
}

double peak_signal_to_noise(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal) {
    double originalMaxAbsval= signal_max_abs_value(pSignalOriginal);
    double originalMaxSq = originalMaxAbsval * originalMaxAbsval;
    double mse = signal_mean_squared_error(pSignalImitated, pSignalOriginal);
    
    return 10.0 * log10(originalMaxSq / mse);
}

double signal_to_noise(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal, double* pOutEnob) {
    double signalPower = mean_signal_power(pSignalOriginal);
    double noisePower = signal_mean_squared_error(pSignalImitated, pSignalOriginal);

    double snr = 10.0 * log10(signalPower / noisePower);
    if (pOutEnob != NULL) {
        *pOutEnob = (snr - 1.76) / 6.02;
    }
    
    return snr;
}


