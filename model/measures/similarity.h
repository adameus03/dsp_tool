#ifndef SIMILARITY_H_INCLUDED
#define SIMILARITY_H_INCLUDED

#include "../signal.h"

double signal_mean_squared_error(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal);
double signal_to_noise(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal, double* pOutEnob);
double peak_signal_to_noise(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal);
double signal_max_difference(real_signal_t* pSignalImitated, real_signal_t* pSignalOriginal);

#endif // SIMILARITY_H_INCLUDED
