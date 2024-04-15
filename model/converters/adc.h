#include <stdint.h>
#include "../signal.h"
typedef struct {
    uint64_t sampling_frequency;
    double quantization_threshold;
} adc_caps_t;

void adc_sample_real_signal(adc_caps_t* pCaps, real_signal_t* pSignal);
void adc_quantize_real_signal(adc_caps_t* pCaps, real_signal_t* pSignal);
void adc_convert(adc_caps_t* pCaps, real_signal_t* pSignal);


