#include "transform.h"
#include <stdlib.h>
#include <stdio.h> // for error logging
#include <memory.h>
#include <assert.h>
#define __USE_MISC
#include <math.h>
void histogram_data_aloc_codomain (pHistogram_data_t pHistogramData) { pHistogramData->codomain_values = (uint64_t*) malloc(pHistogramData->num_intervals * sizeof(uint64_t)); }
void histogram_data_free_codomain (pHistogram_data_t pHistogramData) { free(pHistogramData->codomain_values); }

histogram_data_t rsignal_to_histogram_transform(real_signal_t* pRealSignal, uint64_t numIntervals) {
    if (pRealSignal->info.num_samples == 0) {
        fprintf(stderr, "Error: Won't transform a null signal\n");
        return (histogram_data_t) {
            .num_intervals = 0,
            .domain_min = 0,
            .domain_max = 0,
            .codomain_values = 0
        }; 
    }

    double minimalSignalValue = pRealSignal->pValues[0];
    double maximalSignalValue = pRealSignal->pValues[0];

    for (uint64_t i = 1; i < pRealSignal->info.num_samples; i++) {
        double* pValue = pRealSignal->pValues + i;
        if (*pValue < minimalSignalValue) {
            minimalSignalValue = *pValue;
        } else if (*pValue > maximalSignalValue ) {
            maximalSignalValue = *pValue;
        }
    }

    histogram_data_t histogramData = {
        .num_intervals = numIntervals,
        .domain_min = minimalSignalValue,
        .domain_max = maximalSignalValue,
        .codomain_values = 0
    };

    histogram_data_aloc_codomain (&histogramData);

    if (histogramData.codomain_values == 0) {
        fprintf (stderr, "Error: Failed to allocate codomain for transform result\n");
    }
    
    double intervalLength = ((double)(maximalSignalValue - minimalSignalValue)) / (double)numIntervals;
    
    for (uint64_t i = 0; i < histogramData.num_intervals; i++) {
        histogramData.codomain_values[i] = 0U;
    }
    
    uint64_t* pHistogramCodomainValue = 0;
    double intervalLeftBound = 0;
    double intervalRightBound = 0;
    
    for (uint64_t i = 0; i < pRealSignal->info.num_samples; i++) {
        double* pRSignalValue = pRealSignal->pValues + i;
        
        /*uint64_t leftIntervalIndex = 0;
        uint64_t rightIntervalIndex = */
        
        #ifdef HISTOGRAM_TRANSFORM_USE_LINEAR_SEARCH
        pHistogramCodomainValue = histogramData.codomain_values;
        intervalRightBound = histogramData.domain_min + intervalLength;
        if (*pRSignalValue < intervalRightBound) {
            (*pHistogramCodomainValue)++;
        } else {
            unsigned char interval_found = 0x0;
            for (uint64_t j = 0; j < histogramData.num_intervals - 1; j++) {
                pHistogramCodomainValue = histogramData.codomain_values + j;
                intervalLeftBound = histogramData.domain_min + j * intervalLength;
                intervalRightBound = intervalLeftBound + intervalLength;
                if ((*pRSignalValue >= intervalLeftBound) && (*pRSignalValue < intervalRightBound)) {
                    (*pHistogramCodomainValue)++;
                    interval_found = 0x1; break;
                }
            }
            if (interval_found == 0x0) {
                pHistogramCodomainValue = histogramData.codomain_values + histogramData.num_intervals - 1;
                intervalLeftBound = histogramData.domain_min + (histogramData.num_intervals - 1) * intervalLength;
                if (*pRSignalValue >= intervalLeftBound) { (*pHistogramCodomainValue)++; }
            }
        }
        #else
        fprintf(stderr, "Binary search for histogram transform not implemented");
        exit (EXIT_FAILURE);
        #endif
        
    }

    return histogramData;
}

