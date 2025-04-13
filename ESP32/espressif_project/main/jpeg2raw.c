//
// Created by skohl on 13.04.2025.
//

#include "jpeg2raw.h"
#include "jpeg_decoder.h"
#include "esp_log.h"
#include "esp_check.h"

static const char* TAG = "jpeg";

#define JPEG_WORK_AREA_SIZE 5100
static uint8_t jpec_work_area[JPEG_WORK_AREA_SIZE];

void jpeg_init( jpeg_stat_t* stat ){}

void jpeg_task( void* pvParameters ){}

void jpeg_unpack(uint8_t* src, uint8_t* dst, uint32_t in_size, uint32_t out_size)
{
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = (uint8_t *)src,
        .indata_size = in_size,
        .outbuf = dst,
        .outbuf_size = out_size,
        .out_format = JPEG_IMAGE_FORMAT_RGB888,
        .out_scale = JPEG_IMAGE_SCALE_0,
        .advanced.working_buffer = &jpec_work_area[0],
        .advanced.working_buffer_size = JPEG_WORK_AREA_SIZE
        // .flags = {
        //     .swap_color_bytes = 1,
        // }
    };

    esp_jpeg_image_output_t outimg;

    esp_jpeg_decode(&jpeg_cfg, &outimg);

    // ESP_LOGI(TAG , "out pic w: %"PRIu16" h: %"PRIu16, outimg.width, outimg.height);
}
