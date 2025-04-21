//
// Created by skohl on 13.04.2025.
//

#include "jpeg2raw.h"
#include "esp_check.h"
#include "esp_log.h"
#include "jpeg_decoder.h"

static const char* TAG = "jpeg";

#define JPEG_WORK_AREA_SIZE 5100
static uint8_t jpec_work_area[ JPEG_WORK_AREA_SIZE ];

struct
{
    task_handles_t* task_handles;
} typedef jpeg_ctrl_t;

jpeg_ctrl_t jpeg_ctrl;

void jpeg_init( task_handles_t* task_handles ) { jpeg_ctrl.task_handles = task_handles; }

static esp_err_t jpeg_unpack( uint8_t* src, uint8_t* dst, uint32_t in_size, uint32_t out_size )
{
    esp_err_t ret;
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata                       = ( uint8_t* ) src,
        .indata_size                  = in_size,
        .outbuf                       = dst,
        .outbuf_size                  = out_size,
        .out_format                   = JPEG_IMAGE_FORMAT_RGB888,
        .out_scale                    = JPEG_IMAGE_SCALE_0,
        .advanced.working_buffer      = &jpec_work_area[ 0 ],
        .advanced.working_buffer_size = JPEG_WORK_AREA_SIZE
        // .flags = {
        //     .swap_color_bytes = 1,
        // }
    };

    esp_jpeg_image_output_t outimg;

    ret = esp_jpeg_decode( &jpeg_cfg, &outimg );

    // ESP_LOGI(TAG , "out pic w: %"PRIu16" h: %"PRIu16, outimg.width, outimg.height);
    return ret;
}

void jpeg_task( void* pvParameters )
{
    uint32_t ulNotifiedValue;
    eth_rx_buffer_t* src_ptr;
    frame_unpacked_t* dst_ptr;
    esp_err_t jpeg_ret;

    if ( JPEG_TASK_VERBOSE ) ESP_LOGI( TAG, "Enter Loop" );
    while ( 1 )
    {
        if ( jpeg_ctrl.task_handles->status_control_task_handle == NULL )
        {
            vTaskDelay( 1 );
            continue;
        }
        /* Wait until triggered by Ctrl
         * Dont clear on Entry, (we may have work to do already)
         * Clear on Exit
         */
        xTaskNotifyWaitIndexed( TASK_NOTIFY_JPEG_START_BIT, pdFALSE, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY );
        if ( JPEG_TASK_VERBOSE ) ESP_LOGI( TAG, "Start JPEG Conversion" );

        /* Get current Buffer ptr */
        src_ptr = buff_ctrl_get_jpeg_src();
        if ( src_ptr == NULL || src_ptr->data_size == 0 )
        {
            /* http task failed to receive in this buffer-> do not calc */
            jpeg_ret = ESP_FAIL;
            ESP_LOGE( TAG, "No Ptr" );
        }
        else
        {
            dst_ptr  = buff_ctrl_get_jpeg_dst();
            /* Decompress JPEG. Note: jpeg must be in YCrCb Colorspace */
            jpeg_ret = jpeg_unpack( src_ptr->buff_start_ptr, dst_ptr->frame_start_ptr, src_ptr->data_size, dst_ptr->total_size );
            if ( jpeg_ret != ESP_OK )
            {
                ESP_LOGE( TAG, "JPEG Conversion Failed" );
            }
        }

        if ( jpeg_ret != ESP_OK )
        {
            buff_ctrl_set_jpec_dst_done( 0 );
            ESP_LOGE( TAG, "JPEG Conversion Failed" );
        }
        else buff_ctrl_set_jpec_dst_done( 1 );

        /* Done. Notify Ctrl */
        xTaskNotifyIndexed( jpeg_ctrl.task_handles->status_control_task_handle, TASK_NOTIFY_CTRL_JPEG_FINISHED_BIT, 0, eSetBits );
    }
}
