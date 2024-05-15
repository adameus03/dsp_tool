#include "signal.h"
#include <stdlib.h>

void real_signal_free_values(real_signal_t* pSignal) {
    free(pSignal->pValues);
}
void complex_signal_free_values(complex_signal_t* pSignal) {
    free(pSignal->pValues);
}

void real_signal_alloc_values(real_signal_t* pSignal) {
    pSignal->pValues = malloc(pSignal->info.num_samples * sizeof(double));
}
void complex_signal_alloc_values(complex_signal_t* pSignal) {
    pSignal->pValues = malloc(pSignal->info.num_samples * sizeof(complex_number_t));
}

void real_signal_timeshift(real_signal_t* pSignal, double timeshiftValue) {
    pSignal->info.start_time += timeshiftValue;
}