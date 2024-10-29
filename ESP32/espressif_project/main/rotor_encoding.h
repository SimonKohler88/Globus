/*
 * rotor_encoding.h
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#ifndef MAIN_ROTOR_ENCODING_H_
#define MAIN_ROTOR_ENCODING_H_

#include "stdint.h"

// TODO: not yet used

struct {
	uint8_t rotor_connected;
	uint8_t rotor_num;	
} typedef rotor_encoding_status_t;


void rotor_encoding_init( rotor_encoding_status_t * status_ptr, void ( *connected_changed_cb_func )( uint8_t ) );
void rotor_encoding_update( void );

#endif /* MAIN_ROTOR_ENCODING_H_ */
