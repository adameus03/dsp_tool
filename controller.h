typedef enum {
    CONTROLLER_COMPLEX_PLOTTING_MODE_NONE,
    CONTROLLER_COMPLEX_PLOTTING_MODE_CARTESIAN,
    CONTROLLER_COMPLEX_PLOTTING_MODE_POLAR,
    CONTROLLER_COMPLEX_PLOTTING_MODE_PARAMETRIC_CURVE
} controller_complex_plotting_mode;

typedef enum {
    CONTROLLER_PLOT_DOMAIN_UNITS_SECONDS, // Time (default)
    CONTROLLER_PLOT_DOMAIN_UNITS_HERTZ, // Frequency
} controller_plot_domain_units;

/**
 * @returns Exit status (0 for success)
*/
int controller_run(int* psArgc, char*** pppcArgv);

