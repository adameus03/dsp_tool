#include "similarity.h"
#include <math.h>

double signal_mean_squared_error(real_signal_t* quantized, real_signal_t* original) {
    if (quantized->info.num_samples != original->info.num_samples) return NAN;
    double counter = 0.0;
    for (int i = 0; i < quantized->info.num_samples; i++) {
        double difference = quantized->pValues[i] - original->pValues[i];
        counter += difference * difference;
    }
    return counter / (double) quantized->info.num_samples;
}

double signal_max_difference(real_signal_t* quantized, real_signal_t* original) {
    if (quantized->info.num_samples != original->info.num_samples) return NAN;
    double peak_difference = -INFINITY;
    for (int i = 0; i < quantized->info.num_samples; i++) {
        double difference = fabs(quantized->pValues[i] - original->pValues[i]);
        if (difference > peak_difference) {
            peak_difference = difference;
        }
    }
    return peak_difference;
}

double peak_signal_to_noise(real_signal_t* quantized, real_signal_t* original) {
    if (quantized->info.num_samples != original->info.num_samples) return NAN;
    double peak_original_signal = -INFINITY;
    for (int i = 0; i < original->info.num_samples; i++) {
        if (original->pValues[i] > peak_original_signal) {
            peak_original_signal = original->pValues[i];
        }
    }
    return 10.0 * log10(peak_original_signal / signal_mean_squared_error(quantized, original));
}

double signal_to_noise(real_signal_t* quantized, real_signal_t* original) {
    if (quantized->info.num_samples != original->info.num_samples) return NAN;
    double sum_of_original_squares = 0.0;
    double sum_of_difference_squares = 0.0;
    for (int i = 0; i < quantized->info.num_samples; i++) {
        sum_of_original_squares += original->pValues[i] * i[original->pValues]; // not sure if it should be original or quantized
        double difference = quantized->pValues[i] - original->pValues[i];
        sum_of_difference_squares += difference * difference;
    }

    return 10.0 * log10(sum_of_original_squares / sum_of_difference_squares);
}
