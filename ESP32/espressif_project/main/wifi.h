/*
 * wifi.h
 *
 *  Created on: 1 May 2021
 *      Author: cyril
 *
 *  copied and adapted: 19.10.2024
 *      Author: skohl
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

#include "freertos/FreeRTOS.h"

#include "inttypes.h"
#include "PSRAM_FIFO.h"


//enum {
//	FD_INIT = 0,
//	FD_SEND_START,
//	FD_WAIT_PACKET,	
//}; typedef uint8_t FRAME_DOWNLOAD_STATUS_t;
//
//enum {
//	WIFI_CMD_NONE,
//	WIFI_CMD_REQUEST_FRAME,
//	WIFI_CMD_RESET_FRAME,
//	
//}; typedef uint32_t wifi_fd_command_t;


struct {
	int UDP_socket;
	fifo_frame_t* current_frame_download;
//	FRAME_DOWNLOAD_STATUS_t fd_status;
	uint16_t tftp_block_number;
	uint16_t s_retry_num;
	uint8_t wifi_connected;
	

}  typedef WIFI_STAT_INTERNAL_t;


typedef enum { 
	eReadRequest = 1, 
	eWriteRequest, 
	eData, 
	eAck, 
	eError 
} eTFTPOpcode_t; /* Error codes from the RFC. */ 

typedef enum { 
	eFileNotFound = 1, 
	eAccessViolation, 
	eDiskFull, 
	eIllegalTFTPOperation, 
	eUnknownTransferID, 
	eFileAlreadyExists 
} eTFTPErrorCode_t; 

void wifi_receive_init(void);
void wifi_receive_udp_task(void *pvParameters);
void wifi_send_udp_task( void *pvParameters );

uint8_t wifi_request_frame( fifo_frame_t *frame_info );
//void update_frame_download( wifi_fd_command_t cmd, fifo_frame_t *frame_info );
uint8_t wifi_is_connected();
#endif /* MAIN_WIFI_H_ */
