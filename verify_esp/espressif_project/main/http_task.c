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

#define WEB_SERVER CONFIG_WIFI_IPV4_ADDR
#define WEB_PORT   HTTP_PORT
#define WEB_PATH   HTTP_PATH

static const char* TAG = "http";

static struct sockaddr_in dest_addr;
static int UDP_socket;

static uint8_t tx_buffer[ QSPI_MAX_TRANSFER_SIZE ];
static uint8_t tx_buffer_test[ QSPI_MAX_TRANSFER_SIZE ];

struct
{
    uint8_t spi1[ QSPI_MAX_TRANSFER_SIZE / 4 + 1 ];
    uint8_t spi2[ QSPI_MAX_TRANSFER_SIZE / 4 + 1 ];
    uint8_t spi3[ QSPI_MAX_TRANSFER_SIZE / 4 + 1 ];
    uint8_t spi4[ QSPI_MAX_TRANSFER_SIZE / 4 + 1 ];
} typedef spi_buffer_t;

static spi_buffer_t spi_buffer;

void copy_buffer( uint8_t* src, uint32_t size )
{
    if ( src == NULL || size <= 0 || size >= QSPI_MAX_TRANSFER_SIZE ) return;
    memcpy( tx_buffer, src, size );
}

bool wifi_send_packet_raw( uint8_t* data_ptr, uint32_t size )
{
    if ( UDP_socket < 0 ) return false;
    int err = sendto( UDP_socket, data_ptr, size, 0, ( struct sockaddr* ) &dest_addr, sizeof( dest_addr ) );
    if ( err < 0 )
    {

        ESP_LOGE( "WIFI", "Error occurred during sending: err%i, errno %i", err, errno );
        return false;
    }

    return true;
}

void convert( uint8_t* src, uint16_t size_in, spi_buffer_t* dst )
{
    /* Split single SPI data out of QSPI nibbles */
    uint8_t data_spi1 = 0;
    uint8_t data_spi2 = 0;
    uint8_t data_spi3 = 0;
    uint8_t data_spi4 = 0;

    uint8_t data1 = 0;
    uint8_t data2 = 0;
    uint8_t data3 = 0;
    uint8_t data4 = 0;

    uint8_t k    = 0;
    uint8_t data = 0;
    uint8_t rest;
    for ( uint16_t i = 0; i < size_in; i++ )
    {
        data = src[ i ];

        data_spi1 |= ( data & ( 1 << 7 ) ) << 0;
        data_spi1 |= ( data & ( 1 << 3 ) ) << 3;

        data_spi2 |= ( data & ( 1 << 6 ) ) << 1;
        data_spi2 |= ( data & ( 1 << 2 ) ) << 4;

        data_spi3 |= ( data & ( 1 << 5 ) ) << 2;
        data_spi3 |= ( data & ( 1 << 1 ) ) << 5;

        data_spi4 |= ( data & ( 1 << 4 ) ) << 3;
        data_spi4 |= ( data & ( 1 << 0 ) ) << 6;

        rest = i % 4;

        switch ( rest )
        {
            case 0 :
                data1 = 0;
                data2 = 0;
                data3 = 0;
                data4 = 0;
                data1 |= data_spi1 >> 0;
                data2 |= data_spi2 >> 0;
                data3 |= data_spi3 >> 0;
                data4 |= data_spi4 >> 0;
                break;
            case 1 :
                data1 |= data_spi1 >> 2;
                data2 |= data_spi2 >> 2;
                data3 |= data_spi3 >> 2;
                data4 |= data_spi4 >> 2;
                break;
            case 2 :
                data1 |= data_spi1 >> 4;
                data2 |= data_spi2 >> 4;
                data3 |= data_spi3 >> 4;
                data4 |= data_spi4 >> 4;
                break;
            case 3 :
                data1 |= data_spi1 >> 6;
                data2 |= data_spi2 >> 6;
                data3 |= data_spi3 >> 6;
                data4 |= data_spi4 >> 6;

                dst->spi1[ k ] = data1;
                dst->spi2[ k ] = data2;
                dst->spi3[ k ] = data3;
                dst->spi4[ k ] = data4;

                k++;
                break;
        }
    }
}

void http_task( void* pvParameters )
{

    char host_ip[]  = CONFIG_WIFI_IPV4_ADDR;
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    dest_addr.sin_family      = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr( CONFIG_WIFI_IPV4_ADDR );
    dest_addr.sin_port        = htons( CONFIG_UDP_PORT );

    uint8_t ret;

    for ( uint16_t i = 0; i < QSPI_MAX_TRANSFER_SIZE; i++ )
    {
        tx_buffer_test[ i ] = ( uint8_t ) i;
    }

    xTaskNotifyWaitIndexed( TASK_NOTIFY_WIFI_READY_BIT, ULONG_MAX, ULONG_MAX, NULL, portMAX_DELAY );

    UDP_socket = socket( addr_family, SOCK_DGRAM, ip_protocol );

    while ( 1 )
    {
        /*  Wait until notified by Ctrl
         *  Clear on Entry
         *  Clear on Exit
         */
        if ( HTTP_TASK_VERBOSE ) ESP_LOGI( TAG, "HTTP Req Start Waiting" );
#ifdef DEVELOPMENT_HTTP_NOTIFY_TIMEOUT
        ret = xTaskNotifyWaitIndexed( TASK_NOTIFY_HTTP_START_BIT, ULONG_MAX, ULONG_MAX, NULL, 10 );
#else
        xTaskNotifyWaitIndexed( TASK_NOTIFY_HTTP_START_BIT, ULONG_MAX, ULONG_MAX, NULL, portMAX_DELAY );
#endif
        if ( HTTP_TASK_VERBOSE ) ESP_LOGI( TAG, "HTTP Req Start" );
#ifdef DEVELOPMENT_HTTP_NOTIFY_TIMEOUT

        if ( ret != pdTRUE )
        {
            if ( HTTP_TASK_VERBOSE ) ESP_LOGI( TAG, "Test Pack" );
            convert( &tx_buffer_test[ 0 ], QSPI_MAX_TRANSFER_SIZE, &spi_buffer );
            wifi_send_packet_raw( ( uint8_t* ) &spi_buffer, sizeof( spi_buffer ) );
        }
        else
        {
#endif
            convert( &tx_buffer[ 0 ], QSPI_MAX_TRANSFER_SIZE, &spi_buffer );
            wifi_send_packet_raw( ( uint8_t* ) &spi_buffer, sizeof( spi_buffer ) );
#ifdef DEVELOPMENT_HTTP_NOTIFY_TIMEOUT
        }
#endif
    }
}
