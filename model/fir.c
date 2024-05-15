#include "fir.h"

#include <stdio.h> //error logs
#include <stdlib.h> // exit

void fir_filter_real_signal_lowpass(real_signal_t* pInputSignal, real_signal_t* pOutputSignal, fir_lowpass_config_t* pConfig) {
    fprintf(stderr, "Error: Not implemented."); exit(EXIT_FAILURE);
}

void fir_filter_real_signal_highpass(real_signal_t* pInputSignal, real_signal_t* pOutputSignal, fir_highpass_config_t* pConfig) {
    fprintf(stderr, "Error: Not implemented."); exit(EXIT_FAILURE);
}

void fir_filter_real_signal_bandpass(real_signal_t* pInputSignal, real_signal_t* pOutputSignal, fir_bandpass_config_t* pConfig) {
    fprintf(stderr, "Error: Not implemented."); exit(EXIT_FAILURE);
}
