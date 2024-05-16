/**
 * Functions for filtering real and complex signals using FIR filters.
*/

#include "signal.h"

typedef enum {
    FIR_WINDOWING_WINDOW_TYPE_RECTANGULAR,
    FIR_WINDOWING_WINDOW_TYPE_HAMMING,
    FIR_WINDOWING_WINDOW_TYPE_HANNING,
    FIR_WINDOWING_WINDOW_TYPE_BLACKMAN
} fir_windowing_window_type_t;

typedef struct {
    double cutoff_frequency;
    struct {
        fir_windowing_window_type_t window_type;
        uint64_t num_fir_coeffs;
    } windowing;
} fir_one_sided_real_filter_config_t;

typedef struct {
    double left_cutoff_frequency;
    double right_cutoff_frequency;
    struct {
        fir_windowing_window_type_t window_type;
        uint64_t num_fir_coeffs;
    } windowing;
} fir_double_sided_real_filter_config_t;

typedef enum {
    FIR_FILTER_TYPE_LOWPASS,
    FIR_FILTER_TYPE_HIGHPASS,
    FIR_FILTER_TYPE_BANDPASS
} fir_filter_type;

typedef struct {
    fir_filter_type filterType;
    union {
        fir_one_sided_real_filter_config_t oneSidedConfig;
        fir_double_sided_real_filter_config_t doubleSidedConfig;
    };
} fir_common_config_t;

typedef fir_one_sided_real_filter_config_t fir_lowpass_config_t;
typedef fir_one_sided_real_filter_config_t fir_highpass_config_t;
typedef fir_double_sided_real_filter_config_t fir_bandpass_config_t;

void fir_filter_real_signal_lowpass(real_signal_t* pInputSignal, real_signal_t* pOutputSignal, fir_lowpass_config_t* pConfig);
void fir_filter_real_signal_highpass(real_signal_t* pInputSignal, real_signal_t* pOutputSignal, fir_highpass_config_t* pConfig);
void fir_filter_real_signal_bandpass(real_signal_t* pInputSignal, real_signal_t* pOutputSignal, fir_bandpass_config_t* pConfig);

void fir_common_config_print(fir_common_config_t* pCommonConfig);