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

#include "PSRAM_FIFO.h"
#include "inttypes.h"

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
#include "status_control_task.h"

#define WIFI_CONNECTED_BIT             BIT0
#define WIFI_FAIL_BIT                  BIT1
#define WIFI_ACK_MESSAGE               "OK"
#define WIFI_NACK_MESSAGE              "NOK"
#define WIFI_LED_PACKET_IDENTIFIER     ( 0x44 )  //"D"

#define WIFI_REQUEST_FRAME_MESSAGE     "FRAME"

#define WIFI_CONTROL_IDENTIFIER        ( 0x43 )  //"C"
#define WIFI_CONTROL_STATUS_IDENTIFIER ( 0x53 )  //"S"
#define WIFI_CONTROL_PARAM_IDENTIFIER  ( 0x50 )  //"P"

#define WIFI_SENSOR_BRIGHTNESS_KEYWORD "BRIGHT"
#define WIFI_DEBUG_PACKET_IDENTIFIER   "D"

#define WIFI_TX_CMD_QUEUE_SIZE         5

void wifi_receive_event_handler( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data );
bool wifi_send_packet( char* message );
bool wifi_receive_packet();
bool wifi_receive_data_packet( void );
// bool wifi_receive_LED_packet(void);

bool wifi_receive_control_packet( void );

static bool wifi_send_tftp_ack( uint16_t block_nr, uint16_t port );
static bool wifi_send_tftp_err( uint8_t error_code, uint16_t port );

// static EventGroupHandle_t s_wifi_event_group;

esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;
// static uint8_t wifi_connected = false;

// static int s_retry_num = 0;
static uint8_t rx_buffer[ HW_SETTINGS_UDP_PACKET_SIZE ];
static uint8_t tx_buffer[ HW_SETTINGS_UDP_PACKET_SIZE ];

static struct sockaddr_in dest_addr;
static struct sockaddr_in dest_addr_tftp;

static WIFI_STAT_INTERNAL_t wifi_stat;

static TaskHandle_t wifi_task_handle      = NULL;
static TaskHandle_t wifi_send_task_handle = NULL;

enum
{
    WIFI_TX_SEND_FRAME_REQUEST,
    WIFI_TX_SEND_STATUS
};
typedef uint8_t SEND_TASK_COMMAND_t;

StaticQueue_t xQueueBuffer_send_task;
uint8_t send_task[ UDP_TX_NUMBER_OF_CMD * sizeof( SEND_TASK_COMMAND_t ) ];
QueueHandle_t send_task_cmd_queue_handle;

void wifi_receive_event_handler( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data )
{
    if ( event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START )
    {
        esp_wifi_connect();
        wifi_stat.wifi_connected = false;
    }
    else if ( event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED )
    {
        esp_wifi_connect();
        wifi_stat.wifi_connected = false;
        wifi_stat.s_retry_num++;
        if ( HW_SETTINGS_DEBUG )
        {
            ESP_LOGI( "WIFI", "retry to connect to the AP" );
        }
    }
    else if ( event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP )
    {
        ip_event_got_ip_t* event = ( ip_event_got_ip_t* ) event_data;

        wifi_stat.wifi_connected = true;
        if ( HW_SETTINGS_DEBUG )
        {
            ESP_LOGI( "WIFI", "got ip:" IPSTR, IP2STR( &event->ip_info.ip ) );
        }
        wifi_stat.s_retry_num = 0;
    }
}

