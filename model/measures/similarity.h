#ifndef SIMILARITY_H_INCLUDED
#define SIMILARITY_H_INCLUDED

#include "../signal.h"

double signal_mean_squared_error(real_signal_t* quantized, real_signal_t* original);
double signal_to_noise(real_signal_t* quantized, real_signal_t* original);
double peak_signal_to_noise(real_signal_t* quantized, real_signal_t* original);
double signal_max_difference(real_signal_t* quantized, real_signal_t* original);

#endif // SIMILARITY_H_INCLUDED
