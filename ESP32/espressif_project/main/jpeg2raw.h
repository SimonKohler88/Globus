//
// Created by skohl on 13.04.2025.
//

#ifndef JPEG2RAW_H
#define JPEG2RAW_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "pic_buffer.h"
#include "status_control_task.h"
#include <stdint.h>

    struct
    {

    } typedef jpeg_stat_t;

    void jpeg_init( jpeg_stat_t* stat, task_handles_t* task_handles );

    void jpeg_task( void* pvParameters );


#ifdef __cplusplus
}
#endif
#endif  // JPEG2RAW_H
