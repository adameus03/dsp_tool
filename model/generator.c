#include "generator.h"

#define __USE_MISC
#include <math.h>


real_signal_t generate_uniform_noise(generator_info_t info, double A, double t1, double d) {
    
}
real_signal_t generate_gaussian_noise(generator_info_t info, double A, double t1, double d) {

}
real_signal_t generate_sine(generator_info_t info, double A, double T, double t1, double d) {
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

}
real_signal_t generate_full_wave_rectified_sine(generator_info_t info, double A, double T, double t1, double d) {

}
real_signal_t generate_rectangular(generator_info_t info, double A, double T, double t1, double d, double kw) {

}
real_signal_t generate_symmetric_rectangular(generator_info_t info, double A, double T, double t1, double d, double kw) {

}
real_signal_t generate_triangle(generator_info_t info, double A, double T, double t1, double d, double kw) {

}
real_signal_t generate_heaviside(generator_info_t info, double A, double t1, double d, double ts) {

}
real_signal_t generate_kronecker_delta(generator_info_t info, double A, double ns, double n1, double f) {

}
real_signal_t generate_impulse_noise(generator_info_t info, double A, double t1, double d, double f, double p) {
    
}