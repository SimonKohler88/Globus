//
// Created by skohl on 13.04.2025.
//

#include "jpeg2raw.h"

#include "esp_check.h"
#include "esp_log.h"
#include "jpeg_decoder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "pic_buffer.h"
#include "psram_fifo.h"

static const char* TAG = "jpeg";

#define JPEG_WORK_AREA_SIZE   ( 4100 )
static uint8_t jpec_work_area[ JPEG_WORK_AREA_SIZE ];

struct
{
    task_handles_t* task_handles;
} typedef jpeg_ctrl_t;

jpeg_ctrl_t jpeg_ctrl;

void jpeg_init( task_handles_t* task_handles ) { jpeg_ctrl.task_handles = task_handles; }

static esp_err_t jpeg_unpack( uint8_t* src, uint8_t* dst, uint32_t in_size, uint32_t out_size )
{

    assert(src != NULL);
    assert(dst != NULL);
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
    fifo_frame_t* dst_ptr;
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

        jpeg_ret       = ESP_FAIL;
        uint8_t src_ok = 0;
        uint8_t dst_ok = 0;

        /* Get current Buffer ptr */
        src_ptr = buff_ctrl_get_jpeg_src();
        if ( src_ptr == NULL || src_ptr->data_size == 0 )
        {
            /* http task failed to receive in this buffer-> do not calc */
            ESP_LOGE( TAG, "No Src Ptr" );
        }
        else src_ok = 1;

        /* Get Frame Buffer from FIFO */
        if ( src_ok )
        {
            dst_ptr = fifo_get_free_frame();

            if ( dst_ptr == NULL )
            {
                ESP_LOGE( TAG, "No FIFO frame" );
            }
            else dst_ok = 1;
        }

        if ( src_ok && dst_ok )
        {
            /* Decompress JPEG. Note: jpeg must be in YCrCb Colorspace */
            jpeg_ret = jpeg_unpack( src_ptr->buff_start_ptr, dst_ptr->frame_start_ptr, src_ptr->data_size, dst_ptr->total_size );

            if ( jpeg_ret != ESP_OK )
            {
                ESP_LOGE( TAG, "JPEG Conversion Failed" );
            }
            else if ( JPEG_TASK_VERBOSE ) ESP_LOGI( TAG, "JPEG Conversion OK" );
        }

        if ( dst_ok && jpeg_ret != ESP_OK )
        {
            fifo_return_free_frame();
        }
        else if ( jpeg_ret == ESP_OK ) fifo_mark_free_frame_done();

        /* Done. Notify Ctrl */
        xTaskNotifyIndexed( jpeg_ctrl.task_handles->status_control_task_handle, TASK_NOTIFY_CTRL_JPEG_FINISHED_BIT, 0, eSetBits );
    }
}
