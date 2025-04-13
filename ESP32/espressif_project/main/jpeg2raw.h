//
// Created by skohl on 13.04.2025.
//

#ifndef JPEG2RAW_H
#define JPEG2RAW_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

    struct
    {

    } typedef jpeg_stat_t;

    void jpeg_init( jpeg_stat_t* stat );

    void jpeg_task( void* pvParameters );

    void jpeg_unpack(uint8_t* src, uint8_t* dst, uint32_t in_size, uint32_t out_size);

#ifdef __cplusplus
}
#endif
#endif  // JPEG2RAW_H
