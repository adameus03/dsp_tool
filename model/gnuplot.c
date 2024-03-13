#include "gnuplot.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Write signal samples to a file, use a format recognizable by gnuplot
*/
void __gnuplot_prepare_real_signal_data(real_signal_t* pSignal) {
    FILE* data_file;
    data_file = fopen(GNUPLOT_DATAFILE_PATH, "w");
    if (data_file == NULL) {
        fprintf(stderr, "Error: Could not open gnuplot data file for writing\n");
        return;
    }

    double dt = 1.0 / pSignal->info.sampling_frequency;

    for (uint64_t i = 0; i < pSignal->info.num_samples; i++) {
        fprintf(data_file, "%f %f\n", pSignal->info.start_time + i * dt, pSignal->pValues[i]);
    }
    
    fclose(data_file);
}

void __gnuplot_prepare_real_signal_historgram_data(real_signal_t* pSignal) {
    
}

void __gnuplot_call(const char* script_path) {
    char cmd_buf[50];
    snprintf(cmd_buf, 50, "gnuplot %s", script_path);
    int status = system(cmd_buf);
    if (status != 0) {
        fprintf(stderr, "Error: gnuplot call failed");
    }
}

void gnuplot_prepare_real_signal_plot(real_signal_t* pSignal, const char* script_path) {
    __gnuplot_prepare_real_signal_data(pSignal);
    // Call gnuplot to create output image
    __gnuplot_call(script_path);
}
void gnuplot_prepare_complex_signal_plots(complex_signal_t* pSignal) {
    fprintf(stderr, "Error: Complex signal plotting not implemented yet\n");
}
void gnuplot_prepare_real_signal_histogram(real_signal_t* pSignal) {
    
}

void gnuplot_cleanup() {
    remove(GNUPLOT_DATAFILE_PATH);
    remove(GNUPLOT_OUTFILE_PATH);
}