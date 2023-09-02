/*
 * dht22_lib.cpp
 *
 *  Created on: 28 черв. 2023 р.
 *      Author: 2andn
 */
#include <cstdint>
#include "tim.h"

static void pinMode (uint8_t val)
{
	  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

	  /* GPIO Ports Clock Enable */
	  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);

	  /**/
	  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_9);


	  /**/
	  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
	  if (val) {
		  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	  } else {
		  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	  }

	  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void pinSet (uint8_t val)
{
	if (val) {
		LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_9);
	} else {
		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_9);
	}
}

static uint8_t pinGet ()
{
	return (LL_GPIO_ReadInputPort(GPIOB) & LL_GPIO_PIN_9) ? 1 : 0;
}

void dht22Read(float *temperature, float *humidity)
{
	uint8_t timeWait;
	uint64_t raw = 0;
	uint16_t hum, temp;
	uint8_t crc8;
	uint8_t sumBytes = 0;

	pinMode(1);
	pinSet(0);
	LL_mDelay(2);

	pinMode(0);
	for(timeWait = 0; timeWait < 4 && pinGet() == 0; timeWait++)delayUS(10);
	for(timeWait = 0; timeWait < 4 && pinGet() == 1; timeWait++)delayUS(10);
	for(timeWait = 0; timeWait < 8 && pinGet() == 0; timeWait++)delayUS(10);
	for(timeWait = 0; timeWait < 8 && pinGet() == 1; timeWait++)delayUS(10);
	for (uint8_t i = 0; i < 40; i++) {
		for(timeWait = 0; timeWait < 50 && pinGet() == 0; timeWait++)delayUS(1);
		delayUS(28);
		if (pinGet() == 1) {
			raw++;
		}
		if(i < 39) {
			raw<<=1;
		}
		for(timeWait = 0; timeWait < 70 && pinGet() == 1; timeWait++)delayUS(1);
	}
	pinMode(1);
	pinSet(1);
	hum = raw>>24;
	temp = raw>>8;
	crc8 = raw;

	sumBytes += hum >> 8;
	sumBytes += hum & 0xff;

	sumBytes += temp >> 8;
	sumBytes += temp & 0xff;

	if (crc8 == sumBytes){
		*temperature = (temp & (1UL<<15)) ? (temp & 0x7FFF)/-10.0 : temp/10.0;
		*humidity = hum / 10.0;
	} else {
		*temperature -273;
		*humidity -1;
	}

}
