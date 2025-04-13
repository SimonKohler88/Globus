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
#include <stdint.h>
#include <string.h>

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "sdkconfig.h"

#include "hw_settings.h"
#include "jpeg2raw.h"

/* gpio access for debugging purposes */
#include "status_control_task.h"


#define WEB_SERVER CONFIG_WIFI_IPV4_ADDR
#define WEB_PORT   HTTP_PORT
#define WEB_PATH   HTTP_PATH

static const char* TAG = "http";

static const char* REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER ":" WEB_PORT "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";
/* internal Buffer for JPEG */
static char jpeg_buffer[ IMAGE_JPEG_SIZE_BYTES ];

struct
{
    struct addrinfo hints;
    struct addrinfo* res;
    struct in_addr* addr;
    int s, r;
    char recv_buf[ 1024 ];
    uint8_t* psram_buffer;
} typedef http_stat_t;

static http_stat_t http_stat;

void init_http_stat( uint8_t* psram_ptr )
{
    http_stat.psram_buffer      = psram_ptr;
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
        freeaddrinfo( stat->res );
        return 0;
    }
    return 1;
}
static uint8_t connect_socket( http_stat_t* stat )
{
    if ( connect( stat->s, stat->res->ai_addr, stat->res->ai_addrlen ) != 0 )
    {
        ESP_LOGE( TAG, "socket connect failed errno=%d", errno );
        close( stat->s );
        freeaddrinfo( stat->res );

        return 0;
    }
    return 1;
}

static uint8_t send_request( http_stat_t* stat )
{
    if ( write( stat->s, REQUEST, strlen( REQUEST ) ) < 0 )
    {
        ESP_LOGE( TAG, "socket send failed" );
        return 0;
    }
    return 1;
}

static uint8_t set_socket_timeout( http_stat_t* stat )
{
    struct timeval receiving_timeout;
    receiving_timeout.tv_sec  = 5;
    receiving_timeout.tv_usec = 0;
    if ( setsockopt( stat->s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof( receiving_timeout ) ) < 0 )
    {
        ESP_LOGE( TAG, "failed to set socket receiving timeout" );
        return 0;
    }
    return 1;
}

static uint32_t receive_frame( http_stat_t* stat )
{

    //        ESP_LOGI(TAG, "... set socket receiving timeout success");

    uint32_t data_size = 0;
    char* buffer_ptr   = &jpeg_buffer[ 0 ];
    /* Read HTTP response */
    do
    {
        bzero( stat->recv_buf, sizeof( stat->recv_buf ) );
        stat->r = read( stat->s, stat->recv_buf, sizeof( stat->recv_buf ) - 1 );
        memcpy( buffer_ptr, stat->recv_buf, stat->r );
        buffer_ptr += stat->r;
        data_size += stat->r;
        // Todo: copy to RAMbuffer
    } while ( stat->r > 0 );
    return data_size;
}

void http_task( void* pvParameters )
{
    /* Task to get a frame by making a GET request to server.
     * It is triggered by TaskNotify.
     * Will Notify Jpeg task
     */
    uint8_t ret        = 0;
    uint32_t data_size = 0;
    uint32_t time_start =0;
    set_gpio_reserve_1_async(0);

    while ( 1 )
    {
        /* We have to go through DNS lookup and socket creation per Request.
         * Does not seem to work otherwise, but does not affect speed very much */
        set_gpio_reserve_1_async(1);
        time_start = xTaskGetTickCount();
        ret                 = lookup_dns( &http_stat );
        if ( !ret )
        {
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
            continue;
        }

        /* Print the resolved IP. */
        http_stat.addr = &( ( struct sockaddr_in* ) http_stat.res->ai_addr )->sin_addr;

        ret = create_socket( &http_stat );
        if ( !ret )
        {
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
            continue;
        }

        ret = connect_socket( &http_stat );
        if ( !ret )
        {
            vTaskDelay( 4000 / portTICK_PERIOD_MS );
            continue;
        }

        // ESP_LOGI( TAG, "connected" );
        freeaddrinfo( http_stat.res );  //?

        ret = send_request( &http_stat );
        if ( !ret )
        {
            continue;
        }
        ret = set_socket_timeout( &http_stat );
        if ( !ret )
        {
            continue;
        }

        data_size = receive_frame( &http_stat );
        if ( !data_size )
        {
            ret = 0;
            continue;
        }

        close( http_stat.s );
        // uint32_t time = ( xTaskGetTickCount() - time_start ) * 10;
        // ESP_LOGI( TAG, "done. time=%" PRIu32 ", %" PRIu32, time, data_size );

        // time_start = xTaskGetTickCount();
        // set_gpio_reserve_1_async(1);
        // set_gpio_reserve_1_async(0);
        jpeg_unpack( ( uint8_t* ) &jpeg_buffer[ 0 ], http_stat.psram_buffer, data_size, IMAGE_TOTAL_BYTE_SIZE );
        set_gpio_reserve_1_async(0);
        uint32_t time = pdTICKS_TO_MS( xTaskGetTickCount() - time_start );

        ESP_LOGI( TAG, "jpeg. time=%" PRIu32, time );
        for ( int countdown = 1; countdown >= 0; countdown-- )
        {
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
        }

        // vTaskDelay( 1000 / portTICK_PERIOD_MS );
        //
        // ESP_LOGI( TAG, "Starting again!" );
    }
}
