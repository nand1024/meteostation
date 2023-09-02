/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "LiquidCrystal.h"
#include "LiquidCrystalUserInterface.h"
#include "BME280I2C.h"
#include <cmath>
#include "dht22Lib.h"
#include "records.h"

using namespace std;

#define LEN_RECORDS    6 * 24
#define ABS(V) (V < 0 ? -V : V)
#define GET_FRACTION_NUM(V) ((uint16_t)ABS((int16_t)((int16_t)(V * 10.0) - ((int16_t)V) * 10)))

static float pressBuff[LEN_RECORDS];
static float tempHomeBuff[LEN_RECORDS];
static float tempYardBuff[LEN_RECORDS];
static float humHomeBuff[LEN_RECORDS];
static float humYardBuff[LEN_RECORDS];
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

void uartTX(char *data) {
	for(; *data != '\0'; data++) {
		while(!LL_USART_IsActiveFlag_TXE_TXFNF(USART1));
		LL_USART_TransmitData8(USART1, *data);
	}
}

void showInfo(LiquidCrystal *display,
		      float yardTemp,
		      float yardHum,
		      float homeTemp,
		      float homeHum,
		      float pressure,
			  float diffPress,
		      char *str)
{
    char strBuff[21];

	display->setCursor(0, 0);
	display->print("                    ");
	display->setCursor(0, 0);
	if (yardHum == -1 || yardTemp == -273) {//якщо зовнішній сенсор не під'єднанний
		sprintf(strBuff, "out sensor NC");
	} else {
		sprintf(strBuff, "Yard T%3i.%1uC RH%3u%%",
				(int16_t)yardTemp, GET_FRACTION_NUM(yardTemp),
				(uint16_t)yardHum);
	}
	display->print(strBuff);

	display->setCursor(0, 1);
	display->print("                    ");
	display->setCursor(0, 1);
	sprintf(strBuff, "Home T%3i.%1uC RH%3u%%",
			(int16_t)homeTemp, GET_FRACTION_NUM(homeTemp),
			(uint16_t)homeHum);
	display->print(strBuff);
	display->setCursor(0, 2);
	sprintf(strBuff, "Press %3u.%1umm  %2i.%1u", (uint16_t)pressure,
			GET_FRACTION_NUM(pressure),
			(int16_t)diffPress,
			GET_FRACTION_NUM(diffPress));
	display->print(strBuff);

	display->setCursor(0, 3);
	display->print("                    ");
	display->setCursor(0, 3);
	display->print(str);

}

