#include "gnuplot.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h> //floor and ceil
#include "transform.h"
#include "__gcall.h"


//real_signal_t* pCachedSignal = 0;

/**
 * Write signal samples to a file, use a format recognizable by gnuplot
*/
void __gnuplot_prepare_real_signal_data(real_signal_t* pSignal) {
    /*if (pSignal == pCachedSignal) { return; } else { 
        fprintf(stdout, "Info: Caching signal address\n");
        pCachedSignal = pSignal; 
    }*/

    FILE* data_file;
    data_file = fopen(GNUPLOT_DATAFILE_PATH, "w");
    if (data_file == NULL) {
        fprintf(stderr, "Error: Could not open gnuplot data file for writing\n");
        return;
    }

    double dt = 1.0 / pSignal->info.sampling_frequency;

    for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
        if (__isnan(pSignal->pValues[i])) {
            continue;    
        }
        if ((pSignal->pValues[i] > MAX_SIGNAL_PLOTTED_VALUE) || (pSignal->pValues[i] < MIN_SIGNAL_PLOTTED_VALUE)) {
            continue;
        }
        fprintf(data_file, "%f %f\n", pSignal->info.start_time + i * dt, pSignal->pValues[i]);
    }
    
    fclose(data_file);
}

/** 
 * @attention Redundant, because gnuplot can do it
 * @_optimize DRY 
 * */
void __gnuplot_prepare_real_signal_histogram_data(real_signal_t* pSignal, uint64_t num_intervals) { 
    rsignal_to_histogram_transform (pSignal, num_intervals);

    FILE* data_file;
    data_file = fopen(GNUPLOT_DATAFILE_PATH, "w");
    if (data_file == NULL) {
        fprintf(stderr, "Error: Could not open gnuplot data file for writing\n");
        return;
    }

    fprintf(stderr, "Not implemented (this function is obsolete)"); exit(EXIT_FAILURE); //

    fclose(data_file);
}



void gnuplot_prepare_real_signal_plot(real_signal_t* pSignal, const char* script_path) {
    __gnuplot_prepare_real_signal_data(pSignal);
    // Call gnuplot to create output image
    __gnuplot_simple_call(script_path);
}
/*void gnuplot_prepare_complex_signal_plots(complex_signal_t* pSignal) {
    fprintf(stderr, "Error: Complex signal plotting not implemented yet\n");
}*/
void gnuplot_prepare_real_signal_histogram(real_signal_t* pSignal, uint64_t num_intervals, char* plotTitle, char* script_path) {
    if (pSignal->info.num_samples == 0) {
        fprintf(stderr, "Error: Won't prepare a histogram for a null signal\n");
        return;
    }
    
    double minimalSignalValue = pSignal->pValues[0];
    double maximalSignalValue = pSignal->pValues[0];

    if (__isnan(minimalSignalValue)) { minimalSignalValue = 0; }
    if (__isnan(maximalSignalValue)) { maximalSignalValue = 0; }
    if ((minimalSignalValue < MIN_SIGNAL_PLOTTED_VALUE) || (minimalSignalValue > MAX_SIGNAL_PLOTTED_VALUE)) {
        minimalSignalValue = 0;
    }
    if ((maximalSignalValue < MIN_SIGNAL_PLOTTED_VALUE) || (maximalSignalValue > MAX_SIGNAL_PLOTTED_VALUE)) {
        maximalSignalValue = 0;
    }

    for (uint64_t i = 1; i < pSignal->info.num_samples; i++) {
        double* pValue = (pSignal->pValues) + i;
        if (__isnan(*pValue)) {
            continue;
        if (((*pValue) < MIN_SIGNAL_PLOTTED_VALUE) || ((*pValue) > MAX_SIGNAL_PLOTTED_VALUE)) {
            continue;
        }
        } else if (*pValue < minimalSignalValue) {
            minimalSignalValue = *pValue;
        } else if (*pValue > maximalSignalValue ) {
            maximalSignalValue = *pValue;
        }
    }
    
    printf("Minimal=%f; maximal=%f\n", minimalSignalValue, maximalSignalValue);
    if (maximalSignalValue - minimalSignalValue < 0.01) {
        fprintf(stdout, "Info: Widened narrow minmax for gnuplot histogram\n");
        minimalSignalValue -= 0.1;
        maximalSignalValue += 0.1;
    }

    //double minmax = maximalSignalValue - minimalSignalValue;
    //minimalSignalValue -= 0.1 * minmax;
    //maximalSignalValue += 0.1 * minmax;
    //minimalSignalValue = floor(minimalSignalValue);
    //maximalSignalValue = ceil(maximalSignalValue);

    //__gnuplot_prepare_real_signal_data(pSignal); /** @note The prepared data file should typically already be waiting there */
    gnuplot_arglist_t gnuplotArglist = { .num_args = 6U, .args = 0 };
    __gnuplot_arglist_aloc_args (&gnuplotArglist);

    char nStr[20]; char minStr[20]; char maxStr[20];
    snprintf(nStr, 20, "%lu", num_intervals); 
    snprintf(minStr, 20, "%f", minimalSignalValue); 
    snprintf(maxStr, 20, "%f", maximalSignalValue);

    gnuplotArglist.args[0] = (gnuplot_arg_t) { .argname = "n", .argval = nStr };
    gnuplotArglist.args[1] = (gnuplot_arg_t) { .argname = "min", .argval = minStr };
    gnuplotArglist.args[2] = (gnuplot_arg_t) { .argname = "max", .argval = maxStr };
    gnuplotArglist.args[3] = (gnuplot_arg_t) { .argname = "infile", .argval = GNUPLOT_DATAFILE_PATH };
    gnuplotArglist.args[4] = (gnuplot_arg_t) { .argname = "outfile", .argval = GNUPLOT_OUTFILE_PATH };
    gnuplotArglist.args[5] = (gnuplot_arg_t) { .argname = "plottitle", .argval = plotTitle };

    __gnuplot_advanced_call (script_path, &gnuplotArglist);
    __gnuplot_arglist_free_args (&gnuplotArglist);
}

void gnuplot_cleanup() {
    remove(GNUPLOT_DATAFILE_PATH);
    remove(GNUPLOT_OUTFILE_PATH);
}