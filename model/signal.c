#include "signal.h"
#include <stdlib.h>
#include <memory.h>

void signal_free_values(signal_t* pSignal) {
    if (pSignal->treat_as_complex) {
        complex_signal_free_values(&pSignal->complex_signal);
    } else {
        real_signal_free_values(&pSignal->real_signal);
    }
}

void signal_alloc_values(signal_t* pSignal) {
    if (pSignal->treat_as_complex) {
        complex_signal_alloc_values(&pSignal->complex_signal);
    } else {
        real_signal_alloc_values(&pSignal->real_signal);
    }
}

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
    pSignal->pValues = malloc(pSignal->info.num_samples * sizeof(double complex));
}


/**
 * @verify
*/
void signal_domain_adjust_start_time(signal_t* pSignal, double newStartTime) {
    
    double t1Diff = 0;
    if (pSignal->treat_as_complex) {
        pSignal->complex_signal.info.start_time = newStartTime;
    } else {
        pSignal->real_signal.info.start_time = newStartTime;
    }
    if (t1Diff > 0) {
        uint64_t num_samples_to_prepend = 0;
        if (pSignal->treat_as_complex) {
            num_samples_to_prepend = (uint64_t)(t1Diff * pSignal->complex_signal.info.sampling_frequency);
        } else {
            num_samples_to_prepend = (uint64_t)(t1Diff * pSignal->real_signal.info.sampling_frequency);
        }
        signal_t tempSignal = { 
            treat_as_complex: pSignal->treat_as_complex,
        };
        if (pSignal->treat_as_complex) {
            tempSignal.complex_signal = (complex_signal_t) {
                .info = {
                    .num_samples = pSignal->complex_signal.info.num_samples + num_samples_to_prepend,
                    .sampling_frequency = pSignal->complex_signal.info.sampling_frequency,
                    .start_time = newStartTime
                },
                .pValues = 0
            };
        } else {
            tempSignal.real_signal = (real_signal_t) {
                .info = {
                    .num_samples = pSignal->real_signal.info.num_samples + num_samples_to_prepend,
                    .sampling_frequency = pSignal->real_signal.info.sampling_frequency,
                    .start_time = newStartTime
                },
                .pValues = 0
            };
        }
        signal_alloc_values (&tempSignal);

        if (pSignal->treat_as_complex) {
            for (uint64_t i = 0; i < num_samples_to_prepend; i++) {
                tempSignal.complex_signal.pValues[i] = 0;
            }
            for (uint64_t i = 0; i < pSignal->complex_signal.info.num_samples; i++) {
                tempSignal.complex_signal.pValues[i + num_samples_to_prepend] = pSignal->complex_signal.pValues[i];
            }
        } else {
            for (uint64_t i = 0; i < num_samples_to_prepend; i++) {
                tempSignal.real_signal.pValues[i] = 0;
            }
            for (uint64_t i = 0; i < pSignal->real_signal.info.num_samples; i++) {
                tempSignal.real_signal.pValues[i + num_samples_to_prepend] = pSignal->real_signal.pValues[i];
            }
        }

        signal_free_values (pSignal);
        

        if (pSignal->treat_as_complex) {
            pSignal->complex_signal.pValues = tempSignal.complex_signal.pValues;
            pSignal->complex_signal.info.num_samples = tempSignal.complex_signal.info.num_samples;
            pSignal->complex_signal.info.start_time = tempSignal.complex_signal.info.start_time;
        } else {
            pSignal->real_signal.pValues = tempSignal.real_signal.pValues;
            pSignal->real_signal.info.num_samples = tempSignal.real_signal.info.num_samples;
            pSignal->real_signal.info.start_time = tempSignal.real_signal.info.start_time;
        }

    } else if (t1Diff < 0) {
        if (pSignal->treat_as_complex) {
            uint64_t num_samples_to_delete = (uint64_t)((-t1Diff) * pSignal->complex_signal.info.sampling_frequency);
            uint64_t new_num_samples = pSignal->complex_signal.info.num_samples - num_samples_to_delete;
            memmove(pSignal->complex_signal.pValues, pSignal->complex_signal.pValues + num_samples_to_delete, sizeof(double complex) * new_num_samples);
            pSignal->complex_signal.pValues = (double complex*) realloc(pSignal->complex_signal.pValues, sizeof(double complex) * new_num_samples);

            pSignal->complex_signal.info.num_samples = new_num_samples;
            pSignal->complex_signal.info.start_time = newStartTime;
        } else {
            uint64_t num_samples_to_delete = (uint64_t)((-t1Diff) * pSignal->real_signal.info.sampling_frequency);
            uint64_t new_num_samples = pSignal->real_signal.info.num_samples - num_samples_to_delete;
            memmove(pSignal->real_signal.pValues, pSignal->real_signal.pValues + num_samples_to_delete, sizeof(double) * new_num_samples);
            pSignal->real_signal.pValues = (double*) realloc(pSignal->real_signal.pValues, sizeof(double) * new_num_samples);

            pSignal->real_signal.info.num_samples = new_num_samples;
            pSignal->real_signal.info.start_time = newStartTime;
        }
        
    }
}

