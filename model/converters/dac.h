#include <stdint.h>
#include "../signal.h"

typedef enum {
    DAC_DECONSTRUCTION_ZERO_ORDER_EXTRAPOLATION,
    DAC_RECONSTRUCTION_SINC
} dac_reconstruction_type_t;

typedef void* dac_reconstruction_config_ptr_t;

typedef struct {
    uint64_t symmetric_num_neighbours;
} sinc_reconstruction_config_t;

typedef struct {
    dac_reconstruction_type_t reconstruction_type;
    dac_reconstruction_config_ptr_t pReconstruction_config;
} dac_config_t;

void dac_convert(dac_config_t* pConfig, real_signal_t* pSignal);