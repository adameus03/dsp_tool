
#include "model/fir.h"
typedef void (*controller_fir_callback_fn)(fir_common_config_t);

/**
 * @returns 0 on success, 1 on failure
*/
int controller_fir_run(controller_fir_callback_fn windowDestroyCallback);
