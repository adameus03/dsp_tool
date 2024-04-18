#include "similarity.h"
#include <math.h>

double signal_mean_squared_error(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal) {
    if (pSignalImitated->info.num_samples != pSignalOriginal->info.num_samples) return NAN;
    double counter = 0.0;
    for (int i = 0; i < pSignalImitated->info.num_samples; i++) {
        double difference = pSignalImitated->pValues[i] - pSignalOriginal->pValues[i];
        counter += difference * difference;
    }
    return counter / (double) pSignalImitated->info.num_samples;
}

double signal_max_difference(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal) {
    if (pSignalImitated->info.num_samples != pSignalOriginal->info.num_samples) return NAN;
    double peak_difference = -INFINITY;
    for (int i = 0; i < pSignalImitated->info.num_samples; i++) {
        double difference = fabs(pSignalImitated->pValues[i] - pSignalOriginal->pValues[i]);
        if (difference > peak_difference) {
            peak_difference = difference;
        }
    }
    return peak_difference;
}

double peak_signal_to_noise(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal) {
    if (pSignalImitated->info.num_samples != pSignalOriginal->info.num_samples) return NAN;
    double peak_pSignalOriginal_signal = -INFINITY;
    for (int i = 0; i < pSignalOriginal->info.num_samples; i++) {
        if (pSignalOriginal->pValues[i] > peak_pSignalOriginal_signal) {
            peak_pSignalOriginal_signal = pSignalOriginal->pValues[i];
        }
    }
    return 10.0 * log10(peak_pSignalOriginal_signal * peak_pSignalOriginal_signal / signal_mean_squared_error(pSignalImitated, pSignalOriginal));
}

double signal_to_noise(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal) {
    if (pSignalImitated->info.num_samples != pSignalOriginal->info.num_samples) return NAN;
    double sum_of_pSignalOriginal_squares = 0.0;
    double sum_of_difference_squares = 0.0;
    for (int i = 0; i < pSignalImitated->info.num_samples; i++) {
        sum_of_pSignalOriginal_squares += pSignalOriginal->pValues[i] * i[pSignalOriginal->pValues]; // not sure if it should be pSignalOriginal or pSignalImitated
        double difference = pSignalImitated->pValues[i] - pSignalOriginal->pValues[i];
        sum_of_difference_squares += difference * difference;
    }

    return 10.0 * log10(sum_of_pSignalOriginal_squares / sum_of_difference_squares);
}
