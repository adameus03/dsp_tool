#include "generator.h"

#define __USE_MISC
#include <math.h>
#include <stdio.h> // for logging
#include <stdlib.h> // for rand

#define return_blank_signal return (real_signal_t) { .info = { .num_samples = 0, .sampling_frequency = 0, .start_time = 0 }, .pValues = 0 }
/**
 * Uses Marsaglia polar method and rand() to generate a standard-normally distributed pseudo-random floating-point value
 * 
*/
double __standard_gaussian_rand() {
    double x; double y; 
    double s;
    do {
        x = -1.0 + 2 * ((double)rand()) / (double)RAND_MAX;
        y = -1.0 + 2 * ((double)rand()) / (double)RAND_MAX;
        s = x*x + y*y;
    } while (s >= 1.0);
    return x * sqrt(-2.0 * log(s) / s);
}

real_signal_t generate_uniform_noise(generator_info_t info, double A, double t1, double d) {
    if (info.sampling_frequency == 0.0) { return_blank_signal; }
    
    real_signal_t signal = {
        .info = {
            .num_samples = d * info.sampling_frequency,
            .start_time = t1,
            .sampling_frequency = info.sampling_frequency
        },
        .pValues = 0
    };
    real_signal_alloc_values(&signal);

    for (uint64_t i = 0; i < signal.info.num_samples; i++) {
        signal.pValues[i] = A * (-1 + 2 * ((double)rand()) / (double)RAND_MAX);
    }
    return signal;
}

real_signal_t generate_gaussian_noise(generator_info_t info, double A, double t1, double d) {
    if (info.sampling_frequency == 0.0) { return_blank_signal; }
    
    real_signal_t signal = {
        .info = {
            .num_samples = d * info.sampling_frequency,
            .start_time = t1,
            .sampling_frequency = info.sampling_frequency
        },
        .pValues = 0
    };
    real_signal_alloc_values(&signal);

    for (uint64_t i = 0; i < signal.info.num_samples; i++) {
        signal.pValues[i] = A * __standard_gaussian_rand();
    }
    return signal;
}

real_signal_t generate_sine(generator_info_t info, double A, double T, double t1, double d) {
    if (info.sampling_frequency == 0.0) { return_blank_signal; }
    
    real_signal_t signal = {
        .info = {
            .num_samples = d * info.sampling_frequency,
            .start_time = t1,
            .sampling_frequency = info.sampling_frequency
        },
        .pValues = 0
    };
    real_signal_alloc_values(&signal);

    double omega = 2 * M_PI / T;
    double dt = 1.0 / info.sampling_frequency;
    for (uint64_t i = 0; i < signal.info.num_samples; i++) {
        signal.pValues[i] = A * sin(omega * ( i * dt )); // t1 + i * dt - t1 = i * dt
    }
    return signal;
}

