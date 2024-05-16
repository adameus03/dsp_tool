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

void fir_common_config_print(fir_common_config_t* pCommonConfig) {
    fir_windowing_window_type_t wtype;
    if ((pCommonConfig->filterType == FIR_FILTER_TYPE_LOWPASS) || (pCommonConfig->filterType == FIR_FILTER_TYPE_HIGHPASS)) {
        if (pCommonConfig->filterType == FIR_FILTER_TYPE_LOWPASS) {
            printf("Filter type: FIR_FILTER_TYPE_LOWPASS\n");
        } else {
            printf("Filter type: FIR_FILTER_TYPE_HIGHPASS\n");
        }
        printf("Cutoff frequency: %f\n", pCommonConfig->oneSidedConfig.cutoff_frequency);
        printf("Num FIR coeffs: %lu\n", pCommonConfig->oneSidedConfig.windowing.num_fir_coeffs);
        wtype = pCommonConfig->oneSidedConfig.windowing.window_type;
    } else {
        printf("Filter type: FIR_FILTER_TYPE_BANDPASS\n");
        printf("Left cutoff frequency: %f\n", pCommonConfig->doubleSidedConfig.left_cutoff_frequency);
        printf("Right cutoff frequency: %f\n", pCommonConfig->doubleSidedConfig.right_cutoff_frequency);
        printf("Num FIR coeffs: %lu\n", pCommonConfig->doubleSidedConfig.windowing.num_fir_coeffs);
        wtype = pCommonConfig->doubleSidedConfig.windowing.window_type;
    }

    switch (wtype) {
        case FIR_WINDOWING_WINDOW_TYPE_RECTANGULAR:
            printf("Windowing window type: RECTANGULAR\n");
            break;
        case FIR_WINDOWING_WINDOW_TYPE_HAMMING:
            printf("Windowing window type: HAMMING\n");
            break;
        case FIR_WINDOWING_WINDOW_TYPE_HANNING:
            printf("Windowing window type: HANNING\n");
            break;
        case FIR_WINDOWING_WINDOW_TYPE_BLACKMAN:
            printf("Windowing window type: BLACKMAN\n");
            break;
        default:
            printf("Windowing window type: [UNKNOWN]\n");
            break;
    }
}