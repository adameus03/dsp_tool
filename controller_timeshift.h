//#include <semaphore.h>

/**
 * @returns Pointer to semaphore which can be used to wait for window destruction
*/
//sem_t* controller_timeshift_run();


typedef void (*timeshift_callback_fn)(double timeshiftValue);

/**
 * @returns 0 on success, 1 on failure
*/
int controller_timeshift_run(timeshift_callback_fn windowDestroyCallback);

//void controller_timeshift_cleanup();