/** @verify */
void signal_domain_adjust_end_time(signal_t* pSignal, double oldEndTime, double newEndTime) {
    double t2Diff = newEndTime - oldEndTime;
    if (t2Diff > 0) {
        uint64_t num_samples_to_append = 0;
        if (pSignal->treat_as_complex) {
            num_samples_to_append = (uint64_t)(t2Diff * pSignal->complex_signal.info.sampling_frequency);
        } else {
            num_samples_to_append = (uint64_t)(t2Diff * pSignal->real_signal.info.sampling_frequency);
        }
        signal_t tempSignal = {
            .treat_as_complex = pSignal->treat_as_complex
        };

        if (pSignal->treat_as_complex) {
            tempSignal.complex_signal = (complex_signal_t) {
                .info = {
                    .num_samples = pSignal->complex_signal.info.num_samples + num_samples_to_append,
                    .sampling_frequency = pSignal->complex_signal.info.sampling_frequency,
                    .start_time = pSignal->complex_signal.info.start_time
                },
                .pValues = 0
            };
        } else {
            tempSignal.real_signal = (real_signal_t) {
                .info = {
                    .num_samples = pSignal->real_signal.info.num_samples + num_samples_to_append,
                    .sampling_frequency = pSignal->real_signal.info.sampling_frequency,
                    .start_time = pSignal->real_signal.info.start_time
                },
                .pValues = 0
            };
        }

        signal_alloc_values (&tempSignal);

        if (pSignal->treat_as_complex) {
            for (uint64_t i = 0; i < pSignal->complex_signal.info.num_samples; i++) {
                tempSignal.complex_signal.pValues[i] = pSignal->complex_signal.pValues[i];
            }
            for (uint64_t i = 0; i < num_samples_to_append; i++) {
                tempSignal.complex_signal.pValues[i + pSignal->complex_signal.info.num_samples] = 0;
            }
        } else {
            for (uint64_t i = 0; i < pSignal->real_signal.info.num_samples; i++) {
                tempSignal.real_signal.pValues[i] = pSignal->real_signal.pValues[i];
            }
            for (uint64_t i = 0; i < num_samples_to_append; i++) {
                tempSignal.real_signal.pValues[i + pSignal->real_signal.info.num_samples] = 0;
            }
        }

        signal_free_values (pSignal);
        if (pSignal->treat_as_complex) {
            pSignal->complex_signal.pValues = tempSignal.complex_signal.pValues;
            pSignal->complex_signal.info.num_samples = tempSignal.complex_signal.info.num_samples;
        } else {
            pSignal->real_signal.pValues = tempSignal.real_signal.pValues;
            pSignal->real_signal.info.num_samples = tempSignal.real_signal.info.num_samples;
        }
    } else if (t2Diff < 0) {
        if (pSignal->treat_as_complex) {
            uint64_t num_samples_to_delete = (uint64_t)((-t2Diff) * pSignal->complex_signal.info.sampling_frequency);
            uint64_t new_num_samples = pSignal->complex_signal.info.num_samples - num_samples_to_delete;
            pSignal->complex_signal.pValues = (double complex*) realloc(pSignal->complex_signal.pValues, sizeof(double complex) * new_num_samples);
            pSignal->complex_signal.info.num_samples = new_num_samples;
        } else {
            uint64_t num_samples_to_delete = (uint64_t)((-t2Diff) * pSignal->real_signal.info.sampling_frequency);
            uint64_t new_num_samples = pSignal->real_signal.info.num_samples - num_samples_to_delete;
            pSignal->real_signal.pValues = (double*) realloc(pSignal->real_signal.pValues, sizeof(double) * new_num_samples);
            pSignal->real_signal.info.num_samples = new_num_samples;
        }
    }
}

void real_signal_domain_adjust_start_time(real_signal_t* pSignal, double newStartTime) {
    signal_t tempSignal = {
        .treat_as_complex = false,
        .real_signal = *pSignal
    };
    signal_domain_adjust_start_time(&tempSignal, newStartTime);
    *pSignal = tempSignal.real_signal;
}

void real_signal_domain_adjust_end_time(real_signal_t* pSignal, double oldEndTime, double newEndTime) {
    signal_t tempSignal = {
        .treat_as_complex = false,
        .real_signal = *pSignal
    };
    signal_domain_adjust_end_time(&tempSignal, oldEndTime, newEndTime);
    *pSignal = tempSignal.real_signal;
}

void complex_signal_domain_adjust_start_time(complex_signal_t* pSignal, double newStartTime) {
    signal_t tempSignal = {
        .treat_as_complex = true,
        .complex_signal = *pSignal
    };
    signal_domain_adjust_start_time(&tempSignal, newStartTime);
    *pSignal = tempSignal.complex_signal;
}

void complex_signal_domain_adjust_end_time(complex_signal_t* pSignal, double oldEndTime, double newEndTime) {
    signal_t tempSignal = {
        .treat_as_complex = true,
        .complex_signal = *pSignal
    };
    signal_domain_adjust_end_time(&tempSignal, oldEndTime, newEndTime);
    *pSignal = tempSignal.complex_signal;
}

