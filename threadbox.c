#include "threadbox.h"
#include <stdio.h>

threadbox_t threadbox_create() {
    return (threadbox_t) {
        .thread = 0,
        .taskFn = NULL,
        .pTaskData = NULL,
        .isRunning = false
    };
}

void threadbox_set_task_function(threadbox_t* pThreadbox, threadbox_task_fn taskFn) {
    pThreadbox->taskFn = taskFn;
}

void threadbox_set_task_data(threadbox_t* pThreadbox, void* pTaskData) {
    pThreadbox->pTaskData = pTaskData;
}

bool threadbox_task_run(threadbox_t* pThreadbox) {
    if (NULL == pThreadbox->taskFn) {
        fprintf(stderr, "No task function specified\n");
        return false;
    }
    if (0 != pthread_create(&pThreadbox->thread, NULL, pThreadbox->taskFn, pThreadbox->pTaskData)) {
        fprintf(stderr, "Failed to create thread\n");
        return false;
    } else {
        pThreadbox->isRunning = true;
        return true;
    }
}

void threadbox_task_kill(threadbox_t* pThreadbox) {
    if (pThreadbox->isRunning && 0 != pThreadbox->thread) {
        pthread_cancel(pThreadbox->thread);
        pThreadbox->thread = 0;
    } else {
        fprintf(stderr, "No thread to kill\n");
    }
}