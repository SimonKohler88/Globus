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

#define WEB_SERVER CONFIG_WIFI_IPV4_ADDR
#define WEB_PORT   HTTP_PORT
#define WEB_PATH   HTTP_PATH

static const char* TAG = "http";

static const char* REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER ":" WEB_PORT "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";
// struct
// {
//
// };

struct
{
    // const struct addrinfo hints = {
    //     .ai_family   = AF_INET,
    //     .ai_socktype = SOCK_STREAM,
    // };
    struct addrinfo hints;
    struct addrinfo* res;
    struct in_addr* addr;
    int s, r;
    char recv_buf[ 1024 ];
} typedef http_stat_t;

static http_stat_t http_stat;


void init_http_stat( void )
{
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
        close( stat->s );
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
        close( stat->s );
        return 0;
    }
    return 1;
}

static uint32_t receive_frame( http_stat_t* stat )
{

    //        ESP_LOGI(TAG, "... set socket receiving timeout success");

    uint32_t data_size = 0;
    /* Read HTTP response */
    do
    {
        bzero( stat->recv_buf, sizeof( stat->recv_buf ) );
        stat->r = read( stat->s, stat->recv_buf, sizeof( stat->recv_buf ) - 1 );
        data_size += stat->r;
        //            for(int i = 0; i < r; i++) {
        //                putchar(recv_buf[i]);
        //            }
    } while ( stat->r > 0 );
    return data_size;
}

void http_task( void* pvParameters )
{

    uint8_t ret = 0;
    uint32_t data_size = 0;

    while ( 1 )
    {

        ret = lookup_dns( &http_stat );
        if ( !ret )
        {
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
            continue;
        }

        /* Print the resolved IP. */
        http_stat.addr = &( ( struct sockaddr_in* ) http_stat.res->ai_addr )->sin_addr;
        ESP_LOGI( TAG, "DNS lookup succeeded. IP=%s", inet_ntoa( *http_stat.addr ) );

        ret = create_socket( &http_stat );
        if ( !ret )
        {
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
            continue;
        }
        ESP_LOGI( TAG, "allocated socket" );

        ret = connect_socket( &http_stat );
        if ( !ret )
        {
            vTaskDelay( 4000 / portTICK_PERIOD_MS );
            continue;
        }

        ESP_LOGI( TAG, "connected" );
        freeaddrinfo( http_stat.res );  //?

        uint32_t time_start = xTaskGetTickCount();

        ret = send_request( &http_stat );
        if ( !ret )
        {
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
            continue;
        }
        ret = set_socket_timeout( &http_stat );
        if ( !ret )
        {
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
            continue;
        }

        data_size = receive_frame( &http_stat );
        if ( !data_size )
        {
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
            continue;
        }

        uint32_t time = ( xTaskGetTickCount() - time_start ) * 10;

        ESP_LOGI( TAG, "done. time=%" PRIu32 ", %" PRIu32, time, data_size );
        close( http_stat.s );
        for ( int countdown = 1; countdown >= 0; countdown-- )
        {
            //            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
        }
        ESP_LOGI( TAG, "Starting again!" );
    }
}
