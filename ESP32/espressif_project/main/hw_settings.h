/*
 * hw_settings.h
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#ifndef MAIN_HW_SETTINGS_H_
#define MAIN_HW_SETTINGS_H_

#define HW_SETTINGS_DEBUG                  1

/* Freertos */
#define FREERTOS_STACK_SIZE_FPGA_CTRL      4096 * 2
#define FREERTOS_STACK_SIZE_STATUS_CTRL    4096 * 3
#define FREERTOS_STACK_SIZE_QSPI           4096 * 2
#define FREERTOS_STACK_SIZE_WIFI           4096 * 2

/* Status Control task */
#define STAT_CTRL_QUEUE_NUMBER_OF_COMMANDS 5
#define STAT_CTRL_PIN_FRAME_REQUEST        8
#define STAT_CTRL_PIN_RESERVE_1            6
#define STAT_CTRL_PIN_RESERVE_2            5
#define STAT_CTRL_PIN_RESERVE_3            4
#define STAT_CTRL_PIN_RESET_FPGA           7

/* QSPI */
#define QSPI_PIN_HD_D3                     9
#define QSPI_PIN_CS0                       10
#define QSPI_PIN_D_D0                      11
#define QSPI_PIN_CLK                       12
#define QSPI_PIN_Q_D1                      13
#define QSPI_PIN_WP_D2                     14
#define QSPI_HOST                          SPI2_HOST
#define QSPI_DMA_CHANNEL                   SPI_DMA_CH_AUTO
#define QSPI_BUS_FREQ                      SPI_MASTER_FREQ_26M

/* FPGA Control */
#define SPI_CS                             18
#define SPI_MISO                           16
#define SPI_MOSI                           17
#define SPI_CLK                            15
#define SPI_FREQ                           SPI_MASTER_FREQ_8M
#define SPI_HOST                           SPI3_HOST
#define SPI_MAX_TRANSFER_BYTES             256
#define SPI_CMD_QUEUE_SIZE                 10

/* Image */
#define IMAGE_MAX_PIXEL_HEIGHT             120
#define IMAGE_MAX_PIXEL_WIDTH              256
#define IMAGE_BYTES_PER_PIXEL              3
#define IMAGE_TOTAL_BYTE_SIZE              ( IMAGE_MAX_PIXEL_HEIGHT * IMAGE_MAX_PIXEL_WIDTH * IMAGE_BYTES_PER_PIXEL )

#define IMAGE_JPEG_SIZE_BYTES              19118

/* MISC TASK : LED */
#define CONFIG_BLINK_PERIOD                1000
#define CONFIG_BLINK_GPIO                  48
#define LED_COLOR_B                        16
#define LED_COLOR_G                        8
#define LED_COLOR_R                        2

/* Rotor Encoding */
#if CONFIG_BLINK_GPIO == 48
    #define ENC_PIN_CONNECTED 38
#else
    #define ENC_PIN_CONNECTED 48
#endif

#define ENC_PIN_EXP_0                    21
#define ENC_PIN_EXP_1                    1
#define ENC_PIN_EXP_2                    2
#define ENC_PIN_EXP_3                    47

/* Buffer Control*/
// #define FIFO_NUMBER_OF_FRAMES            3

/* UDP TX Task */
#define UDP_TX_NUMBER_OF_CMD             3

/* Networkk definitions */
#define HW_SETTINGS_UDP_PACKET_SIZE      1500
#define HW_SETTINGS_WIFI_DONNECT_RETRIES 100
#define HW_SETTINGS_WIFI_DONNECT_RETRIES 100
#define HW_SETTINGS_LINES_PER_PACKET     3
#define HW_SETTINGS_WIFI_RX_OFFSET       2  // unusable length in bytes


#define HTTP_PORT "8123"
#define HTTP_PATH "/frame"
#define WIFI_CONN_MAX_RETRY 6

#define WHERE                            1
#if ( WHERE == 1 )
    #define CONFIG_WIFI_SSID      "UPCF611258"
    #define CONFIG_WIFI_PASSWORD  "Fs4nzkzne4tu"
    #define CONFIG_WIFI_IPV4_ADDR "192.168.0.22"
#endif

#if ( WHERE == 2 )
    #define CONFIG_WIFI_SSID      "DESKTOP-P96TM8B 9415"
    #define CONFIG_WIFI_PASSWORD  "5716Jt3/"
    #define CONFIG_WIFI_IPV4_ADDR "192.168.137.1"
#endif

#define CONFIG_UDP_PORT              1234
// #define CONFIG_UDP_FRAME_PACKET_SIZE 1024
#define CONFIG_UDP_FRAME_PACKET_SIZE 1400

#endif /* MAIN_HW_SETTINGS_H_ */
