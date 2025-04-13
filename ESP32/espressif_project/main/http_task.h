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

    void init_http_stat( uint8_t* psram_ptr );
    void http_task( void* pvParameters );
#ifdef __cplusplus
}
#endif
#endif  // HTTP_TASK_H
