/*
 * fpga_registers.h
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#ifndef MAIN_FPGA_REGISTERS_H_
#define MAIN_FPGA_REGISTERS_H_

/* Read command structure: [ 1 byte command ] then clock as many bits fpga_status_t is big  */
#define FPGA_READ_ALL_COMMAND    ( uint8_t ) 1

/* Write command structure: [ 1 byte command ][ 1 byte addr ] [ 4 byte value ] */
#define FPGA_WRITE_COMMAND       ( uint8_t ) 3

/*1+1+4*/
#define FPGA_WRITE_COMMAND_SIZE_BYTES  ( 6 )    

#define FPGA_ADDR_LEDS           ( uint8_t ) 0
#define FPGA_ADDR_IMAGE_WIDTH    ( uint8_t ) 1
#define FPGA_ADDR_IMAGE_HEIGHT   ( uint8_t ) 2
#define FPGA_ADDR_BRIGHTNESS     ( uint8_t ) 3

#endif /* MAIN_FPGA_REGISTERS_H_ */