static void transform_complex_adjust_frequency_domain(complex_signal_t* pComplexSignal) {
    if (pComplexSignal->info.num_samples == 0) {
        assert(0);
        return;
    }
    // double oldDomainSpan = ((double)(pComplexSignal->info.num_samples - 1)) / pComplexSignal->info.sampling_frequency;
    double newDomainSpan = pComplexSignal->info.sampling_frequency;
    pComplexSignal->info.sampling_frequency = ((double)(pComplexSignal->info.num_samples - 1)) / newDomainSpan;
    pComplexSignal->info.start_time = 0.0;
}

static void transform_real_adjust_frequency_domain(real_signal_t* pRealSignal) {
    if (pRealSignal->info.num_samples == 0) {
        assert(0);
        return;
    }
    // double oldDomainSpan = ((double)(pRealSignal->info.num_samples - 1)) / pRealSignal->info.sampling_frequency;
    double newDomainSpan = pRealSignal->info.sampling_frequency;
    pRealSignal->info.sampling_frequency = ((double)(pRealSignal->info.num_samples - 1)) / newDomainSpan;
    pRealSignal->info.start_time = 0.0;
}

static transform_progress_callback_fn __progress_callback = 0;
void transform_set_progress_callback(transform_progress_callback_fn progressCallback) {
    __progress_callback = progressCallback;
}

//double complex __w_dftKernel

static transform_progress_report_t __progress_report = { .progress = 0.0 };

complex_signal_t transform_dft_real_naive(real_signal_t* pRealSignal) {
    if (pRealSignal->info.num_samples == 0) {
        fprintf(stderr, "Error: Won't transform a null signal\n");
        return (complex_signal_t) {
            .info = pRealSignal->info,
            .pValues = 0
        };
    }

    complex_signal_t dftSignal = {
        .info = pRealSignal->info,
    };

    complex_signal_alloc_values(&dftSignal);

    if (dftSignal.pValues == 0) {
        fprintf(stderr, "Error: Failed to allocate memory for DFT signal\n");
        return dftSignal;
    }

    for (uint64_t i = 0; i < dftSignal.info.num_samples; i++) {
        double complex* pDftValue = dftSignal.pValues + i;
        *pDftValue = 0.0;
        for (uint64_t j = 0; j < dftSignal.info.num_samples; j++) {
            *pDftValue += pRealSignal->pValues[j] * cexp(-2.0 * M_PI * I * (double)i * (double)j / (double)pRealSignal->info.num_samples);
        }
        *pDftValue /= (double)pRealSignal->info.num_samples;
        
        // if (__progress_callback != 0 && (i % 100 == 99)) {
        //     __progress_report = (transform_progress_report_t){
        //         .progress = (double)(i + 1) / (double)dftSignal.info.num_samples
        //     };
        //     __progress_callback(&__progress_report);
        // }
    }

    transform_complex_adjust_frequency_domain(&dftSignal);

    return dftSignal;
}

complex_signal_t transform_idft_complex_naive(complex_signal_t* pComplexSignal) {
    if (pComplexSignal->info.num_samples == 0) {
        fprintf(stderr, "Error: Won't transform a null signal\n");
        return (complex_signal_t) {
            .info = pComplexSignal->info,
            .pValues = 0
        };
    }

    complex_signal_t idftSignal = {
        .info = pComplexSignal->info,
    };

    complex_signal_alloc_values(&idftSignal);

    if (idftSignal.pValues == 0) {
        fprintf(stderr, "Error: Failed to allocate memory for DFT signal\n");
        return idftSignal;
    }

    for (uint64_t i = 0; i < idftSignal.info.num_samples; i++) {
        double complex* pDftValue = idftSignal.pValues + i;
        *pDftValue = 0.0;
        for (uint64_t j = 0; j < idftSignal.info.num_samples; j++) {
            *pDftValue += pComplexSignal->pValues[j] * cexp(2.0 * M_PI * I * (double)i * (double)j / (double)pComplexSignal->info.num_samples);
        }
        *pDftValue /= (double)pComplexSignal->info.num_samples;
    }

    return idftSignal;
}

uint64_t transform_bits_reverse(uint64_t x, uint64_t m) {
    uint64_t y = 0;
    for (uint64_t i = 0; i < m; i++) {
        y <<= 1;
        y |= x & 1;
        x >>= 1;
    }
    return y;
}

