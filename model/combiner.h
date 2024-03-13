/**
 * Functions for combining real and complex signals
*/

#include "signal.h"

void add_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal);
void substract_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal);
void multiply_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal);
void divide_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal);
