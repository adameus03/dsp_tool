#include "transform.h"
#include <stdlib.h>
#include <stdio.h> // for error logging
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

//double complex __w_dftKernel

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
        for (uint64_t j = 0; j < pRealSignal->info.num_samples; j++) {
            *pDftValue += pRealSignal->pValues[j] * cexp(-2.0 * M_PI * I * (double)i * (double)j / (double)pRealSignal->info.num_samples);
        }
        *pDftValue /= (double)pRealSignal->info.num_samples;
    }

    return dftSignal;
}