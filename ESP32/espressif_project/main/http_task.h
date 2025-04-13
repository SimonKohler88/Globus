//
// Created by skohl on 13.04.2025.
//

#ifndef HTTP_TASK_H
#define HTTP_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

    void init_http_stat( void );
    void http_task( void* pvParameters );
#ifdef __cplusplus
}
#endif
#endif  // HTTP_TASK_H
