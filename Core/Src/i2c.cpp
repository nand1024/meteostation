/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32g0xx_ll_utils.h"

/* USER CODE BEGIN 0 */
void i2cWrite255(uint8_t adress, uint8_t regAdr, uint8_t *data, uint32_t size) {

	//start and connect to slave
	LL_I2C_HandleTransfer(I2C2,
						  adress << 1,
						  LL_I2C_ADDRSLAVE_7BIT,
						  size + 1,
						  LL_I2C_MODE_SOFTEND,
						  LL_I2C_GENERATE_START_WRITE);
	//send addres of registr

	for (uint8_t timeOut = 0;
		 timeOut < 50 && LL_I2C_IsActiveFlag_TXIS(I2C2) == 0;
		 timeOut++) {
		LL_mDelay(1);
	}
	LL_I2C_TransmitData8(I2C2, regAdr);

	//send data
	for (uint8_t i = 0; i < size; i++) {
		for (uint8_t timeOut = 0;
			 timeOut < 50 && LL_I2C_IsActiveFlag_TXIS(I2C2) == 0;
			 timeOut++) {
			LL_mDelay(1);
		}
		LL_I2C_TransmitData8(I2C2, data[i]);
	}
	for (uint8_t timeOut = 0;
		 timeOut < 50 && LL_I2C_IsActiveFlag_TXIS(I2C2) == 0;
		 timeOut++) {
		LL_mDelay(1);
	}
	LL_I2C_GenerateStopCondition(I2C2);
}

uint8_t i2cRead255(uint8_t adress, uint8_t regAdr, uint8_t *data, uint32_t size) {

	uint8_t readBytes = 0;
	LL_I2C_HandleTransfer(I2C2,
						  adress << 1,
						  LL_I2C_ADDRSLAVE_7BIT,
						  1,
						  LL_I2C_MODE_SOFTEND,
						  LL_I2C_GENERATE_START_WRITE);
	for (uint8_t timeOut = 0;
		 timeOut < 50 && LL_I2C_IsActiveFlag_TXIS(I2C2) == 0;
		 timeOut++) {
		LL_mDelay(1);
	}
	LL_I2C_TransmitData8(I2C2, regAdr);
	for (uint8_t timeOut = 0;
		 timeOut < 50 && LL_I2C_IsActiveFlag_TXIS(I2C2) == 0;
		 timeOut++) {
		LL_mDelay(1);
	}
	for (uint8_t timeOut = 0;
			 timeOut < 100;
			 timeOut++) {
		LL_I2C_HandleTransfer(I2C2,
						   adress << 1,
						   LL_I2C_ADDRSLAVE_7BIT,
						   size,
						   LL_I2C_MODE_SOFTEND,
						   LL_I2C_GENERATE_RESTART_7BIT_READ);
		if (LL_I2C_IsActiveFlag_NACK(I2C2) == 1) {
			LL_mDelay(1);
		} else {
			break;
		}
	}
	for (uint8_t i = 0; i < size; i++) {
		uint8_t timeOut;
		for (timeOut = 0;
			 timeOut < 50 && LL_I2C_IsActiveFlag_RXNE(I2C2) == 0;
			 timeOut++) {
			LL_mDelay(1);
		}
		if (timeOut < 50) {
			data[i] = LL_I2C_ReceiveData8(I2C2);
			readBytes++;
		} else {
			break;
		}

	}
	LL_I2C_GenerateStopCondition(I2C2);
	return readBytes;
}
/* USER CODE END 0 */

/* I2C2 init function */
void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  /**I2C2 GPIO Configuration
  PA11 [PA9]   ------> I2C2_SCL
  PA12 [PA10]   ------> I2C2_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */

  /** I2C Initialization
  */
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x00303D5B;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C2, &I2C_InitStruct);
  LL_I2C_EnableAutoEndMode(I2C2);
  LL_I2C_SetOwnAddress2(I2C2, 0, LL_I2C_OWNADDRESS2_NOMASK);
  LL_I2C_DisableOwnAddress2(I2C2);
  LL_I2C_DisableGeneralCall(I2C2);
  LL_I2C_EnableClockStretching(I2C2);
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
