//
// Created by skohl on 13.04.2025.
//

#ifndef JPEG2RAW_H
#define JPEG2RAW_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "status_control_task.h"
#include <stdint.h>


    void jpeg_init( task_handles_t* task_handles );

    void jpeg_task( void* pvParameters );


#ifdef __cplusplus
}
#endif
#endif  // JPEG2RAW_H
