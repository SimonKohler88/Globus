//
// Created by skohl on 13.04.2025.
//

#include "http_task.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include <stdint.h>
#include <string.h>

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "sdkconfig.h"

#include "hw_settings.h"
#include "pic_buffer.h"

#define WEB_SERVER CONFIG_WIFI_IPV4_ADDR
#define WEB_PORT   HTTP_PORT
#define WEB_PATH   HTTP_PATH

static const char* TAG = "http";

static char request_buffer[ 100 ];

struct
{
    struct addrinfo hints;
    struct addrinfo* res;
    struct in_addr* addr;
    int s, r;
    char recv_buf[ 1024 ];
    task_handles_t* task_handles;
} typedef http_stat_t;

static http_stat_t http_stat;

void init_http_stat( task_handles_t* task_handles )
{
    http_stat.task_handles      = task_handles;
    http_stat.hints.ai_family   = AF_INET;
    http_stat.hints.ai_socktype = SOCK_STREAM;
}

static uint8_t lookup_dns( http_stat_t* stat )
{
    int err = getaddrinfo( WEB_SERVER, WEB_PORT, &stat->hints, &stat->res );

    if ( err != 0 || stat->res == NULL )
    {
        ESP_LOGE( TAG, "DNS lookup failed err=%d res=%p", err, stat->res );
        return 0;
    }
    return 1;
}
static uint8_t create_socket( http_stat_t* stat )
{
    stat->s = socket( stat->res->ai_family, stat->res->ai_socktype, 0 );
    if ( stat->s < 0 )
    {
        ESP_LOGE( TAG, "Failed to allocate socket." );
        return 0;
    }
    return 1;
}
static uint8_t connect_socket( http_stat_t* stat )
{
    if ( connect( stat->s, stat->res->ai_addr, stat->res->ai_addrlen ) != 0 )
    {
        ESP_LOGE( TAG, "socket connect failed errno=%i", errno );

        return 0;
    }
    return 1;
}

static uint8_t send_request( http_stat_t* stat )
{
    if ( write( stat->s, request_buffer, strlen( request_buffer ) ) < 0 )
    {
        ESP_LOGE( TAG, "socket send failed" );
        return 0;
    }
    return 1;
}

static uint8_t set_socket_timeout( http_stat_t* stat )
{
    struct timeval receiving_timeout;
    receiving_timeout.tv_sec  = 0;
    receiving_timeout.tv_usec = 400000;
    if ( setsockopt( stat->s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof( receiving_timeout ) ) < 0 )
    {
        ESP_LOGE( TAG, "failed to set socket receiving timeout" );
        return 0;
    }
    return 1;
}

static uint32_t receive_frame( http_stat_t* stat, eth_rx_buffer_t* eth_buff )
{
    uint32_t data_size     = 0;
    uint8_t* buff_ptr      = eth_buff->buff_start_ptr;
    /* Read HTTP response */
    do
    {
        bzero( stat->recv_buf, sizeof( stat->recv_buf ) );
        stat->r = read( stat->s, stat->recv_buf, sizeof( stat->recv_buf ) - 1 );
        data_size += stat->r;
        if ( data_size > IMAGE_JPEG_SIZE_BYTES )
        {
            ESP_LOGE( TAG, "File size More than %" PRIu32, ( uint32_t ) IMAGE_JPEG_SIZE_BYTES );
            return 0;
        }
        /* last sanity check */
        if (buff_ptr == NULL || stat->r > sizeof( stat->recv_buf ) - 1)
        {
            ESP_LOGE( TAG, "Sanity fail: bf_ptr: 0x%" PRIx32", r: %i", ( uint32_t ) buff_ptr, stat->r );
            return 0;
        }
        if (stat->r > 0) memcpy( buff_ptr, stat->recv_buf, stat->r );
        buff_ptr += stat->r;


    } while ( stat->r > 0 );
    return data_size;
}

void http_task( void* pvParameters )
{
    /* Task to get a frame by making a GET request to server.
     * It is triggered by TaskNotify.
     * Will Notify QSPI task
     */
    uint8_t ret         = 0;
    uint32_t data_size  = 0;
    uint32_t time_start = 0;

    uint32_t last_frame_time_used;

    eth_rx_buffer_t* eth_buff;

    while ( 1 )
    {
        /* We have to go through DNS lookup and socket creation per Request.
         * Does not seem to work otherwise, but does not affect speed very much (it does) */
        // TODO: make socket static?

        /*  Wait until notified by Ctrl
         *  Clear on Entry
         *  Clear on Exit
         */
        if ( HTTP_TASK_VERBOSE ) ESP_LOGI( TAG, "HTTP Req Start Waiting" );
        xTaskNotifyWaitIndexed( TASK_NOTIFY_HTTP_START_BIT, ULONG_MAX, ULONG_MAX, &last_frame_time_used, portMAX_DELAY );
        if ( HTTP_TASK_VERBOSE ) ESP_LOGI( TAG, "HTTP Req Start: dT %" PRIu32, last_frame_time_used );

        /* Get Buffer */
        eth_buff = buff_ctrl_get_eth_buff();

        /* Notify JPEG Task to start conversion */
        xTaskNotifyIndexed( http_stat.task_handles->JPEG_task_handle, TASK_NOTIFY_JPEG_START_BIT, 0, eSetBits );

        time_start = xTaskGetTickCount();

        ret = lookup_dns( &http_stat );
        if ( !ret )
        {
            vTaskDelay( pdMS_TO_TICKS( 10 ) );
        }

        if ( ret )
        {
            /* Print the resolved IP. */
            http_stat.addr = &( ( struct sockaddr_in* ) http_stat.res->ai_addr )->sin_addr;

            ret = create_socket( &http_stat );
            if ( !ret )
            {
                vTaskDelay( pdMS_TO_TICKS( 10 ) );
            }
        }
        if ( ret )
        {
            ret = connect_socket( &http_stat );
            if ( !ret )
            {
                vTaskDelay( pdMS_TO_TICKS( 10 ) );
            }
        }
        freeaddrinfo( http_stat.res );  //?
        if ( ret )
        {
            snprintf( request_buffer, 88,
                      "GET " WEB_PATH "/%" PRIu32 " HTTP/1.0\r\nHost: " WEB_SERVER ":" WEB_PORT "\r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n",
                      last_frame_time_used );

            ret = send_request( &http_stat );
            if ( !ret )
            {
                vTaskDelay( pdMS_TO_TICKS( 10 ) );
            }
        }

        if ( ret )
        {
            ret = set_socket_timeout( &http_stat );
            if ( !ret )
            {
                vTaskDelay( pdMS_TO_TICKS( 10 ) );
            }
        }

        data_size = 0;
        if ( ret )
        {
            data_size = receive_frame( &http_stat, eth_buff );
            if ( !data_size )
            {
                ret = 0;
                vTaskDelay( pdMS_TO_TICKS( 10 ) );
            }
        }
        close( http_stat.s );

        uint32_t time = pdTICKS_TO_MS( xTaskGetTickCount() - time_start );

        /* set buffer valid if more than 0 data received */
        buff_ctrl_set_eth_buff_done( data_size );

        if ( HTTP_TASK_VERBOSE ) ESP_LOGI( TAG, "http time=%" PRIu32, time );

        // Start QSPI Task
        xTaskNotifyIndexed( http_stat.task_handles->status_control_task_handle, TASK_NOTIFY_CTRL_HTTP_FINISHED_BIT, data_size, eSetBits );
    }
}