complex_signal_t transform_dft_real_fast_p2(real_signal_t* pRealSignal) {
    fprintf(stdout, "transform_dft_real_fast_p2 called\n");
    
    // Set m as the smallest power of 2 that is greater than or equal to the original number of samples
    uint64_t m = 0;
    uint64_t n = pRealSignal->info.num_samples;
    while (n > 1) {
        n >>= 1;
        m++;
    }

    uint64_t lenDiff = (1U << m) - pRealSignal->info.num_samples;
    if (lenDiff > 0) {
        fprintf(stdout, "Warning: Zero-padding the input signal copy to the nearest power of 2 for performing FFT\n");
    }

    complex_signal_t s1 = { .info = pRealSignal->info };
    complex_signal_t s2 = { .info = pRealSignal->info };

    s1.info.num_samples = 1U << m;
    s2.info.num_samples = s1.info.num_samples;

    complex_signal_alloc_values(&s1);
    complex_signal_alloc_values(&s2);

    for (uint64_t i = 0; i < s2.info.num_samples; i++) {
        s2.pValues[i] = (double complex) pRealSignal->pValues[i];
    }

    for (uint64_t i = pRealSignal->info.num_samples; i < s2.info.num_samples; i++) {
        s2.pValues[i] = 0.0;
    }

    for (uint64_t i = 0; i < s1.info.num_samples; i++) {
        uint64_t jRev = transform_bits_reverse(i, m);
        // assert(jRev < s1.info.num_samples);
        s1.pValues[i] = s2.pValues[jRev];
    }

    complex_signal_t* pS1 = &s1;
    complex_signal_t* pS2 = &s2;

    uint64_t blk_size = 1;
    uint64_t num_blks = s1.info.num_samples;

    while (num_blks > 1) {
        blk_size <<= 1;
        num_blks >>= 1;
         
        for (uint64_t i = 0; i < num_blks; i++) {
            double complex* pSourceSublkCpy = pS1->pValues + (blk_size * i);
            double complex* pSourceSublkAgg = pSourceSublkCpy + (blk_size >> 1);
            double complex* pTargetSublkAdd = pS2->pValues + (blk_size * i);
            double complex* pTargetSublkSub = pTargetSublkAdd + (blk_size >> 1);

            for (uint64_t j = 0; j < blk_size >> 1; j++) {
                double complex* pSourceSublkCpyValue = pSourceSublkCpy + j;
                double complex* pSourceSublkAggValue = pSourceSublkAgg + j;
                double complex* pTargetSublkAddValue = pTargetSublkAdd + j;
                double complex* pTargetSublkSubValue = pTargetSublkSub + j;

                // fprintf(stdout, "[dbg] blk_size = %lu, num_blks = %lu, i = %lu, j = %lu\n", blk_size, num_blks, i, j);
                // fprintf(stdout, "[dbg] Before (source):  pSourceSublkCpyValue = %lf%+lfi, pSourceSublkAggValue = %lf%+lfi\n", creal(*pSourceSublkCpyValue), cimag(*pSourceSublkCpyValue), creal(*pSourceSublkAggValue), cimag(*pSourceSublkAggValue));
                // assert(pSourceSublkCpyValue < pS1->pValues + s1.info.num_samples);
                // assert(pSourceSublkAggValue < pS1->pValues + s1.info.num_samples);
                // assert(pTargetSublkAddValue < pS2->pValues + s2.info.num_samples);
                // assert(pTargetSublkSubValue < pS2->pValues + s2.info.num_samples);

                // Displacements for debugging the butterfly diagram implementation of FFT
                // int sourceSublkCpyValueDisplacement = (int)(pSourceSublkCpyValue - pS1->pValues);
                // int sourceSublkAggValueDisplacement = (int)(pSourceSublkAggValue - pS1->pValues);
                // int targetSublkAddValueDisplacement = (int)(pTargetSublkAddValue - pS2->pValues);
                // int targetSublkSubValueDisplacement = (int)(pTargetSublkSubValue - pS2->pValues);
                
                double complex scaler = cexp(-2.0 * M_PI * I / (double)blk_size * (double)j);
                // fprintf(stdout, "[dbg] Scaler: %lf%+lfi\n", creal(scaler), cimag(scaler));

                *pTargetSublkAddValue = *pSourceSublkCpyValue;
                *pTargetSublkAddValue += (*pSourceSublkAggValue) * scaler;
                *pTargetSublkSubValue = *pSourceSublkCpyValue;
                *pTargetSublkSubValue -= (*pSourceSublkAggValue) * scaler;

                // fprintf(stdout, "[dbg] After (target): pTargetSublkAddValue = %lf%+lfi, pTargetSublkSubValue = %lf%+lfi\n", creal(*pTargetSublkAddValue), cimag(*pTargetSublkAddValue), creal(*pTargetSublkSubValue), cimag(*pTargetSublkSubValue));
                // fprintf(stdout, "[dbg] Displacements: sourceSublkCpyValue = %d, sourceSublkAggValue = %d, targetSublkAddValue = %d, targetSublkSubValue = %d\n", sourceSublkCpyValueDisplacement, sourceSublkAggValueDisplacement, targetSublkAddValueDisplacement, targetSublkSubValueDisplacement);
            }
        }
        complex_signal_t* pS = pS1;
        pS1 = pS2;
        pS2 = pS;
    }

    complex_signal_t* pOutputSignal = pS1;
    complex_signal_t* pDisposeSignal = pS2;

    // assert (pS1->pValues != pS2->pValues);

    complex_signal_free_values(pDisposeSignal);

    // Normalize the output signal
    for (uint64_t i = 0; i < pOutputSignal->info.num_samples; i++) {
        double complex* pValue = pOutputSignal->pValues + i;
        *pValue /= pOutputSignal->info.num_samples;
    }

    transform_complex_adjust_frequency_domain(pOutputSignal);

    return *pOutputSignal;    
    
}

