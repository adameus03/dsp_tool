#include "signal.h"

#define GNUPLOT_DATAFILE_PATH "data.txt"
#define GNUPLOT_DATAFILE_EXTRA_PATH "data_extra.txt"
#define GNUPLOT_SCRIPT_PATH_PLOT_A_OLD "gnuplot_scripts/script_plot_A_old.plt"
#define GNUPLOT_SCRIPT_PATH_PLOT_B_OLD "gnuplot_scripts/script_plot_B_old.plt"
#define GNUPLOT_SCRIPT_PATH_PLOT "gnuplot_scripts/script_plot.plt"
#define GNUPLOT_SCRIPT_PATH_HISTOGRAM "gnuplot_scripts/script_histogram.plt"

#define GNUPLOT_OUTFILE_PATH "out.png"

#define MAX_SIGNAL_PLOTTED_VALUE 1000000
#define MIN_SIGNAL_PLOTTED_VALUE -1000000

#define GNUPLOT_DEFAULT_SIZEX 800
#define GNUPLOT_DEFAULT_SIZEY 640

#define GNUPLOT_UNITS_LABEL_TIME "t [s]"
#define GNUPLOT_UNITS_LABEL_FREQUENCY "f [Hz]"
#define GNUPLOT_UNITS_LABEL_AMPLITUDE_VOLTAGE "A [V]"

typedef enum {
    GNUPLOT_SIZE_MODE_DEFAULT,
    GNUPLOT_SIZE_MODE_DEFAULT_HALF_HEIGHT
} gnuplot_size_mode;

//void gnuplot_prepare_real_signal_plot(real_signal_t* pSignal, const char* script_path, bool isExtra);
void gnuplot_prepare_real_signal_plot(real_signal_t* pSignal, char* plotTitle, char* script_path, bool isExtra, gnuplot_size_mode sizeMode, char* domainUnitsLabel, char* codomainUnitsLabel);
// [TODO] Maybe do parametric plotting? (experimental)
//void gnuplot_prepare_complex_signal_plots(complex_signal_t* pSignal);
void gnuplot_prepare_real_signal_histogram(real_signal_t* pSignal, uint64_t num_intervals, char* plotTitle, char* script_path, bool isExtra);

void gnuplot_cleanup();