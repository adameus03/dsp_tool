typedef enum {
    CONTROLLER_COMPLEX_PLOTTING_MODE_NONE,
    CONTROLLER_COMPLEX_PLOTTING_MODE_CARTESIAN,
    CONTROLLER_COMPLEX_PLOTTING_MODE_POLAR,
    CONTROLLER_COMPLEX_PLOTTING_MODE_PARAMETRIC_CURVE
} controller_complex_plotting_mode;

/**
 * @returns Exit status (0 for success)
*/
int controller_run(int* psArgc, char*** pppcArgv);