complex_signal_t transform_idft_complex_fast_p2(complex_signal_t* pComplexSignal) {
    fprintf(stdout, "transform_dft_real_fast_p2 called\n");
    
    // Set m as the smallest power of 2 that is greater than or equal to the original number of samples
    uint64_t m = 0;
    uint64_t n = pComplexSignal->info.num_samples;
    while (n > 1) {
        n >>= 1;
        m++;
    }

    uint64_t lenDiff = (1U << m) - pComplexSignal->info.num_samples;
    if (lenDiff > 0) {
        fprintf(stdout, "Warning: Zero-padding the input signal copy to the nearest power of 2 for performing FFT\n");
    }

    complex_signal_t s1 = { .info = pComplexSignal->info };
    complex_signal_t s2 = { .info = pComplexSignal->info };

    s1.info.num_samples = 1U << m;
    s2.info.num_samples = s1.info.num_samples;

    complex_signal_alloc_values(&s1);
    complex_signal_alloc_values(&s2);

    for (uint64_t i = 0; i < s2.info.num_samples; i++) {
        s2.pValues[i] = (double complex) pComplexSignal->pValues[i];
    }

    for (uint64_t i = pComplexSignal->info.num_samples; i < s2.info.num_samples; i++) {
        s2.pValues[i] = 0.0;
    }

    for (uint64_t i = 0; i < s1.info.num_samples; i++) {
        uint64_t jRev = transform_bits_reverse(i, m);
        // assert(jRev < s1.info.num_samples);
        s1.pValues[i] = s2.pValues[jRev];
    }

    complex_signal_t* pS1 = &s1;
    complex_signal_t* pS2 = &s2;

    uint64_t blk_size = 1;
    uint64_t num_blks = s1.info.num_samples;

    while (num_blks > 1) {
        blk_size <<= 1;
        num_blks >>= 1;
         
        for (uint64_t i = 0; i < num_blks; i++) {
            double complex* pSourceSublkCpy = pS1->pValues + (blk_size * i);
            double complex* pSourceSublkAgg = pSourceSublkCpy + (blk_size >> 1);
            double complex* pTargetSublkAdd = pS2->pValues + (blk_size * i);
            double complex* pTargetSublkSub = pTargetSublkAdd + (blk_size >> 1);

            for (uint64_t j = 0; j < blk_size >> 1; j++) {
                double complex* pSourceSublkCpyValue = pSourceSublkCpy + j;
                double complex* pSourceSublkAggValue = pSourceSublkAgg + j;
                double complex* pTargetSublkAddValue = pTargetSublkAdd + j;
                double complex* pTargetSublkSubValue = pTargetSublkSub + j;

                // fprintf(stdout, "[dbg] blk_size = %lu, num_blks = %lu, i = %lu, j = %lu\n", blk_size, num_blks, i, j);
                // fprintf(stdout, "[dbg] Before (source):  pSourceSublkCpyValue = %lf%+lfi, pSourceSublkAggValue = %lf%+lfi\n", creal(*pSourceSublkCpyValue), cimag(*pSourceSublkCpyValue), creal(*pSourceSublkAggValue), cimag(*pSourceSublkAggValue));
                // assert(pSourceSublkCpyValue < pS1->pValues + s1.info.num_samples);
                // assert(pSourceSublkAggValue < pS1->pValues + s1.info.num_samples);
                // assert(pTargetSublkAddValue < pS2->pValues + s2.info.num_samples);
                // assert(pTargetSublkSubValue < pS2->pValues + s2.info.num_samples);

                // Displacements for debugging the butterfly diagram implementation of FFT
                // int sourceSublkCpyValueDisplacement = (int)(pSourceSublkCpyValue - pS1->pValues);
                // int sourceSublkAggValueDisplacement = (int)(pSourceSublkAggValue - pS1->pValues);
                // int targetSublkAddValueDisplacement = (int)(pTargetSublkAddValue - pS2->pValues);
                // int targetSublkSubValueDisplacement = (int)(pTargetSublkSubValue - pS2->pValues);
                
                double complex scaler = cexp(2.0 * M_PI * I / (double)blk_size * (double)j);
                // fprintf(stdout, "[dbg] Scaler: %lf%+lfi\n", creal(scaler), cimag(scaler));

                *pTargetSublkAddValue = *pSourceSublkCpyValue;
                *pTargetSublkAddValue += (*pSourceSublkAggValue) * scaler;
                *pTargetSublkSubValue = *pSourceSublkCpyValue;
                *pTargetSublkSubValue -= (*pSourceSublkAggValue) * scaler;

                // fprintf(stdout, "[dbg] After (target): pTargetSublkAddValue = %lf%+lfi, pTargetSublkSubValue = %lf%+lfi\n", creal(*pTargetSublkAddValue), cimag(*pTargetSublkAddValue), creal(*pTargetSublkSubValue), cimag(*pTargetSublkSubValue));
                // fprintf(stdout, "[dbg] Displacements: sourceSublkCpyValue = %d, sourceSublkAggValue = %d, targetSublkAddValue = %d, targetSublkSubValue = %d\n", sourceSublkCpyValueDisplacement, sourceSublkAggValueDisplacement, targetSublkAddValueDisplacement, targetSublkSubValueDisplacement);
            }
        }
        complex_signal_t* pS = pS1;
        pS1 = pS2;
        pS2 = pS;
    }

    complex_signal_t* pOutputSignal = pS1;
    complex_signal_t* pDisposeSignal = pS2;

    // assert (pS1->pValues != pS2->pValues);

    complex_signal_free_values(pDisposeSignal);

    return *pOutputSignal;    
    
}

