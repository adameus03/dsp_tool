#include "signal.h"

double mean_signal_value(real_signal_t* pSignal);
double mean_signal_absolute_value(real_signal_t* pSignal);
double mean_signal_power(real_signal_t* pSignal);
double signal_variance(real_signal_t* pSignal);
double signal_RMS(real_signal_t* pSignal);
double signal_max_abs_value(real_signal_t* pSignal);

double signal_rightmost_argmax(real_signal_t* pSignal);
double signal_radar_object_cdist(real_signal_t* pSignal);
