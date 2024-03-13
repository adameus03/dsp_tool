#include "combiner.h"
#include <stdio.h> //for logging
#include <math.h>

void add_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {
    if (pAccumulatorSignal->info.sampling_frequency != pOtherSignal->info.sampling_frequency) {
        fprintf(stdout, "Warning: Refusing to combine signals sampled differently");
        return;
    }
    double accumulatorDt = 1.0 / pAccumulatorSignal->info.sampling_frequency;
    double otherDt = 1.0 / pOtherSignal->info.sampling_frequency;
    //if (fmod(pAccumulatorSignal->info.start_time, accumulatorDt) != )
    
    fprintf(stderr, "During implementation\n");
    
    
    real_signal_t tempSignal = { 
        .info = {
            
        },
        .pValues = 0
    };
    //real_signal_alloc_values ()
}
void substract_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {

}
void multiply_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {

}
void divide_signal(real_signal_t* pAccumulatorSignal, real_signal_t* pOtherSignal) {
    
}