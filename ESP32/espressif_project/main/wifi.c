/*
 * wifi.c
 *
 *  Created on: 1 May 2021
 *      Author: cyril
 *
 *  copied and adapted: 19.10.2024
 *      Author: skohl
 *
 *
 */

#include "wifi.h"

#include "inttypes.h"
#include "pic_buffer.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "hw_settings.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "portmacro.h"
#include "rpi_interface.h"
#include "sdkconfig.h"

// optimization purposes
#include "pic_buffer.h"
#include "hw_settings.h"
#include "status_control_task.h"

// #include "portmacro.h"
// #include "sdkconfig.h"


static task_handles_t* task_handles;
static uint8_t is_wifi_connected = 0;

/*
 * Code copied and adapted from esp-idf's "http_request" template
 *
 *
 *
 */
#define NETIF_DESC_STA "globus_netif_sta"
static esp_netif_t* s_sta_netif               = NULL;
static const char* TAG                        = "Wifi";
static SemaphoreHandle_t s_semph_get_ip_addrs = NULL;
static int s_retry_num                        = 0;

uint8_t wifi_is_connected() { return is_wifi_connected; }

static void wifi_handler_on_wifi_disconnect( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data )
{
    is_wifi_connected = 0;
    s_retry_num++;

    ESP_LOGI( TAG, "Wi-Fi disconnected, trying to reconnect..." );
    esp_err_t err = esp_wifi_connect();
    if ( err == ESP_ERR_WIFI_NOT_STARTED )
    {
        return;
    }
    ESP_ERROR_CHECK( err );
}

static void wifi_handler_on_wifi_connect( void* esp_netif, esp_event_base_t event_base, int32_t event_id, void* event_data )
{
    is_wifi_connected = 1;
}

static bool wifi_is_our_netif( const char* prefix, esp_netif_t* netif )
{
    return strncmp( prefix, esp_netif_get_desc( netif ), strlen( prefix ) - 1 ) == 0;
}

static void wifi_handler_on_sta_got_ip( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data )
{
    s_retry_num              = 0;
    ip_event_got_ip_t* event = ( ip_event_got_ip_t* ) event_data;
    if ( !wifi_is_our_netif( NETIF_DESC_STA, event->esp_netif ) )
    {
        return;
    }
    ESP_LOGI( TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc( event->esp_netif ), IP2STR( &event->ip_info.ip ) );
    if ( s_semph_get_ip_addrs )
    {
        xSemaphoreGive( s_semph_get_ip_addrs );
    }
    else
    {
        ESP_LOGI( TAG, "- IPv4 address: " IPSTR ",", IP2STR( &event->ip_info.ip ) );
    }
}

static esp_err_t wifi_sta_do_connect( wifi_config_t wifi_config, bool wait )
{
    if ( wait )
    {
        s_semph_get_ip_addrs = xSemaphoreCreateBinary();
        if ( s_semph_get_ip_addrs == NULL )
        {
            return ESP_ERR_NO_MEM;
        }
    }
    s_retry_num = 0;
    ESP_ERROR_CHECK( esp_event_handler_register( WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_handler_on_wifi_disconnect, NULL ) );
    ESP_ERROR_CHECK( esp_event_handler_register( IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_handler_on_sta_got_ip, NULL ) );
    ESP_ERROR_CHECK( esp_event_handler_register( WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &wifi_handler_on_wifi_connect, s_sta_netif ) );

    ESP_LOGI( TAG, "Connecting to %s...", wifi_config.sta.ssid );
    ESP_ERROR_CHECK( esp_wifi_set_config( WIFI_IF_STA, &wifi_config ) );
    esp_err_t ret = esp_wifi_connect();
    if ( ret != ESP_OK )
    {
        ESP_LOGE( TAG, "WiFi connect failed! ret:%x", ret );
        return ret;
    }
    if ( wait )
    {
        ESP_LOGI( TAG, "Waiting for IP(s)" );
        xSemaphoreTake( s_semph_get_ip_addrs, portMAX_DELAY );

        if ( s_retry_num > WIFI_CONN_MAX_RETRY )
        {
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

static esp_err_t wifi_connect( void )
{
    ESP_LOGI( TAG, "Initializing WIFI" );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init( &cfg ) );
    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    esp_netif_config.if_desc                     = NETIF_DESC_STA;
    esp_netif_config.route_prio                  = 128;
    s_sta_netif                                  = esp_netif_create_wifi( WIFI_IF_STA, &esp_netif_config );
    esp_wifi_set_default_wifi_sta_handlers();
    ESP_ERROR_CHECK( esp_wifi_set_storage( WIFI_STORAGE_RAM ) );
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    wifi_config_t wifi_config = {
        .sta =
            {
                .ssid               = CONFIG_WIFI_SSID,
                .password           = CONFIG_WIFI_PASSWORD,
                .scan_method        = WIFI_ALL_CHANNEL_SCAN,
                .sort_method        = WIFI_CONNECT_AP_BY_SIGNAL,
                .threshold.rssi     = -127,
                .threshold.authmode = WIFI_AUTH_OPEN,
            },
    };

    return wifi_sta_do_connect( wifi_config, true );
}

esp_err_t wifi_sta_do_disconnect( void )
{
    ESP_ERROR_CHECK( esp_event_handler_unregister( WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_handler_on_wifi_disconnect ) );
    ESP_ERROR_CHECK( esp_event_handler_unregister( IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_handler_on_sta_got_ip ) );
    ESP_ERROR_CHECK( esp_event_handler_unregister( WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &wifi_handler_on_wifi_connect ) );

    if ( s_semph_get_ip_addrs )
    {
        vSemaphoreDelete( s_semph_get_ip_addrs );
    }

    return esp_wifi_disconnect();
}

// static void wifi_stop( void )
// {
//     esp_err_t err = esp_wifi_stop();
//     if ( err == ESP_ERR_WIFI_NOT_INIT )
//     {
//         return;
//     }
//     ESP_ERROR_CHECK( err );
//     ESP_ERROR_CHECK( esp_wifi_deinit() );
//     ESP_ERROR_CHECK( esp_wifi_clear_default_wifi_driver_and_handlers( s_sta_netif ) );
//     esp_netif_destroy( s_sta_netif );
//     s_sta_netif = NULL;
// }

// static void wifi_shutdown( void )
// {
//     wifi_sta_do_disconnect();
//     wifi_stop();
// }

void wifi_receive_init_task( void* pvParameter )
{
    task_handles = ( task_handles_t* ) pvParameter;
    uint32_t notify_value;
    esp_err_t err;

    ESP_ERROR_CHECK( esp_netif_init() );
    ESP_ERROR_CHECK( esp_event_loop_create_default() );
    ESP_LOGI( TAG, "Start Loop" );
    while ( 1 )
    {
        /* Wait until notified by Ctrl
         * No clear on Entry, Clear on Exit
         */
        xTaskNotifyWaitIndexed( TASK_NOTIFY_WIFI_START_BIT, pdFALSE, ULONG_MAX, &notify_value, portMAX_DELAY );
        ESP_LOGI( TAG, "Connecting wifi" );

        err = wifi_connect();
        if ( err != ESP_OK )
        {
            ESP_LOGE( TAG, "Could not connect to Wifi" );
        }
        // ESP_ERROR_CHECK( esp_register_shutdown_handler( &wifi_shutdown ) );
        ESP_LOGI( TAG, "Wifi initializing done" );
        xTaskNotifyIndexed( task_handles->status_control_task_handle, TASK_NOTIFY_CTRL_WIFI_FINISHED_BIT, err == ESP_OK, eSetBits );

    }
}