double* transform_generate_matrix_walsh_hadamard_recursive(uint64_t m) {
    fprintf(stdout, "transform_generate_matrix_walsh_hadamard_recursive called\n");
    uint64_t matrixSize = 1U << m;
    double* pMatrix = (double*) malloc(matrixSize * matrixSize * sizeof(double));

    if (pMatrix == 0) {
        fprintf(stderr, "Error: Failed to allocate memory for Walsh-Hadamard matrix\n");
        return 0;
    }

    if (m == 0) {
        pMatrix[0] = 1.0;
        return pMatrix;
    }

    double* pSubMatrix = transform_generate_matrix_walsh_hadamard_recursive(m - 1);

    if (pSubMatrix == 0) {
        free(pMatrix);
        fprintf(stderr, "Error: Failed to allocate memory for one of the Walsh-Hadamard submatrices\n");
        return 0;
    }

    for (uint64_t i = 0; i < matrixSize; i++) { // iterate over rows
        for (uint64_t j = 0; j < matrixSize; j++) { // iterate over columns
            if (i < matrixSize / 2) {
                if (j < matrixSize / 2) { // A (upper left)
                    pMatrix[i * matrixSize + j] = pSubMatrix[i * (matrixSize / 2) + j];
                } else { // B (upper right)
                    pMatrix[i * matrixSize + j] = pSubMatrix[i * (matrixSize / 2) + j - matrixSize / 2];
                }
            } else {
                if (j < matrixSize / 2) { // C (lower left)
                    pMatrix[i * matrixSize + j] = pSubMatrix[(i - matrixSize / 2) * (matrixSize / 2) + j];
                } else { // D (lower right)
                    pMatrix[i * matrixSize + j] = -pSubMatrix[(i - matrixSize / 2) * (matrixSize / 2) + j - matrixSize / 2];
                }
            }
        }
    }

    free(pSubMatrix);

    return pMatrix;
}

