/*
 * rotor_encoding.c
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#include "rotor_encoding.h"
#include "hw_settings.h"
#include "driver/gpio.h"


static rotor_encoding_status_t * status;
void ( *connected_changed_cb )( uint8_t );


void rotor_encoding_init( rotor_encoding_status_t * status_ptr, void ( *connected_changed_cb_func )( uint8_t ) )
{
	status = status_ptr;
	connected_changed_cb = connected_changed_cb_func;
	
	gpio_set_direction( ENC_PIN_CONNECTED, GPIO_MODE_INPUT );
	gpio_set_direction( ENC_PIN_EXP_0, GPIO_MODE_INPUT );
	gpio_set_direction( ENC_PIN_EXP_1, GPIO_MODE_INPUT );
	gpio_set_direction( ENC_PIN_EXP_2, GPIO_MODE_INPUT );
	gpio_set_direction( ENC_PIN_EXP_3, GPIO_MODE_INPUT );
	
}

void rotor_encoding_update( void )
{
	uint8_t connected;
	uint8_t exp_0;
	uint8_t exp_1;
	uint8_t exp_2;
	uint8_t exp_3;
	
	connected = ( uint8_t ) gpio_get_level( ENC_PIN_CONNECTED );
	if( status->rotor_connected != connected )
	{
		if( connected_changed_cb != NULL ) connected_changed_cb( connected );
	}
	
	status->rotor_connected = connected;
	
	if( connected )
	{
		exp_0 = ( uint8_t ) gpio_get_level( ENC_PIN_EXP_0 );
		exp_1 = ( uint8_t ) gpio_get_level( ENC_PIN_EXP_0 );
		exp_2 = ( uint8_t ) gpio_get_level( ENC_PIN_EXP_0 );
		exp_3 = ( uint8_t ) gpio_get_level( ENC_PIN_EXP_0 );
		
		status->rotor_num = exp_0 + (exp_1<<1) + (exp_2<<2) + (exp_3<<3);
	}
}