void wifi_receive_init( void )
{
    wifi_stat.UDP_socket = -1;

    ESP_ERROR_CHECK( esp_netif_init() );

    ESP_ERROR_CHECK( esp_event_loop_create_default() );
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init( &cfg ) );

    ESP_ERROR_CHECK( esp_event_handler_instance_register( WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_receive_event_handler, NULL, &instance_any_id ) );
    ESP_ERROR_CHECK( esp_event_handler_instance_register( IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_receive_event_handler, NULL, &instance_got_ip ) );

    wifi_config_t wifi_config = {
        .sta =
            {
                .ssid               = CONFIG_WIFI_SSID,
                .password           = CONFIG_WIFI_PASSWORD,
                /* Setting a password implies station will connect to all
                 * security modes including WEP/WPA. However these modes are
                 * deprecated and not advisable to be used. Incase your Access
                 * point doesn't support WPA2, these mode can be enabled by
                 * commenting below line */
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,

                .pmf_cfg = { .capable = true, .required = false },
            },
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
    ESP_ERROR_CHECK( esp_wifi_set_config( ESP_IF_WIFI_STA, &wifi_config ) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    if ( HW_SETTINGS_DEBUG )
    {
        ESP_LOGI( "WIFI", "wifi_init_sta finished." );
    }

    dest_addr.sin_family      = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr( CONFIG_WIFI_IPV4_ADDR );
    dest_addr.sin_port        = htons( CONFIG_UDP_PORT );

    dest_addr_tftp.sin_family      = AF_INET;
    dest_addr_tftp.sin_addr.s_addr = inet_addr( CONFIG_WIFI_IPV4_ADDR );
    dest_addr_tftp.sin_port        = htons( 49600 );
}

uint8_t wifi_is_connected() { return wifi_stat.wifi_connected; }

bool wifi_send_packet( char* message )
{
    if ( !wifi_stat.wifi_connected ) return false;

    int err = sendto( wifi_stat.UDP_socket, message, strlen( message ), 0, ( struct sockaddr* ) &dest_addr, sizeof( dest_addr ) );
    if ( err < 0 )
    {
        if ( HW_SETTINGS_DEBUG )
        {
            ESP_LOGE( "WIFI", "Error occurred during sending: errno %d", errno );
        }
        return false;
    }
    //	if(HW_SETTINGS_DEBUG)
    //	{
    //		ESP_LOGI("WIFI", "Message sent");
    //	}

    return true;
}

bool wifi_send_packet_raw( uint8_t* data_ptr, uint32_t size )
{
    int err = sendto( wifi_stat.UDP_socket, data_ptr, size, 0, ( struct sockaddr* ) &dest_addr, sizeof( dest_addr ) );
    if ( err < 0 )
    {
        if ( HW_SETTINGS_DEBUG )
        {
            ESP_LOGE( "WIFI", "Error occurred during sending: errno %d", errno );
        }
        return false;
    }
    if ( HW_SETTINGS_DEBUG )
    {
        // ESP_LOGI("WIFI", "Message sent");
    }

    return true;
}

void wifi_send_udp_task( void* pvParameters )  // todo
{
    /* Not for all packets, but for those who must be resent if not worked */
    send_task_cmd_queue_handle =
        xQueueCreateStatic( WIFI_TX_CMD_QUEUE_SIZE, sizeof( SEND_TASK_COMMAND_t ), &send_task[ 0 ], &xQueueBuffer_send_task );

    // uint8_t ret;
    BaseType_t xReturn;
    SEND_TASK_COMMAND_t current_command;
    TickType_t xLastWakeTime    = xTaskGetTickCount();
    const TickType_t xPeriod_ms = 10;

    wifi_send_task_handle     = xTaskGetCurrentTaskHandle();
    wifi_stat.wifi_ctrl_state = WIFI_CTRL_IDLE;
    uint32_t proc_counter     = 0;
    uint32_t notify_value     = 0;
    uint8_t num_free_frames   = 0;

    while ( 1 )
    {

        switch ( wifi_stat.wifi_ctrl_state )
        {
            case WIFI_CTRL_IDLE :
            {
                if ( ( !wifi_stat.wifi_connected ) || ( wifi_stat.UDP_socket < 0 ) ) break;

                num_free_frames = fifo_has_free_frame();
                if ( xTaskNotifyWaitIndexed( 0, ULONG_MAX, ULONG_MAX, &notify_value, 0 ) )
                {
                    /* Received first Block in between timeout and now... buffer is aquired and Transfer is in progress */
                    if ( notify_value == WIFI_FIRST_PACKET_RECEIVED )
                    {
                        wifi_stat.wifi_ctrl_state = WIFI_CTRL_DATA_TRANSFER;
                        proc_counter              = 0;
                    }
                }
                else if ( fifo_has_free_frame() )
                {
                    /* No Messages from UDP RX, free frames available -> send request */
                    ESP_LOGI( "WIFI", "nr fframes  %" PRIu8, num_free_frames );
                    wifi_send_packet( WIFI_REQUEST_FRAME_MESSAGE );
                    wifi_stat.wifi_ctrl_state = WIFI_CTRL_WAIT_FIRST_PKT;
                    proc_counter              = 0;
                }

                break;
            }

            case WIFI_CTRL_WAIT_FIRST_PKT :
            {
                if ( xTaskNotifyWaitIndexed( 0, ULONG_MAX, ULONG_MAX, &notify_value, 0 ) )
                {
                    ESP_LOGI( "WIFI", "udp send wait first" );
                    /* RX Task received first Packet */
                    if ( notify_value == WIFI_FIRST_PACKET_RECEIVED )
                    {
                        wifi_stat.wifi_ctrl_state = WIFI_CTRL_DATA_TRANSFER;
                        proc_counter              = 0;
                    }
                    /* RX Task could not aquire buffer from FIFO */
                    else if ( notify_value == WIFI_DATA_TRANSFER_ERROR )
                    {
                        wifi_stat.wifi_ctrl_state = WIFI_CTRL_IDLE;
                    }
                }
                else proc_counter++;

                /* Waited some time until 1. packet should have arrived... go back and resend request */
                if ( proc_counter >= 100 ) wifi_stat.wifi_ctrl_state = WIFI_CTRL_IDLE;
                break;
            }
            case WIFI_CTRL_DATA_TRANSFER :
            {
                /* Wait until RX Task has finished transfer. Success/Failure does not matter, it will return buffer anyway */
                if ( xTaskNotifyWaitIndexed( 0, ULONG_MAX, ULONG_MAX, &notify_value, 0 ) )
                {
                    if ( notify_value == WIFI_DATA_TRANSFER_COMPLETED || notify_value == WIFI_DATA_TRANSFER_ERROR )
                    {
                        wifi_stat.wifi_ctrl_state = WIFI_CTRL_IDLE;
                    }
                }
                else proc_counter++;

                /* Somehow transfer took longer than 1 sec without message from RX Task... Should not get here */
                if ( proc_counter >= 100 ) wifi_stat.wifi_ctrl_state = WIFI_CTRL_IDLE;
                break;
            }
            default : break;
        }

        xReturn = xQueueReceive( send_task_cmd_queue_handle, &current_command, 0 );
        if ( xReturn == pdTRUE )
        {
            /* Cant send stuff if we are not connected */
            if ( ( !wifi_stat.wifi_connected ) || ( wifi_stat.UDP_socket < 0 ) )
            {
                xQueueSend( send_task_cmd_queue_handle, &current_command, 1 );
            }
            else
            {
                switch ( current_command )
                {
                    case WIFI_TX_SEND_STATUS :
                    {
                        tx_buffer[ 0 ] = WIFI_CONTROL_STATUS_IDENTIFIER;
                        uint32_t size  = get_status_data( tx_buffer + 1 );
                        wifi_send_packet_raw( ( uint8_t* ) tx_buffer, size + 1 );
                        break;
                    }
                    default : break;
                }
            }
        }

        vTaskDelayUntil( &xLastWakeTime, xPeriod_ms );
    }
}

void wifi_receive_udp_task( void* pvParameters )
{
    int addr_family  = AF_INET;
    int ip_protocol  = IPPROTO_IP;
    wifi_task_handle = xTaskGetCurrentTaskHandle();

    while ( 1 )
    {
        if ( wifi_stat.wifi_connected )
        {
            if ( wifi_stat.UDP_socket < 0 ) wifi_stat.UDP_socket = socket( addr_family, SOCK_DGRAM, ip_protocol );
            if ( wifi_stat.UDP_socket < 0 )
            {
                if ( HW_SETTINGS_DEBUG )
                {
                    ESP_LOGE( "WIFI", "Unable to create socket: errno %d", errno );
                }
                break;
            }
            if ( HW_SETTINGS_DEBUG )
            {
                ESP_LOGI( "WIFI", "Socket created, sending to %s:%d", CONFIG_WIFI_IPV4_ADDR, CONFIG_UDP_PORT );
            }
        }

        while ( wifi_stat.wifi_connected )
        {
            if ( !wifi_receive_packet() )
            {
                break;
            }

            taskYIELD();
        }

        if ( wifi_stat.UDP_socket != -1 )
        {
            if ( HW_SETTINGS_DEBUG )
            {
                ESP_LOGE( "WIFI", "Shutting down socket and restarting..." );
            }
            shutdown( wifi_stat.UDP_socket, 0 );
            close( wifi_stat.UDP_socket );
        }
        // what?
        vTaskDelay( 1000 );
    }
    ESP_LOGE( "WIFI", "Delete Recv Task" );
    vTaskDelete( NULL );
}

bool wifi_receive_packet()
{
    static struct sockaddr_in source_addr;
    source_addr.sin_family      = AF_INET;
    source_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    source_addr.sin_port        = htons( CONFIG_UDP_PORT );

    if ( bind( wifi_stat.UDP_socket, ( struct sockaddr* ) &source_addr, sizeof( struct sockaddr_in ) ) == -1 )
    {
        if ( HW_SETTINGS_DEBUG )
        {
            ESP_LOGI( "WIFI", "Binding failed" );
        }
        return false;
    }

    struct iovec iov;
    struct msghdr msg;
    // struct cmsghdr *cmsgtmp;
    u8_t cmsg_buf[ CMSG_SPACE( sizeof( struct in_pktinfo ) ) ];

    iov.iov_base       = rx_buffer;
    iov.iov_len        = sizeof( rx_buffer );
    msg.msg_control    = cmsg_buf;
    msg.msg_controllen = sizeof( cmsg_buf );
    msg.msg_flags      = 0;
    msg.msg_iov        = &iov;
    msg.msg_iovlen     = 1;
    msg.msg_name       = ( struct sockaddr* ) &source_addr;
    msg.msg_namelen    = sizeof( source_addr );

    while ( 1 )
    {

        int len         = recvmsg( wifi_stat.UDP_socket, &msg, 0 );
        uint8_t tftp_ok = 1;
        uint8_t is_ctrl = 0;

        // Error occurred during receiving
        if ( len < 0 )
        {
            if ( HW_SETTINGS_DEBUG )
            {
                ESP_LOGE( "WIFI", "recvfrom failed: errno %d", errno );
            }
            return false;
        }

        is_ctrl = ( ( rx_buffer[ 0 ] == WIFI_CONTROL_IDENTIFIER ) && ( len < 5 ) );
        if ( is_ctrl )
        {
            wifi_receive_control_packet();
        }
        /* TFTP Transfer start request */
        else if ( ( rx_buffer[ 0 ] == ( uint8_t ) 0 ) && ( rx_buffer[ 1 ] == ( uint8_t ) eWriteRequest ) )
        {

            wifi_stat.current_frame_download = fifo_get_free_frame();

            if ( wifi_stat.current_frame_download != NULL )
            {
                ESP_LOGI( "WIFI", "Frame start nr %" PRIu8, wifi_stat.current_frame_download->frame_nr );
                wifi_stat.tftp_block_number = 0;

                wifi_send_tftp_ack( 0, source_addr.sin_port );
                wifi_stat.current_frame_download->size        = 0;
                wifi_stat.current_frame_download->current_ptr = wifi_stat.current_frame_download->frame_start_ptr;
                xTaskNotifyIndexed( wifi_send_task_handle, 0, WIFI_FIRST_PACKET_RECEIVED, eSetBits );
            }
            else
            {
                wifi_send_tftp_err( eFileAlreadyExists, source_addr.sin_port );
                tftp_ok = 0;
            }
        }
        else /* TFTP data transfer */
        {

            // ESP_LOGI( "WIFI", "data opcode  %x %x %x %x",  rx_buffer[ 0 ], rx_buffer[ 1 ], rx_buffer[ 2 ], rx_buffer[ 3 ] );
            if ( wifi_stat.current_frame_download == NULL )
            {
                /* we lost the buffer*/
                wifi_send_tftp_err( eFileNotFound, source_addr.sin_port );
                tftp_ok = 0;
            }
            else
            {
                wifi_stat.tftp_block_number++;
                uint8_t op_code   = ( uint8_t ) ( rx_buffer[ 1 ] );
                uint16_t block_nr = 0;
                block_nr |= rx_buffer[ 2 ] << 8;
                block_nr |= rx_buffer[ 3 ];

                if ( ( op_code == ( uint16_t ) eData ) && ( block_nr == wifi_stat.tftp_block_number ) )
                {
                    uint16_t data_len = len - 4;
                    if ( data_len == 0 && ( block_nr < 90 ) )
                    {
                        wifi_send_tftp_err( eUnknownTransferID, source_addr.sin_port );
                        tftp_ok = 0;
                    }
                    else
                    {
                        wifi_send_tftp_ack( wifi_stat.tftp_block_number, source_addr.sin_port );

                        // ESP_LOGI( "WIFI", "data  %d %d %d", op_code, block_nr, data_len );
                        memcpy( ( void* ) wifi_stat.current_frame_download->current_ptr, ( void* ) &rx_buffer + 4, data_len );

                        wifi_stat.current_frame_download->size += ( data_len );
                        wifi_stat.current_frame_download->current_ptr += ( data_len );

                        if ( data_len != CONFIG_UDP_FRAME_PACKET_SIZE )
                        {
                            /* last packet received */
                            uint32_t size = wifi_stat.current_frame_download->size;
                            ESP_LOGI( "WIFI", "Frame Recv Finished %" PRIu32 " bytes", size );

                            fifo_mark_free_frame_done();
                            wifi_stat.current_frame_download->current_ptr = wifi_stat.current_frame_download->frame_start_ptr;
                            wifi_stat.current_frame_download              = NULL;
                            xTaskNotifyIndexed( wifi_send_task_handle, 0, WIFI_DATA_TRANSFER_COMPLETED, eSetBits );
                        }
                    }
                }
                else
                {
                    wifi_send_tftp_err( eAccessViolation, source_addr.sin_port );
                    tftp_ok = 0;
                }
            }

            if ( tftp_ok == 0 )
            {
                /* An Error Occured. we need to put back the taken frame*/
                fifo_return_free_frame();
                wifi_stat.current_frame_download = NULL;
                xTaskNotifyIndexed( wifi_send_task_handle, 0, WIFI_DATA_TRANSFER_ERROR, eSetBits );
            }
        }
        taskYIELD();
    }
    return true;
}

static bool wifi_send_tftp_ack( uint16_t block_nr, uint16_t port )
{
    set_gpio_reserve_1_async( 1 );
    struct sockaddr_in source_addr;
    source_addr.sin_family      = AF_INET;
    source_addr.sin_addr.s_addr = inet_addr( CONFIG_WIFI_IPV4_ADDR );
    //	source_addr.sin_port = htons( port );
    source_addr.sin_port        = port;

    uint8_t tx_ack[ 4 ];
    uint16_t ack = ( uint8_t ) eAck;
    tx_ack[ 0 ]  = 0;
    tx_ack[ 1 ]  = ack;            // htons( ack );
    tx_ack[ 2 ]  = block_nr >> 8;  // htons( block_nr );
    tx_ack[ 3 ]  = ( uint8_t ) block_nr;

    int err = sendto( wifi_stat.UDP_socket, ( uint8_t* ) tx_ack, sizeof( tx_ack ), 0, ( struct sockaddr* ) &source_addr, sizeof( source_addr ) );

    set_gpio_reserve_1_async( 0 );
    if ( err < 0 )
    {
        if ( HW_SETTINGS_DEBUG )
        {
            ESP_LOGE( "WIFI", "Error occurred during sending: errno %d", errno );
        }
        return false;
    }
    //	if( HW_SETTINGS_DEBUG )
    //	{
    //		ESP_LOGI("WIFI", "Message sent");
    //	}

    return true;
    //	return wifi_send_packet_raw( (uint8_t*) tx_ack,  18 );
}

static bool wifi_send_tftp_err( uint8_t error_code, uint16_t port )
{
    struct sockaddr_in source_addr;
    source_addr.sin_family      = AF_INET;
    source_addr.sin_addr.s_addr = inet_addr( CONFIG_WIFI_IPV4_ADDR );
    //	source_addr.sin_port = htons( port );
    source_addr.sin_port        = port;

    uint8_t tx_ack[ 4 ];

    tx_ack[ 0 ] = 0;
    tx_ack[ 1 ] = eError;
    tx_ack[ 2 ] = 0;
    tx_ack[ 3 ] = ( uint8_t ) error_code;

    int err = sendto( wifi_stat.UDP_socket, ( uint8_t* ) tx_ack, sizeof( tx_ack ), 0, ( struct sockaddr* ) &source_addr, sizeof( source_addr ) );
    if ( err < 0 )
    {
        if ( HW_SETTINGS_DEBUG )
        {
            ESP_LOGE( "WIFI", "Error occurred during sending: errno %d", errno );
        }
        return false;
    }
    return true;
}

bool wifi_receive_control_packet( void )
{
    uint8_t ret = false;
    if ( HW_SETTINGS_DEBUG )
    {
        // ESP_LOGI( "WIFI", "Control Data packet received" );
    }

    if ( rx_buffer[ 1 ] == WIFI_CONTROL_STATUS_IDENTIFIER )
    {
        // ESP_LOGI( "WIFI", "Control Status packet received" );
        SEND_TASK_COMMAND_t cmd = WIFI_TX_SEND_STATUS;
        ret                     = xQueueSend( send_task_cmd_queue_handle, ( void* ) &cmd, 1 );
    }
    return !!ret;
}