double* transform_generate_matrix_walsh_hadamard_normalized_recursive(uint64_t m) {
    fprintf(stdout, "transform_generate_matrix_walsh_hadamard_normalized_recursive called\n");
    double* pMatrix = transform_generate_matrix_walsh_hadamard_recursive(m);

    if (pMatrix == 0) {
        return 0;
    }

    double divider = sqrt((double)(1U << m));

    for (uint64_t i = 0; i < (1U << m); i++) {
        for (uint64_t j = 0; j < (1U << m); j++) {
            pMatrix[i * (1U << m) + j] /= divider;
        }
    }

    return pMatrix;
}

real_signal_t transform_walsh_hadamard_real_naive(real_signal_t* pRealSignal, walsh_hadamard_config_t* pConfig) {
    fprintf(stdout, "transform_walsh_hadamard_real_naive called\n");
    if (pRealSignal->info.num_samples == 0) {
        fprintf(stderr, "Error: Won't transform a null signal\n");
        return (real_signal_t) {
            .info = pRealSignal->info,
            .pValues = 0
        };
    }

    real_signal_t whSignal = {
        .info = pRealSignal->info
    };

    whSignal.info.num_samples = 1U << pConfig->m;

    real_signal_alloc_values(&whSignal);

    if (whSignal.pValues == 0) {
        fprintf(stderr, "Error: Failed to allocate memory for Walsh-Hadamard signal\n");
        return whSignal;
    }

    double* pMatrix = transform_generate_matrix_walsh_hadamard_normalized_recursive(pConfig->m);

    if (pMatrix == 0) {
        fprintf(stderr, "Error: Failed to generate Walsh-Hadamard matrix with m = %lu\n", pConfig->m);
        return whSignal;
    }

    for (uint64_t i = 0; i < whSignal.info.num_samples; i++) {
        double* pWhValue = whSignal.pValues + i;
        *pWhValue = 0.0;
        // Iterate over input signal samples and at the same time over WH matrix rows
        for (uint64_t j = 0; j < whSignal.info.num_samples; j++) {
            *pWhValue += pRealSignal->pValues[j] * pMatrix[i * whSignal.info.num_samples + j];
        }
    }

    free(pMatrix);

    transform_real_adjust_frequency_domain(&whSignal);

    return whSignal;
}

