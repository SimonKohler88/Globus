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

#include "inttypes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "sdkconfig.h"
#include "hw_settings.h"

#include "rpi_interface.h"


//#include "driver/timer.h"
#include "wifi.h"

#define WIFI_CONNECTED_BIT 				BIT0
#define WIFI_FAIL_BIT      				BIT1
#define WIFI_ACK_MESSAGE				"OK"
#define WIFI_NACK_MESSAGE				"NOK"
#define WIFI_LED_PACKET_IDENTIFIER		"L"
#define WIFI_SENSOR_PACKET_IDENTIFIER	"S"
#define WIFI_SENSOR_START_KEYWORD		"START"
#define WIFI_SENSOR_STOP_KEYWORD		"STOP"

#define WIFI_CONTROL_IDENTIFIER         "C"
#define WIFI_CONTROL_STATUS_IDENTIFIER  "S"
#define WIFI_CONTROL_PARAM_IDENTIFIER   "P"

#define WIFI_SENSOR_BRIGHTNESS_KEYWORD	"BRIGHT"
#define WIFI_DEBUG_PACKET_IDENTIFIER	"D"

void wifi_receive_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
bool wifi_send_packet(char* message);
bool wifi_receive_packet();
bool wifi_receive_data_packet(void);
bool wifi_receive_LED_packet(void);
//bool wifi_receive_Sensor_packet(void);

bool wifi_receive_control_packet(void);

bool wifi_receive_Debug_packet(void);
char* wifi_receive_debug_esp(void);
char* wifi_receive_start(void);
char* wifi_receive_stop(void);
char* wifi_receive_brightness(void);

//static EventGroupHandle_t s_wifi_event_group;

esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;
static uint8_t wifi_connected = false;

static int s_retry_num = 0;
static char rx_buffer[HW_SETTINGS_UDP_PACKET_SIZE];
static char tx_buffer[HW_SETTINGS_UDP_PACKET_SIZE];
static int UDP_socket;

void wifi_receive_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		esp_wifi_connect();
		wifi_connected = false;
	}
	else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		
		
		esp_wifi_connect();
		wifi_connected = false;
		s_retry_num++;
		if(HW_SETTINGS_DEBUG)
		{
			ESP_LOGI("WIFI", "retry to connect to the AP");
		}
	
//		else
//		{
//			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
//		}
		
	}
	else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		
		wifi_connected = true;
		if(HW_SETTINGS_DEBUG)
		{
			ESP_LOGI("WIFI", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		}
		s_retry_num = 0;
		//xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

void wifi_receive_init(void)
{
	//s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&wifi_receive_event_handler,
														NULL,
														&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
														IP_EVENT_STA_GOT_IP,
														&wifi_receive_event_handler,
														NULL,
														&instance_got_ip));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_WIFI_SSID,
			.password = CONFIG_WIFI_PASSWORD,
			/* Setting a password implies station will connect to all security modes including WEP/WPA.
			 * However these modes are deprecated and not advisable to be used. Incase your Access point
			 * doesn't support WPA2, these mode can be enabled by commenting below line */
		 .threshold.authmode = WIFI_AUTH_WPA2_PSK,

			.pmf_cfg = {
				.capable = true,
				.required = false
			},
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );

	if(HW_SETTINGS_DEBUG)
	{
		ESP_LOGI("WIFI", "wifi_init_sta finished.");
	}

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
//	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
//			WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
//			pdFALSE,
//			pdFALSE,
//			portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
//	if (bits & WIFI_CONNECTED_BIT)
//	{
//		if(HW_SETTINGS_DEBUG)
//		{
//			ESP_LOGI("WIFI", "connected to ap SSID:%s password:%s",
//				 CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);
//		}
//	}
//	else if (bits & WIFI_FAIL_BIT)
//	{
//		if(HW_SETTINGS_DEBUG)
//		{
//			ESP_LOGI("WIFI", "Failed to connect to SSID:%s, password:%s",
//				 CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);
//		}
//	}
//	else
//	{
//		if(HW_SETTINGS_DEBUG)
//		{
//			ESP_LOGE("WIFI", "UNEXPECTED EVENT");
//		}
//	}
	//TODO: keep this event handler?
	/* The event will not be processed after unregister */
	//ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	//ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	//vEventGroupDelete(s_wifi_event_group);
}

void wifi_receive_udp_task(void *pvParameters)
{
	int addr_family = AF_INET;
	int ip_protocol = IPPROTO_IP;

	while (1)
	{
		if ( wifi_connected ) 
		{
			UDP_socket = socket(addr_family, SOCK_DGRAM, ip_protocol);
			if (UDP_socket < 0)
			{
				if(HW_SETTINGS_DEBUG)
				{
					ESP_LOGE("WIFI", "Unable to create socket: errno %d", errno);
				}
				break;
			}
			if(HW_SETTINGS_DEBUG)
			{
				ESP_LOGI("WIFI", "Socket created, sending to %s:%d", CONFIG_WIFI_IPV4_ADDR, CONFIG_UDP_PORT);
			}
		}

		while( wifi_connected )
		{
			if(!wifi_receive_packet())
			{
				break;
			}

			taskYIELD();
		}

		if (UDP_socket != -1)
		{
			if(HW_SETTINGS_DEBUG)
			{
				ESP_LOGE("WIFI", "Shutting down socket and restarting...");
			}
			shutdown(UDP_socket, 0);
			close(UDP_socket);
		}
		vTaskDelay( 1000 );
	}
	vTaskDelete(NULL);
}

