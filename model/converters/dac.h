#include <stdint.h>
#include "../signal.h"

typedef enum {
    DAC_RECONSTRUCTION_ZERO_ORDER_EXTRAPOLATION,
    DAC_RECONSTRUCTION_SINC
} dac_reconstruction_type_t;

typedef void* dac_reconstruction_config_ptr_t;

typedef struct {
    uint64_t symmetric_num_neighbours;
} sinc_reconstruction_config_t;

typedef struct {
    double output_sampling_freq;
} pseudo_dac_caps_t;

typedef struct {
    dac_reconstruction_type_t reconstruction_type;
    dac_reconstruction_config_ptr_t pReconstruction_config;
} dac_config_t;

//void dac_convert(dac_config_t* pConfig, real_signal_t* pSignal);

double dac_reconstruct_single_real_signal_sample(dac_config_t* pConfig, real_signal_t* pSignal, double t);

real_signal_t dac_reconstruct_real_signal(dac_config_t* pConfig, pseudo_dac_caps_t* pCaps, real_signal_t* pSignal);

