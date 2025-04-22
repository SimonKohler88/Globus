/*
 * hw_settings.h
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#ifndef MAIN_HW_SETTINGS_H_
#define MAIN_HW_SETTINGS_H_

// #define HW_SETTINGS_DEBUG                  ( 1 )

/* Development:
 * - Let QSPI send static Frame triggered by framerequest-input
 * - dont connect to network
 *
 * Comment out following line for hot version
 */
// #define DEVELOPMENT_SET_QSPI_ON_PIN_OUT   ( 1 )

/* Freertos */
#define FREERTOS_STACK_SIZE_FPGA_CTRL      ( 4096 )
#define FREERTOS_STACK_SIZE_STATUS_CTRL    ( 4096 * 2 )
#define FREERTOS_STACK_SIZE_QSPI           ( 4096 * 2 )
#define FREERTOS_STACK_SIZE_HTTP           ( 4096 * 3 )
#define FREERTOS_STACK_SIZE_JPEG           ( 4096 )
#define FREERTOS_STACK_SIZE_WIFI           ( 4096 )

/* Status Control task */
#define STAT_CTRL_QUEUE_NUMBER_OF_COMMANDS ( 5 )
#define STAT_CTRL_PIN_FRAME_REQUEST        ( 8 )
#define STAT_CTRL_PIN_RESERVE_1            ( 6 )
#define STAT_CTRL_PIN_RESERVE_2            ( 5 )
#define STAT_CTRL_PIN_RESERVE_3            ( 4 )
#define STAT_CTRL_PIN_RESET_FPGA           ( 7 )
#define STAT_CTRL_ENABLE_LED               ( 1 )
#define STAT_CTRL_QSPI_FRAME_RATE          ( 16 )
#define T_GROUP_0                          ( TIMER_GROUP_0 )
#define T_ID                               ( TIMER_0 )

/* QSPI */
#define QSPI_PIN_HD_D3                     ( 9 )
#define QSPI_PIN_CS0                       ( 10 )
#define QSPI_PIN_D_D0                      ( 11 )
#define QSPI_PIN_CLK                       ( 12 )
#define QSPI_PIN_Q_D1                      ( 13 )
#define QSPI_PIN_WP_D2                     ( 14 )
#define QSPI_HOST                          ( SPI2_HOST )
#define QSPI_DMA_CHANNEL                   ( SPI_DMA_CH_AUTO )
#define QSPI_BUS_FREQ                      ( SPI_MASTER_FREQ_26M )

/* FPGA Control */
#define SPI_CS                             ( 18 )
#define SPI_MISO                           ( 16 )
#define SPI_MOSI                           ( 17 )
#define SPI_CLK                            ( 15 )
#define SPI_FREQ                           ( SPI_MASTER_FREQ_8M )
#define SPI_HOST                           ( SPI3_HOST )
#define SPI_MAX_TRANSFER_BYTES             ( 256 )
#define SPI_CMD_QUEUE_SIZE                 ( 10 )

/* FIFO */
#define FIFO_NUMBER_OF_FRAMES              ( 10 )

/* Image */
#define IMAGE_MAX_PIXEL_HEIGHT             ( 120 )
#define IMAGE_MAX_PIXEL_WIDTH              ( 256 )
#define IMAGE_BYTES_PER_PIXEL              ( 3 )
/* 92160 Bytes. Must be a multiple of 16 */
#define IMAGE_TOTAL_BYTE_SIZE              ( IMAGE_MAX_PIXEL_HEIGHT * IMAGE_MAX_PIXEL_WIDTH * IMAGE_BYTES_PER_PIXEL )

/* pic: jpeg as baseline, not progressive, in YCrCb Color format: ~8900 bytes, JPEG in-buffer must be 16byte aligned*/
#define IMAGE_JPEG_SIZE_BYTES              ( 11000 )

/* LED */
#define CONFIG_BLINK_GPIO                  ( 48 )

/* Rotor Encoding */
#if CONFIG_BLINK_GPIO == 48
    #define ENC_PIN_CONNECTED ( 38 )
#else
    #define ENC_PIN_CONNECTED ( 48 )
#endif

#define ENC_PIN_EXP_0       ( 21 )
#define ENC_PIN_EXP_1       ( 1 )
#define ENC_PIN_EXP_2       ( 2 )
#define ENC_PIN_EXP_3       ( 47 )

/* Task Verbosity */
#define HTTP_TASK_VERBOSE   ( 0 )
#define QSPI_TASK_VERBOSE   ( 0 )
#define CTRL_TASK_VERBOSE   ( 0 )
#define JPEG_TASK_VERBOSE   ( 1 )
#define FIFO_VERBOSE        ( 0 )
#define PIC_BUFF_VERBOSE    ( 0 )
#define RPI_IF_VERBOSE      ( 0 )

/* Networkk definitions */
#define HTTP_PORT           "8123"
#define HTTP_PATH           "/frame"
#define WIFI_CONN_MAX_RETRY ( 6 )

#define WHERE               ( 1 )
#if ( WHERE == 1 )
    #define CONFIG_WIFI_SSID      "UPCF611258"
    #define CONFIG_WIFI_PASSWORD  "Fs4nzkzne4tu"
    #define CONFIG_WIFI_IPV4_ADDR "192.168.0.22"
#endif

#if ( WHERE == 2 )
    #define CONFIG_WIFI_SSID      "DESKTOP-P96TM8B 9415"
    #define CONFIG_WIFI_PASSWORD  "5716Jt3/"
    #define CONFIG_WIFI_IPV4_ADDR "192.168.137.2"
#endif

#endif /* MAIN_HW_SETTINGS_H_ */