real_signal_t generate_half_wave_rectified_sine(generator_info_t info, double A, double T, double t1, double d) {
    if (info.sampling_frequency == 0.0) { return_blank_signal; }

    real_signal_t signal = generate_sine (info,A, T, t1, d);
    for (uint64_t i = 0; i < signal.info.num_samples; i++) {
        double* pValue = signal.pValues + i;
        if (*pValue < 0.0) { *pValue = 0.0; }
    }
    return signal;
}
real_signal_t generate_full_wave_rectified_sine(generator_info_t info, double A, double T, double t1, double d) {
    if (info.sampling_frequency == 0.0) { return_blank_signal; }

    real_signal_t signal = generate_sine (info,A, T, t1, d);
    for (uint64_t i = 0; i < signal.info.num_samples; i++) {
        double* pValue = signal.pValues + i;
        if (*pValue < 0.0) { *pValue = -*pValue; }
    }
    return signal;
}
real_signal_t generate_rectangular(generator_info_t info, double A, double T, double t1, double d, double kw) {
    if (T == 0.0) { return_blank_signal; }
    if (info.sampling_frequency == 0.0) { return_blank_signal; }

    real_signal_t signal = {
        .info = {
            .num_samples = d * info.sampling_frequency,
            .start_time = t1,
            .sampling_frequency = info.sampling_frequency
        },
        .pValues = 0
    };
    real_signal_alloc_values(&signal);

    uint64_t num_samples_per_period = info.sampling_frequency * T;
    if (num_samples_per_period == 0) { 
        fprintf(stderr, "Error: num_samples_per_period evaluated to 0");
        return_blank_signal;
    }
    uint64_t num_high_samples_per_period = num_samples_per_period * kw;
    uint64_t num_full_periods = signal.info.num_samples / num_samples_per_period;
    uint64_t num_remainder_samples = signal.info.num_samples % num_samples_per_period;
    
    double* pValue = signal.pValues;
    for (uint64_t i = 0; i < num_full_periods; i++) {
        for (uint64_t j = 0; j < num_high_samples_per_period; j++) {
            *(pValue++) = A;
        }
        for (uint64_t j = num_high_samples_per_period; j < num_samples_per_period; j++) {
            *(pValue++) = 0;
        }
    }
    if (num_remainder_samples < num_high_samples_per_period) {
        for (uint64_t j = 0; j < num_remainder_samples; j++) {
            *(pValue++) = A;
        }
    } else {
        for (uint64_t j = 0; j < num_high_samples_per_period; j++) {
            *(pValue++) = A;
        }
        for (uint64_t j = num_high_samples_per_period; j < num_remainder_samples; j++) {
            *(pValue++) = 0;
        }
    }
    return signal;
}
real_signal_t generate_symmetric_rectangular(generator_info_t info, double A, double T, double t1, double d, double kw) {
    if (T == 0.0) { return_blank_signal; }
    if (info.sampling_frequency == 0.0) { return_blank_signal; }
    
    real_signal_t signal = {
        .info = {
            .num_samples = d * info.sampling_frequency,
            .start_time = t1,
            .sampling_frequency = info.sampling_frequency
        },
        .pValues = 0
    };
    real_signal_alloc_values(&signal);

    uint64_t num_samples_per_period = info.sampling_frequency * T;
    if (num_samples_per_period == 0) { 
        fprintf(stderr, "Error: num_samples_per_period evaluated to 0");
        return_blank_signal;
    }
    uint64_t num_high_samples_per_period = num_samples_per_period * kw;
    uint64_t num_full_periods = signal.info.num_samples / num_samples_per_period;
    uint64_t num_remainder_samples = signal.info.num_samples % num_samples_per_period;
    
    double* pValue = signal.pValues;
    for (uint64_t i = 0; i < num_full_periods; i++) {
        for (uint64_t j = 0; j < num_high_samples_per_period; j++) {
            *(pValue++) = A;
        }
        for (uint64_t j = num_high_samples_per_period; j < num_samples_per_period; j++) {
            *(pValue++) = -A;
        }
    }
    if (num_remainder_samples < num_high_samples_per_period) {
        for (uint64_t j = 0; j < num_remainder_samples; j++) {
            *(pValue++) = A;
        }
    } else {
        for (uint64_t j = 0; j < num_high_samples_per_period; j++) {
            *(pValue++) = A;
        }
        for (uint64_t j = num_high_samples_per_period; j < num_remainder_samples; j++) {
            *(pValue++) = -A;
        }
    }
    return signal;
}
real_signal_t generate_triangle(generator_info_t info, double A, double T, double t1, double d, double kw) {
    if (T == 0.0) { return_blank_signal; }
    if (info.sampling_frequency == 0.0) { return_blank_signal; }
    
    real_signal_t signal = {
        .info = {
            .num_samples = d * info.sampling_frequency,
            .start_time = t1,
            .sampling_frequency = info.sampling_frequency
        },
        .pValues = 0
    };
    real_signal_alloc_values(&signal);

    uint64_t num_samples_per_period = info.sampling_frequency * T;
    if (num_samples_per_period == 0) { 
        fprintf(stderr, "Error: num_samples_per_period evaluated to 0");
        return_blank_signal;
    }
    uint64_t num_high_samples_per_period = num_samples_per_period * kw;
    uint64_t num_full_periods = signal.info.num_samples / num_samples_per_period;
    uint64_t num_remainder_samples = signal.info.num_samples % num_samples_per_period;
    
    double* pValue = signal.pValues;
    for (uint64_t i = 0; i < num_full_periods; i++) {
        for (uint64_t j = 0; j < num_high_samples_per_period; j++) {
            *(pValue++) = ((double)j) / ((double)num_high_samples_per_period) * A;
        }
        for (uint64_t j = num_high_samples_per_period; j < num_samples_per_period; j++) {
            *(pValue++) = A - (double)(j - num_high_samples_per_period) / ((double)(num_samples_per_period - num_high_samples_per_period)) * A;
        }
    }
    if (num_remainder_samples < num_high_samples_per_period) {
        for (uint64_t j = 0; j < num_remainder_samples; j++) {
            *(pValue++) = ((double)j) / ((double)num_high_samples_per_period) * A;
        }
    } else {
        for (uint64_t j = 0; j < num_high_samples_per_period; j++) {
            *(pValue++) = ((double)j) / ((double)num_high_samples_per_period) * A;
        }
        for (uint64_t j = num_high_samples_per_period; j < num_remainder_samples; j++) {
            *(pValue++) = A - (double)(j - num_high_samples_per_period) / ((double)(num_samples_per_period - num_high_samples_per_period)) * A;
        }
    }
    return signal;
}
real_signal_t generate_heaviside(generator_info_t info, double A, double t1, double d, double ts) {
    if (info.sampling_frequency == 0.0) { return_blank_signal; }
    
    real_signal_t signal = {
        .info = {
            .num_samples = d * info.sampling_frequency,
            .start_time = t1,
            .sampling_frequency = info.sampling_frequency
        },
        .pValues = 0
    };
    real_signal_alloc_values(&signal);

    uint64_t stepSampleIndex = (ts - t1) * info.sampling_frequency;  /**  @verify **/

    if (stepSampleIndex >= signal.info.num_samples) {
        fprintf(stdout, "Note: Step sample would be after the last sample\n");
        for (uint64_t i = 0;i < signal.info.num_samples; i++) {
            signal.pValues[i] = 0;
        }
        return signal;
    }

    for (uint64_t i = 0; i < stepSampleIndex; i++) {
        signal.pValues[i] = 0;
    }
    signal.pValues[stepSampleIndex] = 0.5 * A;
    for (uint64_t i = stepSampleIndex + 1; i < signal.info.num_samples; i++) {
        signal.pValues[i] = A;
    }
    return signal;
}
real_signal_t generate_kronecker_delta(generator_info_t info, double A, uint64_t ns, uint64_t n1, uint64_t l) {
    if (l == 0) { return_blank_signal; }
    if (info.sampling_frequency == 0.0) { return_blank_signal; }

    real_signal_t signal = {
        .info = {
            .num_samples = l,
            .start_time = n1 / info.sampling_frequency,
            .sampling_frequency = info.sampling_frequency
        },
        .pValues = 0
    };
    real_signal_alloc_values(&signal);
    double* pValue = signal.pValues;
    for (uint64_t i = n1; i < ns; i++) {
        *(pValue++) = 0.0;
    }
    *(pValue++) = A;
    for (uint64_t i = ns + 1; i < l - 1; i++) {
        *(pValue++) = 0.0;
    }
    *pValue = 0.0;
    return signal;
}
real_signal_t generate_impulse_noise(generator_info_t info, double A, double t1, double d, double p) {
    if (info.sampling_frequency == 0.0) { return_blank_signal; }
    
    real_signal_t signal = {
        .info = {
            .num_samples = d * info.sampling_frequency,
            .start_time = t1,
            .sampling_frequency = info.sampling_frequency
        },
        .pValues = 0
    };
    real_signal_alloc_values(&signal);
    
    for (uint64_t i = 0; i < signal.info.num_samples; i++) {
        double* pValue = signal.pValues + i;
        double r = ((double)rand()) / (double)RAND_MAX;
        if (r < p) {
            *pValue = A;
        } else {
            *pValue = 0;
        }
    }
    return signal;
}