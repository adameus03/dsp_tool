#include "signal.h"

#define GNUPLOT_DATAFILE_PATH "data.txt"
#define GNUPLOT_SCRIPT_PATH_PLOT_A "gnuplot_scripts/script_plot_A.plt"
#define GNUPLOT_SCRIPT_PATH_PLOT_B "gnuplot_scripts/script_plot_B.plt"
#define GNUPLOT_SCRIPT_PATH_HISTOGRAM "gnuplot_scripts/script_histogram.plt"

#define GNUPLOT_OUTFILE_PATH "out.png"

void gnuplot_prepare_real_signal_plot(real_signal_t* pSignal, const char* script_path);
void gnuplot_prepare_complex_signal_plots(complex_signal_t* pSignal);
void gnuplot_prepare_real_signal_histogram(real_signal_t* pSignal, uint64_t num_intervals, char* plotTitle, char* script_path);
// TODO: What about complex signal histograms?

void gnuplot_cleanup();