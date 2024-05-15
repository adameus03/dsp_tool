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
    fprintf(stderr, "Error: Not implemented."); exit(EXIT_FAILURE);
}

double signal_radar_object_cdist(real_signal_t* pSignal) {
    fprintf(stderr, "Error: Not implemented."); exit(EXIT_FAILURE);
}