int main(void)
{
	uint32_t count = 6000;
	float yardTemp, yardHum;
	float homeTemp, homeHum, homePress;
	float diffPress_from_3H, diffPress_from_3H_to_6H, diffPress_all_time;
	float diffTempYard_from_1H, diffTempYard_from_1H_to_3H;
	float diffHumYard_from_3H, diffHumYard_from_3H_to_6H;

	float bufferHomeTemp, bufferHomeHum, bufferYardTemp, bufferYardHum, bufferPressure;
	uint8_t lenDataPress, lenDataYardHum, lenDataYardTemp;

	char weather[21] = {0};

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

	/* SysTick_IRQn interrupt configuration */
	NVIC_SetPriority(SysTick_IRQn, 3);

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C2_Init();
	MX_TIM3_Init();
	MX_USART1_UART_Init();

	GpioInterface pRS(GPIOA, LL_GPIO_PIN_4);
	GpioInterface pRW(GPIOA, LL_GPIO_PIN_5);
	GpioInterface pE(GPIOA, LL_GPIO_PIN_6);

	GpioInterface pD4(GPIOA, LL_GPIO_PIN_3);
	GpioInterface pD5(GPIOA, LL_GPIO_PIN_2);
	GpioInterface pD6(GPIOA, LL_GPIO_PIN_1);
	GpioInterface pD7(GPIOA, LL_GPIO_PIN_0);

	LiquidCrystal display(&pRS, &pRW, &pE, &pD4, &pD5, &pD6, &pD7);

	display.begin(20, 4, LCD_5x10DOTS);
	BME280I2C bme;
	if (!bme.begin()) {
		display.clear();
		display.setCursor(0, 0);
		display.print("sensor not find");
		while(1);
	}

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	record pressureRec(pressBuff, LEN_RECORDS);
	record tempHomeRec(tempHomeBuff, LEN_RECORDS);
	record tempYardRec(tempYardBuff, LEN_RECORDS);
	record humHomeRec(humHomeBuff, LEN_RECORDS);
	record humYardRec(humYardBuff, LEN_RECORDS);

	diffPress_from_3H = 0;

	while (1)
	{
		if (count % 30 == 0) {
			BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
			BME280::PresUnit presUnit(BME280::PresUnit_torr);

			bme.read(homePress, homeTemp, homeHum, tempUnit, presUnit);
			dht22Read(&yardTemp, &yardHum);
			showInfo(&display, yardTemp, yardHum, homeTemp, homeHum, homePress, diffPress_from_3H, weather);

		}
		if(count >= 6000) {
			if (yardHum != -1 && yardTemp != -273) {
				tempYardRec.put(yardTemp);
				humYardRec.put(yardHum);
			}
			tempHomeRec.put(homeTemp);
			humHomeRec.put(homeHum);
			pressureRec.put(homePress);

			lenDataYardTemp = tempYardRec.getSizeWrites();
			lenDataYardHum = humYardRec.getSizeWrites();
			lenDataPress = pressureRec.getSizeWrites();

			if(lenDataYardTemp >= 6) {
				diffTempYard_from_1H = tempYardRec.dataAnalyz(6);
			}
			if(lenDataYardTemp > 6) {
				if (lenDataYardTemp <= 18) {
					diffTempYard_from_1H_to_3H = tempYardRec.dataAnalyz(lenDataYardTemp);
				} else {
					diffTempYard_from_1H_to_3H = tempYardRec.dataAnalyz(18);
				}
			}



			if(lenDataYardHum >= 18) {
				diffHumYard_from_3H = humYardRec.dataAnalyz(18);
			}
			if(lenDataYardHum > 18) {
				if (lenDataYardHum <= 36) {
					diffHumYard_from_3H_to_6H = humYardRec.dataAnalyz(lenDataYardHum);
				} else {
					diffHumYard_from_3H_to_6H = humYardRec.dataAnalyz(36);
				}
			}



			if (lenDataPress >= 18) {
				diffPress_from_3H = pressureRec.dataAnalyz(18);
			}
			if (lenDataPress > 18) {
				if (lenDataPress <= 36) {
					diffPress_from_3H_to_6H = pressureRec.dataAnalyz(lenDataPress);
				} else {
					diffPress_from_3H_to_6H = pressureRec.dataAnalyz(36);
				}
			}
			if (lenDataPress > 36) {
				diffPress_all_time = pressureRec.dataAnalyz(lenDataPress);
			}

			memset(weather, 0, sizeof(weather));

			if (yardHum == -1 || yardTemp == -273) {//якщо зовнішній сенсор не під'єднанний
				if (lenDataPress >= 18 && diffPress_from_3H <= -3) { // різке падіння тиску, погіршення погоди
					strcat(weather, "storm");
				} else if (lenDataPress > 18 && (diffPress_from_3H <= -2 || diffPress_from_3H_to_6H <= -2)) { //падіння тиску
					strcat(weather, "rain");
				} else if (lenDataPress >= 18 && (diffPress_from_3H >= 2 || diffPress_from_3H_to_6H >= 2)) { //підняття тиску
					strcat(weather, "sun");
				} else {
					strcat(weather, "weather is unchanged");
				}
			} else {
				if (lenDataPress >= 18 && diffPress_from_3H <= -3) { // різке падіння тиску, погіршення погоди
					strcat(weather, "storm");
				} else if (lenDataPress > 18 && (diffPress_from_3H <= -2 || diffPress_from_3H_to_6H <= -2)) { //падіння тиску
					if (yardTemp > 0) {
						strcat(weather, "rain");
					} else {
						strcat(weather, "snow");
					}
				} else if (lenDataPress > 36 && (diffPress_from_3H_to_6H < -0.5 || diffPress_all_time < -0.5)) { //повільне падіння тиску
					if (yardTemp > 0) {
						strcat(weather, "cloudly maybe rain");
					}  else if(lenDataYardHum >= 18 && diffHumYard_from_3H >= 10) { //температура нижче нуля, різке зростання вологості
						strcat(weather, "snowfall");
					} else if (lenDataYardHum >= 18 && diffHumYard_from_3H > 0
							   && lenDataYardTemp >= 18 && (diffTempYard_from_1H > 0 || diffTempYard_from_1H_to_3H > 0)) { // зростання вологості та температури
						strcat(weather, "wet snow");
					} else {
						strcat(weather, "cloudly maybe snow");
					}
				} else if(lenDataPress >= 18 && diffPress_from_3H >= 3) { //різке підняття тиску
					if (yardTemp < 0 && lenDataYardTemp >= 6 && (diffTempYard_from_1H < -2)) {
						strcat(weather, "cloudly maybe snow");
					} else if (yardTemp >= 0 && lenDataYardTemp >= 18 && (diffTempYard_from_1H < -2)) {
						strcat(weather, "cloudly maybe rain");
					} else {
						strcat(weather, "sun");
					}
				} else if (lenDataPress >= 18 && (diffPress_from_3H >= 2 || diffPress_from_3H_to_6H >= 2)) { //підняття тиску
						strcat(weather, "sun");
				} else {
					if (homePress <= 759) {
						if (yardTemp > 20 && yardHum > 75) {
							strcat(weather, "rain");
						} else if (yardTemp > 20) {
							strcat(weather, "hot weather");
						} else {
							strcat(weather, "weather is unchanged");
						}
					} else if (homePress <= 761) {
						if (lenDataYardTemp >= 6 && (diffTempYard_from_1H < -2)
							&& yardHum < 50) {
							strcat(weather, "cold weather");
						} else {
							strcat(weather, "weather is unchanged");
						}
					} else {
						strcat(weather, "weather is unchanged");
					}
				}
			}
			count = 0;
		} else {
			++count;
		}
		/*
		if(++count > 600) {
			uint16_t x = tempHomeRec.getSizeWrites();
			for (uint16_t i = 0; i < x; i++) {
				bufferHomeTemp = tempHomeRec.get();
				bufferHomeHum = humHomeRec.get();
				bufferYardTemp = tempYardRec.get();
				bufferYardHum = humYardRec.get();
				bufferPressure = pressureRec.get();
				sprintf(strBuff, "Yard T%3u.%1uC RH%3u%%\tHome T%3u.%1uC RH%3u%%\tPressure %3u.%1umm\n\n",
						(uint16_t)bufferYardTemp, (uint16_t)(bufferYardTemp * 10 - ((uint32_t)bufferYardTemp) * 10),
						(uint16_t)bufferYardHum,
						(uint16_t)bufferHomeTemp, (uint16_t)(bufferHomeTemp * 10 - ((uint32_t)bufferHomeTemp) * 10),
						(uint16_t)bufferHomeHum,
						(uint16_t)bufferPressure, (uint16_t)(bufferPressure * 10 - ((uint32_t)bufferPressure) * 10)
						);
				uartTX(strBuff);

			}
			count = 0;

		}
		*/

		LL_mDelay(100);
	/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  /* HSI configuration and activation */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* Sysclk activation on the HSI */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  }

  /* Set APB1 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

  LL_Init1msTick(16000000);

  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(16000000);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
