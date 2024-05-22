#include "aggregator.h"
#include <math.h>
#include <stdio.h> //error logs
#include <stdlib.h> // exit

double mean_signal_value(real_signal_t* pSignal) {
    double sum = 0;
    for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
        sum += pSignal->pValues[i];
    }
    return sum / (double) pSignal->info.num_samples;
}
double mean_signal_absolute_value(real_signal_t* pSignal) {
    double sum = 0;
    for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
        sum += fabs(pSignal->pValues[i]);
    }
    return sum / (double) pSignal->info.num_samples;
}
double mean_signal_power(real_signal_t* pSignal) {
    double sum = 0;
    for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
        double* pValue = pSignal->pValues + i;
        sum += (*pValue) * (*pValue);
    }
    return sum / (double) pSignal->info.num_samples;
}
double signal_variance(real_signal_t* pSignal) {
    double meanSignalValue = mean_signal_value(pSignal);
    double sum = 0;
    for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
        double* pValue = pSignal->pValues + i;
        double base = (*pValue) - meanSignalValue;
        sum += base * base;
    }
    return sum / (double) pSignal->info.num_samples;
}
double signal_RMS(real_signal_t* pSignal) {
    return sqrt (mean_signal_power(pSignal));
}

double signal_max_abs_value(real_signal_t* pSignal) {
    double maxAValue = -__DBL_MAX__;
    for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
        double* pValue = pSignal->pValues + i;
        double absValue = fabs(*pValue);
        if (absValue > maxAValue) {
            maxAValue = absValue;
        }
    }

    return maxAValue;
}

double signal_rightmost_argmax(real_signal_t* pSignal) {
    double maxValue = -__DBL_MAX__;
    double epsilon = 1e-2;
    uint64_t maxIndex = 0;
    for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
        double* pValue = pSignal->pValues + i;
        double diff = *pValue - maxValue;
        if (diff > epsilon) {
            maxValue = *pValue;
            maxIndex = i;
        } else if (diff > 0) {
            if (maxIndex < i) {
                maxValue = *pValue;
                maxIndex = i;
            }
        }
    }

    double dt = 1.0 / pSignal->info.sampling_frequency;
    return pSignal->info.start_time + maxIndex * dt;
}

//#define LIGHT_SPEED 299792458.0
#define AGGREGATOR_LIGHT_SPEED 2.0
double signal_radar_object_cdist(real_signal_t* pSignal) {
    double argmax = signal_rightmost_argmax(pSignal);
    double middleTime = pSignal->info.start_time + 0.5 * ((double)pSignal->info.num_samples) / pSignal->info.sampling_frequency;
    double argdiff = argmax - middleTime;
    if (argdiff < 0) {
        fprintf(stdout, "Warning: negative argdiff detected.\n");
    }
    return 0.5 * (argdiff * AGGREGATOR_LIGHT_SPEED); // 0.5 because of the round trip
}
