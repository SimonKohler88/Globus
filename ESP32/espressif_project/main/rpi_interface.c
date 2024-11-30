/*
 * rpi_interface.c
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#include "rpi_interface.h"
#include "esp_log.h"
#include "hw_settings.h"
#include "inttypes.h"
#include <string.h>

struct
{
    void *struct_ptr;
    uint32_t size;
} typedef if_struct_t;

if_struct_t registered_status_structs[ MAX_STATUS_STRUCTS ];
static uint8_t num_status_structs = 0;

// todo: make parameterstuff
//  list with id (1Byte) and value (4 Bytes)

void register_status_struct( void *if_struct, uint32_t size )
{
    if ( num_status_structs < MAX_STATUS_STRUCTS )
    {
        if_struct_t struc;
        struc.struct_ptr                                = if_struct;
        struc.size                                      = size;
        registered_status_structs[ num_status_structs ] = struc;
        num_status_structs++;
    }
    else ESP_LOGE( "RPI If", "too many status structs" );
}

uint32_t get_status_data( void *data_ptr )
{
    uint32_t size = 0;
    for ( uint8_t i = 0; i < num_status_structs; i++ )
    {
        size += registered_status_structs[ i ].size;
        memcpy( data_ptr, registered_status_structs[ i ].struct_ptr, registered_status_structs[ i ].size );
        data_ptr += registered_status_structs[ i ].size;
    }

    // uint8_t* ptr = (uint8_t*) data_ptr;

    if ( HW_SETTINGS_DEBUG )
    {
        //		ESP_LOGI( "WIFI", "num: %" PRIu8 "\n", num_status_structs );
        //		ESP_LOGI( "WIFI", "size: %" PRIu32 "\n", size );
    }
    // if( size > 0 ) memcpy(data_ptr, registered_status_structs, size);
    // for (uint16_t i = 0; i < size; i++ ) ptr[ i ] = i;
    //*data_ptr = (uint32_t*)(&registered_status_structs[0]);

    return size;
}