bool wifi_send_packet( char* message )
{
	static struct sockaddr_in dest_addr;
	dest_addr.sin_addr.s_addr = inet_addr(CONFIG_WIFI_IPV4_ADDR);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(CONFIG_UDP_PORT);

	int err = sendto(UDP_socket, message, strlen(message), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if (err < 0)
	{
		if(HW_SETTINGS_DEBUG)
		{
			ESP_LOGE("WIFI", "Error occurred during sending: errno %d", errno);
		}
		return false;
	}
	if(HW_SETTINGS_DEBUG)
	{
		ESP_LOGI("WIFI", "Message sent");
	}

	return true;
}

bool wifi_send_packet_raw( uint8_t* data_ptr, uint32_t size )
{
	static struct sockaddr_in dest_addr;
	dest_addr.sin_addr.s_addr = inet_addr(CONFIG_WIFI_IPV4_ADDR);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(CONFIG_UDP_PORT);

	int err = sendto(UDP_socket, data_ptr, size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if (err < 0)
	{
		if(HW_SETTINGS_DEBUG)
		{
			ESP_LOGE("WIFI", "Error occurred during sending: errno %d", errno);
		}
		return false;
	}
	if(HW_SETTINGS_DEBUG)
	{
		ESP_LOGI("WIFI", "Message sent");
	}

	return true;
}


bool wifi_receive_packet()
{
	struct sockaddr_in source_addr;
	source_addr.sin_family = AF_INET;
	source_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	source_addr.sin_port = htons(CONFIG_UDP_PORT);

	if(bind(UDP_socket, (struct sockaddr *)&source_addr, sizeof(struct sockaddr_in)) == -1)
	{
		if(HW_SETTINGS_DEBUG)
		{
			ESP_LOGI("WIFI", "Binding failed");
		}
		return false;
	}

	int len = recv(UDP_socket, rx_buffer, sizeof(rx_buffer), 0);

	// Error occurred during receiving
	if (len < 0)
	{
		if(HW_SETTINGS_DEBUG)
		{
			ESP_LOGE("WIFI", "recvfrom failed: errno %d", errno);
		}
		return false;
	}
	// Data received
	else
	{
		rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
		
		if (strncmp(rx_buffer, WIFI_LED_PACKET_IDENTIFIER, sizeof(WIFI_LED_PACKET_IDENTIFIER)-1) == 0)
		{
			
		}
		else if(strncmp( rx_buffer, WIFI_CONTROL_IDENTIFIER , sizeof( WIFI_CONTROL_IDENTIFIER ) -1 ) == 0) //todo
		{
			
		}
		//wifi_receive_data_packet();
	}

	return true;
}

bool wifi_receive_data_packet(void)
{/*
	
	{
		return wifi_receive_LED_packet();
	}
	
	{
		return wifi_receive_control_packet();
	}
	else if(strncmp(rx_buffer, WIFI_DEBUG_PACKET_IDENTIFIER, sizeof(WIFI_DEBUG_PACKET_IDENTIFIER)-1) == 0)
	{
		return wifi_receive_Debug_packet();
	}
	else
	{
		if(HW_SETTINGS_DEBUG)
		{
			ESP_LOGI("WIFI", "Unknown packet received, %s", rx_buffer);
		}
		return wifi_send_packet(WIFI_NACK_MESSAGE);
	}*/
	return wifi_send_packet(WIFI_NACK_MESSAGE);
}

bool wifi_receive_LED_packet(void)
{
	return 0;
}

bool wifi_receive_control_packet( void )
{
	if( HW_SETTINGS_DEBUG )
	{
		ESP_LOGI( "WIFI", "Control Data packet received" );
	}

	if( strncmp( rx_buffer + 1, WIFI_CONTROL_STATUS_IDENTIFIER, sizeof(WIFI_CONTROL_STATUS_IDENTIFIER) -1 ) == 0 )
	{
		
		ESP_LOGI( "WIFI", "Control Status packet received" );
		
		//todo:marker
		//uint32_t status_addr = 0;
		//uint32_t* data_ptr= &status_addr;
		
		uint32_t size = get_status_data( tx_buffer );
		//tx_buffer[size] = 0;
		ESP_LOGI( "Wifi", "%" PRIu8 "\n", tx_buffer[0] );
		
		//ESP_LOGI( "WIFI", "%s", tx_buffer );
		
		
		return wifi_send_packet_raw( (uint8_t*)tx_buffer, size );
		//TODO: send data from rpi_interface
		//return wifi_send_packet(  WIFI_ACK_MESSAGE );
	}
	
	
	else if( strncmp( rx_buffer + 1, WIFI_CONTROL_PARAM_IDENTIFIER, sizeof( WIFI_CONTROL_PARAM_IDENTIFIER ) - 1 ) == 0 )
	{
		//return wifi_send_packet(wifi_receive_stop());
		return wifi_send_packet(  WIFI_ACK_MESSAGE );
		ESP_LOGI( "WIFI", "Control Param packet received" );
		//todo: send data to rpi_interface --> parameterstuff first
	}
	
	else
	{
		return wifi_send_packet( WIFI_NACK_MESSAGE );
	}
}

bool wifi_receive_Debug_packet(void)
{
	if(HW_SETTINGS_DEBUG)
	{
		ESP_LOGI("WIFI", "Debug Data packet received");
	}

	return wifi_send_packet(wifi_receive_debug_esp());
}

char* wifi_receive_debug_esp(void)
{
	static char message[100];


	return message;
}

char* wifi_receive_start(void)
{

	return WIFI_ACK_MESSAGE;
}

char* Wifi_receive_stop(void)
{

	return WIFI_ACK_MESSAGE;
}

char* Wifi_receive_brightness(void)
{
	
	return WIFI_ACK_MESSAGE;
}
