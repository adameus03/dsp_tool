#include "signal.h"

typedef struct {
    double sampling_frequency;
} generator_info_t;

real_signal_t generate_uniform_noise(generator_info_t info, double A, double t1, double d);
real_signal_t generate_gaussian_noise(generator_info_t info, double A, double t1, double d);
real_signal_t generate_sine(generator_info_t info, double A, double T, double t1, double d);
real_signal_t generate_cosine(generator_info_t info, double A, double T, double t1, double d);
real_signal_t generate_half_wave_rectified_sine(generator_info_t info, double A, double T, double t1, double d);
real_signal_t generate_full_wave_rectified_sine(generator_info_t info, double A, double T, double t1, double d);
real_signal_t generate_rectangular(generator_info_t info, double A, double T, double t1, double d, double kw);
real_signal_t generate_symmetric_rectangular(generator_info_t info, double A, double T, double t1, double d, double kw);
real_signal_t generate_triangle(generator_info_t info, double A, double T, double t1, double d, double kw);
real_signal_t generate_heaviside(generator_info_t info, double A, double t1, double d, double ts);
real_signal_t generate_kronecker_delta(generator_info_t info, double A, uint64_t ns, uint64_t n1, uint64_t l);
real_signal_t generate_impulse_noise(generator_info_t info, double A, double t1, double d, double p);