real_signal_t transform_walsh_hadamard_unnormalized_real_fast(real_signal_t* pRealSignal, walsh_hadamard_config_t* pConfig) {
    fprintf(stdout, "transform_walsh_hadamard_unnormalized_real_fast called\n");
    
    real_signal_t s1 = { .info = pRealSignal->info };
    real_signal_t s2 = { .info = pRealSignal->info };
    s1.info.num_samples = 1U << pConfig->m;
    s2.info.num_samples = s1.info.num_samples;

    if (pRealSignal->info.num_samples < s1.info.num_samples) {
        fprintf(stderr, "Error: Input signal length is less than the side length of the Walsh-Hadamard matrix\n");
        exit(EXIT_FAILURE);
        return (real_signal_t) {
            .info = pRealSignal->info,
            .pValues = 0
        };
    }

    real_signal_alloc_values(&s1);
    real_signal_alloc_values(&s2);
    
    memcpy(s1.pValues, pRealSignal->pValues, s1.info.num_samples * sizeof(double));

    real_signal_t* pS1 = &s1;
    real_signal_t* pS2 = &s2;

    uint64_t blk_size = s1.info.num_samples;
    uint64_t num_blks = 1U;

    while (blk_size > 1) {
        blk_size >>= 1;
        num_blks <<= 1;

        for (uint64_t i = 0; i < num_blks >> 1; i++) {
            double* pTargetBlkAdd = pS2->pValues + ((blk_size * i) << 1);
            double* pTargetBlkSub = pTargetBlkAdd + blk_size;
            double* pSourceBlkCpy = pS1->pValues + ((blk_size * i) << 1);
            double* pSourceBlkAgg = pSourceBlkCpy + blk_size;

            for (uint64_t j = 0; j < blk_size; j++) {
                double* pTargetBlkAddValue = pTargetBlkAdd + j;
                double* pTargetBlkSubValue = pTargetBlkSub + j;
                double* pSourceBlkCpyValue = pSourceBlkCpy + j;
                double* pSourceBlkAggValue = pSourceBlkAgg + j;

                fprintf(stdout, "[dbg] blk_size = %lu, num_blks = %lu, i = %lu, j = %lu\n", blk_size, num_blks, i, j);
                fprintf(stdout, "[dbg] Before (source):  pSourceBlkCpyValue = %lf, pSourceBlkAggValue = %lf\n", *pSourceBlkCpyValue, *pSourceBlkAggValue);

                // Displacements for debugging the butterfly diagram implementation of WHT
                int targetBlkAddValueDisplacement = (int)(pTargetBlkAddValue - pS2->pValues);
                int targetBlkSubValueDisplacement = (int)(pTargetBlkSubValue - pS2->pValues);
                int sourceBlkCpyValueDisplacement = (int)(pSourceBlkCpyValue - pS1->pValues);
                int sourceBlkAggValueDisplacement = (int)(pSourceBlkAggValue - pS1->pValues);

                *pTargetBlkAddValue = *pSourceBlkCpyValue;
                *pTargetBlkAddValue += *pSourceBlkAggValue;
                *pTargetBlkSubValue = *pSourceBlkCpyValue;
                *pTargetBlkSubValue -= *pSourceBlkAggValue;

                fprintf(stdout, "[dbg] After (target): pTargetBlkAddValue = %lf, pTargetBlkSubValue = %lf\n", *pTargetBlkAddValue, *pTargetBlkSubValue);
                fprintf(stdout, "[dbg] Displacements: targetBlkAddValue = %d, targetBlkSubValue = %d, sourceBlkCpyValue = %d, sourceBlkAggValue = %d\n", targetBlkAddValueDisplacement, targetBlkSubValueDisplacement, sourceBlkCpyValueDisplacement, sourceBlkAggValueDisplacement);
            }
        }
        real_signal_t* pS = pS1;
        pS1 = pS2;
        pS2 = pS;
    }

    real_signal_t* pOutputSignal = pS1;
    real_signal_t* pDisposeSignal = pS2;
    
    real_signal_free_values(pDisposeSignal);

    transform_real_adjust_frequency_domain(pOutputSignal);

    return *pOutputSignal;
}

real_signal_t transform_walsh_hadamard_real_fast(real_signal_t* pRealSignal, walsh_hadamard_config_t* pConfig) {
    fprintf(stdout, "transform_walsh_hadamard_real_fast called\n");
    
    real_signal_t s = transform_walsh_hadamard_unnormalized_real_fast(pRealSignal, pConfig);
    
    if (s.pValues && s.info.num_samples) {
        double divider = sqrt((double)(1U << pConfig->m));
        for (uint64_t i = 0; i < s.info.num_samples; i++) {
            double* pValue = s.pValues + i;
            *pValue /= divider;
        }
    } else {
        fprintf(stderr, "Error: blank transformed signal.");
        exit(EXIT_FAILURE);
    }

    return s;
    
}