void real_signal_reverse(real_signal_t* pSignal) {
    uint64_t iMaxInclusive = pSignal->info.num_samples >> 1;
    double* pValueFirst = pSignal->pValues;
    double* pValueLast = pSignal->pValues + pSignal->info.num_samples - 1;
    for (uint64_t i = 0; i <= iMaxInclusive; i++) {
        double* pValueLeft = pValueFirst + i;
        double* pValueRight = pValueLast - i; 
        double temp = *pValueLeft;
        *pValueLeft = *pValueRight;
        *pValueRight = temp;
    }
}

void complex_signal_reverse(complex_signal_t* pSignal) {
    uint64_t iMaxInclusive = pSignal->info.num_samples >> 1;
    double complex* pValueFirst = pSignal->pValues;
    double complex* pValueLast = pSignal->pValues + pSignal->info.num_samples - 1;
    for (uint64_t i = 0; i <= iMaxInclusive; i++) {
        double complex* pValueLeft = pValueFirst + i;
        double complex* pValueRight = pValueLast - i; 
        double complex temp = *pValueLeft;
        *pValueLeft = *pValueRight;
        *pValueRight = temp;
    }
}

void signal_reverse(signal_t* pSignal) {
    if (pSignal->treat_as_complex) {
        complex_signal_reverse(&pSignal->complex_signal);
    } else {
        real_signal_reverse(&pSignal->real_signal);
    }
}

void real_signal_timeshift(real_signal_t* pSignal, double timeshiftValue) {
    pSignal->info.start_time += timeshiftValue;
}

void complex_signal_timeshift(complex_signal_t* pSignal, double timeshiftValue) {
    pSignal->info.start_time += timeshiftValue;
}

void signal_timeshift(signal_t* pSignal, double timeshiftValue) {
    if (pSignal->treat_as_complex) {
        complex_signal_timeshift(&pSignal->complex_signal, timeshiftValue);
    } else {
        real_signal_timeshift(&pSignal->real_signal, timeshiftValue);
    }
}

void signal_collapse_signals_tdomains(signal_t* pSignal_1, signal_t* pSignal_2) {
    double sig1_dt, sig2_dt, sig1_startTime, sig1_endTime, sig2_startTime, sig2_endTime;
    
    if (pSignal_1->treat_as_complex) {
        sig1_dt = 1.0 / pSignal_1->complex_signal.info.sampling_frequency;
        sig1_startTime = pSignal_1->complex_signal.info.start_time;
        sig1_endTime = sig1_startTime + sig1_dt * pSignal_1->complex_signal.info.num_samples;
    } else {
        sig1_dt = 1.0 / pSignal_1->real_signal.info.sampling_frequency;
        sig1_startTime = pSignal_1->real_signal.info.start_time;
        sig1_endTime = sig1_startTime + sig1_dt * pSignal_1->real_signal.info.num_samples;
    }

    if (pSignal_2->treat_as_complex) {
        sig2_dt = 1.0 / pSignal_2->complex_signal.info.sampling_frequency;
        sig2_startTime = pSignal_2->complex_signal.info.start_time;
        sig2_endTime = sig2_startTime + sig2_dt * pSignal_2->complex_signal.info.num_samples;
    } else {
        sig2_dt = 1.0 / pSignal_2->real_signal.info.sampling_frequency;
        sig2_startTime = pSignal_2->real_signal.info.start_time;
        sig2_endTime = sig2_startTime + sig2_dt * pSignal_2->real_signal.info.num_samples;
    }
    
    double newStartTime = sig1_startTime < sig2_startTime ? sig2_startTime : sig1_startTime;
    double newEndTime = sig1_endTime > sig2_endTime ? sig2_endTime : sig1_endTime;
    
    signal_domain_adjust_start_time(pSignal_1, newStartTime);
    signal_domain_adjust_start_time(pSignal_2, newStartTime);
    signal_domain_adjust_end_time(pSignal_1, sig1_endTime, newEndTime);
    signal_domain_adjust_end_time(pSignal_2, sig2_endTime, newEndTime);
    
}

void real_signal_collapse_signals_tdomains(real_signal_t* pSignal_1, real_signal_t* pSignal_2) {
    signal_t signal_1 = {
        .treat_as_complex = false,
        .real_signal = *pSignal_1
    };
    signal_t signal_2 = {
        .treat_as_complex = false,
        .real_signal = *pSignal_2
    };
    signal_collapse_signals_tdomains(&signal_1, &signal_2);
    *pSignal_1 = signal_1.real_signal;
    *pSignal_2 = signal_2.real_signal;
}

void complex_signal_collapse_signals_tdomains(complex_signal_t* pSignal_1, complex_signal_t* pSignal_2) {
    signal_t signal_1 = {
        .treat_as_complex = true,
        .complex_signal = *pSignal_1
    };
    signal_t signal_2 = {
        .treat_as_complex = true,
        .complex_signal = *pSignal_2
    };
    signal_collapse_signals_tdomains(&signal_1, &signal_2);
    *pSignal_1 = signal_1.complex_signal;
    *pSignal_2 = signal_2.complex_signal;
}
