//
// Created by skohl on 13.04.2025.
//

#ifndef HTTP_TASK_H
#define HTTP_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
    #include "status_control_task.h"

    /**
     * Initializes the internal structure
     *
     * @param task_handles pointer to task handle struct
     */
    void init_http_stat( task_handles_t* task_handles);

    /**
     * HTTP Task
     *
     * When Notify-Bit given:
     *   * creates a socket
     *   * requests a frame from server
     *   * validates the downloaded data
     *   * notifies status_control_task
     *
     * @param pvParameters  A pointer to task parameters, if any, used for task-specific
     *                    needs. Typically used to pass structures or other data
     *                    necessary for task operation.
     */
    void http_task( void* pvParameters );
#ifdef __cplusplus
}
#endif
#endif  // HTTP_TASK_H
