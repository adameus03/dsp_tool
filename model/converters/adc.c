#include "adc.h"

/*void adc_sample_real_signal(adc_caps_t* pCaps, real_signal_t* pSignal) {
    // real_signal_t new_signal = {
    //     .info = {
            
    //     }
    // };
}*/

void adc_quantize_real_signal(adc_caps_t* pCaps, real_signal_t* pSignal) {
    if (pSignal->info.num_samples == 0) {
        return;
    }
    double signal_min_val = pSignal->pValues[0];
    
    for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
        double* pValue = pSignal->pValues + i;
        if (*pValue < signal_min_val) {
            signal_min_val = *pValue;
        }
    }
    
    for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
        double* pValue = pSignal->pValues + i;
        uint64_t quantizationLevel = (*pValue - signal_min_val) / pCaps->quantization_threshold;
        *pValue = signal_min_val + quantizationLevel * pCaps->quantization_threshold;   
    }
}

/*void adc_convert(adc_caps_t* pCaps, real_signal_t* pSignal) {
    
}*/
