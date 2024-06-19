#include <pthread.h>
#include <stdbool.h>

#define THREADBOX_MAX_TASKS 1

typedef void* (*threadbox_task_fn)(void* pTaskData);

typedef struct {
    pthread_t thread;
    /* The task function to run */
    threadbox_task_fn taskFn;
    /* The data to pass to the task function */
    void* pTaskData;
    bool isRunning;
} threadbox_t;

/**
 * Creates a threadbox
 */
threadbox_t threadbox_create();

void threadbox_set_task_function(threadbox_t* pThreadbox, threadbox_task_fn taskFn);
void threadbox_set_task_data(threadbox_t* pThreadbox, void* pTaskData);

/**
 * Asynchronously runs the threadbox task function in a separate thread
 * @param pThreadbox The threadbox to run
 * @returns true if the task was successfully started, false otherwise
 */
bool threadbox_task_run(threadbox_t* pThreadbox);

/**
 * Kills the specified task
 * @param taskId The ID of the task to kill
 */
void threadbox_task_kill(threadbox_t* pThreadbox);