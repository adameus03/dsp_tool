#include "model/transform.h"
#include <stdbool.h>

// The boolean parameter specifies, whether the configuration was submitted (true) or the user just escaped the window (false)
typedef void (*controller_transform_callback_fn)(transform_common_config_t, bool);
typedef void (*controller_transform_abort_callback_fn)();

void controller_transform_run(controller_transform_callback_fn onConfiguredCb, controller_transform_abort_callback_fn onAbortedCb);

/**
 * @brief Updates progress GUI
 * @note If fraction >= 1.0, the abort button will be deactivated and the EXECUTE TRANSFORM button will be re-enabled 
 */
void controller_transform_set_progress(double fraction);
