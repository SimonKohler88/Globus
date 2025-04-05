/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    i2c.c
 * @brief   This file provides code for the configuration
 *          of the I2C instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */
static I2C_DATA_MEM_t* data_storage = NULL;
static uint8_t RxData[ 5 ];
union conv
{
    uint32_t tx_val;
    uint8_t bytes[ 4 ];
};
volatile union conv conv_val;
volatile uint8_t next_frame_flag;

static void i2c_set_value( uint8_t addr, uint32_t value );
/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */
  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00201D2B;
  hi2c1.Init.OwnAddress1 = 0x12; // **** Changed by Thomas ****
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

    /* I2C1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void i2c_init( I2C_DATA_MEM_t* data_mem )
{
    data_storage              = data_mem;
    data_storage->num_entries = sizeof(data_mem->data)/sizeof(data_mem->data[0]);
    HAL_I2C_EnableListen_IT( &hi2c1 );
}

uint8_t i2c_update()
{
    uint8_t has_update = data_storage->has_update;
    if ( data_storage->has_update )
    {
        data_storage->has_update = 0;
    }
    HAL_I2C_EnableListen_IT( &hi2c1 );

    return has_update;
}

extern void HAL_I2C_AddrCallback( I2C_HandleTypeDef* hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode )
{
    uint16_t addr_match = AddrMatchCode;
    if ( TransferDirection == I2C_DIRECTION_TRANSMIT )  // if the master wants to transmit the data
    {
        /*  For Write and read, first isr cb comes here. more data received in isr "rx cplt" */
        HAL_I2C_Slave_Sequential_Receive_IT( hi2c, RxData, 1, I2C_FIRST_FRAME );
        next_frame_flag = 1;
    }
    else if ( TransferDirection == I2C_DIRECTION_RECEIVE )  // if the master wants to receive the data
    {
        /* Read Transfer: after repeated start, a read req comes here */
        uint8_t addr    = RxData[ 0 ];
        next_frame_flag = 0;
        if ( addr < data_storage->num_entries )
        {
            conv_val.tx_val = *( data_storage->data[ addr ].val_ptr );
            HAL_I2C_Slave_Sequential_Transmit_IT( hi2c, ( uint8_t* ) conv_val.bytes, 4, I2C_LAST_FRAME );
        }
/* USER CODE BEGIN - Added by Thomas */
        else if (addr == 0xFF)  // Specific request for "I2C between Raspi and Nucleo is working"
                {
                    char message[] = "I2C between Raspi and Nucleo is working";
                    HAL_I2C_Slave_Sequential_Transmit_IT(hi2c, (uint8_t *)message, sizeof(message), I2C_LAST_FRAME);
                }
/* USER CODE END - Added by Thomas */

    }
    else
    {
        next_frame_flag = 0;
        HAL_I2C_EnableListen_IT( hi2c );
    }
}

extern void HAL_I2C_ListenCpltCallback( I2C_HandleTypeDef* hi2c ) { HAL_I2C_EnableListen_IT( hi2c ); }

extern void HAL_I2C_SlaveTxCpltCallback( I2C_HandleTypeDef* hi2c ) { HAL_I2C_EnableListen_IT( hi2c ); }

extern void HAL_I2C_SlaveRxCpltCallback( I2C_HandleTypeDef* hi2c )
{
    if ( next_frame_flag )
    {
        /* Write Req: receive more data */
        HAL_I2C_Slave_Sequential_Receive_IT( hi2c, &RxData[ 1 ], 4, I2C_FIRST_AND_LAST_FRAME );
    }
    else
    {
        uint8_t addr = RxData[ 0 ];

        if ( addr < data_storage->num_entries )
        {
            if ( data_storage->data[ addr ].type == I2C_ENTRY_TYPE_WRITE || data_storage->data[ addr ].type == I2C_ENTRY_TYPE_READ_WRITE )
            {
                uint32_t val                        = RxData[ 1 ] << 24 | RxData[ 2 ] << 16 | RxData[ 3 ] << 8 | RxData[ 4 ];
                *data_storage->data[ addr ].val_ptr = val;
                data_storage->has_update            = 1;
            }
        }
    }
    next_frame_flag = 0;
}

extern void HAL_I2C_ErrorCallback( I2C_HandleTypeDef* hi2c ) { HAL_I2C_EnableListen_IT( hi2c ); }

void i2c_enable_irq( void )
{
    HAL_NVIC_EnableIRQ( I2C1_EV_IRQn );
    HAL_NVIC_EnableIRQ( I2C1_ER_IRQn );
}
void i2c_disable_irq( void )
{
    HAL_NVIC_DisableIRQ( I2C1_EV_IRQn );
    HAL_NVIC_DisableIRQ( I2C1_ER_IRQn );
}

/* USER CODE END 1 */
