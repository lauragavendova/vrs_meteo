/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hts221.h"
#include "lps25hb.h"
#include <stdio.h>
#include <string.h>
#include "text_function.h"
#include "stm32f3xx_hal.h"
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
  WX_CLOUDY = 1,
  WX_SUNNY  = 2,
  WX_RAIN   = 3,
  WX_STORM  = 4,
  WX_FOG    = 5
} WeatherFlag;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PRESS_BUF_SIZE   10         // 10 vzoriek
#define SAMPLE_PERIOD_S  60 		// 1 vzorka tlaku za 60 s
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int16_t temperature = 853;
int16_t pressure = 111;
int8_t screen_index = 0;
int8_t screen_status = 0;
int8_t screen_tick = 0;
int32_t butonCounter = 0;

char string_pressure[7];
char string_humidity[7];
char string_hum_lvl[10];
char string_temp[7];

int8_t mode = 0;
uint8_t screen = 0;
uint32_t change = 0;
volatile uint8_t btn_pressed = 0;

char string_buf[7];
uint8_t date_buff[6];
uint8_t date_changemask = 0;

// SEBO
HTS221_t hts;
LPS22HB_t lps;

// float temperature, humidity, pressure_hPa;

static float   pressBuf[PRESS_BUF_SIZE];
static uint8_t pressIdx = 0;
static bool    pressFull = false;
static uint32_t lastSampleTick = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int USART_ReadTime();
static float readPressure_hPa(void);
static float pressureTrend_hPa_per_hr(void);
static WeatherFlag simpleForecast(float p_hPa, float rh_percent, float trend_hPa_hr, float tempC);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static float readPressure_hPa(void)
{
    float pressure = LPS22_ReadPressure(&lps);   // pravdepodobne mmHg u teba
    return pressure;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_I2C1_Init();
  MX_TIM7_Init();
  MX_TIM6_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */

  	HAL_Delay(1000);
	USART_ReadTime();
	HAL_TIM_Base_Start_IT(&htim16);


	ILI9341_Reset();
	HAL_Delay(10);
	ILI9341_Init();
	HAL_Delay(50);

	ILI9341_SetRotation(3);
	ILI9341_FillScreen(BGCOLOR);

	HAL_TIM_Base_Start_IT(&htim6);


	float sim_temp = 22.1f;
	float last_temp = -1.0f;

	int16_t sim_humidity = 50;
	int16_t last_humidity = -1;

	int16_t sim_pressure = 1013;
	int16_t last_pressure = -1;

	int16_t weather = 2;

	// simulacia casu
	int base_hours = 14;
	int base_minutes = 30;
	int base_seconds = 45;

	int last_second_val = -1;
	char time_string[10];
	char seconds_string[5];
	char date_string[12] = "21.01.2026";

	DrawDataCentered_WithOffset("Welcome!", FONT4, 2, 65, FCOLOR);
	DrawDataCentered_WithOffset("Weather station created by:", FONT4, 1, 115,
	FCOLOR);
	DrawDataCentered_WithOffset("Bodor, Gavendova,", FONT4, 1,
			115 + FONT4[2] + 1, FCOLOR);
	DrawDataCentered_WithOffset("Kapina, Krajmer", FONT4, 1,
			115 + (FONT4[2]) * 2 + 1, FCOLOR);

	if (mode == 0) {
		DrawDataCentered_WithOffset("*Press the button to stop the screen",
		FONT4, 1, 240 - FONT4[2] - 1, GREEN);
		HAL_Delay(3000);

		ILI9341_FillScreen(BGCOLOR);

		while (1) {
			uint32_t total_s = HAL_GetTick() / 1000;
			int current_s = (base_seconds + total_s) % 60;
			int current_m = (base_minutes + (base_seconds + total_s) / 60) % 60;
			int current_h = (base_hours
					+ (base_minutes + (base_seconds + total_s) / 60) / 60) % 24;

			if (!btn_pressed && (HAL_GetTick() - change >= 5000)) {
				screen++;
				if (screen > 5)
					screen = 0;
				change = HAL_GetTick();

				ILI9341_FillScreen(BGCOLOR);
				last_temp = -1.0f;
				last_humidity = -1;
				last_pressure = -1;

				switch (screen) {
				case 0:
					ILI9341_DrawHLine(0, 120, 320, FCOLOR);
					ILI9341_DrawVLine(160, 0, 240, FCOLOR);
					DrawDataInBox(time_string, seconds_string, date_string,
					FONT4, 2, 1, 1, 0, 0, 1, 1, 1);

					DrawDataInBox(string_temp, "\177C", "", FONT4, 2, 1, 1, 160,
							0, 1, 1, 0);

					DrawDataInBox(string_humidity, "%", string_hum_lvl, FONT4,
							2, 1, 1, 0, 120, 1, 1, 1);

					DrawDataInBox(string_pressure, "hPa", "", FONT4, 2, 1, 1,
							160, 120, 1, 1, 0);

					break;
				case 1:
					DrawDataCentered2(time_string, seconds_string, date_string,
					FONT4, 5, 3, 3, 1, 1, 1);
					break;
				case 2:
					DrawDataCentered2(string_temp, "\177C", "",
					FONT4, 5, 3, 0, 1, 1, 0);
					break;

				case 3:
					if (sim_humidity >= 40 && sim_humidity <= 60) {
						snprintf(string_hum_lvl, sizeof(string_hum_lvl),
								"COMFORT");
					}
					if (sim_humidity < 40) {
						snprintf(string_hum_lvl, sizeof(string_hum_lvl), "DRY");
					}
					if (sim_humidity > 60) {
						snprintf(string_hum_lvl, sizeof(string_hum_lvl),
								"HUMID");
					}
					DrawDataCentered2(string_humidity, "%", string_hum_lvl,
					FONT4, 5, 3, 3, 1, 1, 1);
					break;
					break;
				case 4:
					DrawDataCentered2(string_pressure, "hPa", "",
					FONT4, 5, 3, 0, 1, 1, 0);
					break;
				case 5:
					if (weather == 1) { //oblacno
						DrawCloud(DARKGREY, 3);
					}
					if (weather == 2) { //slnecno
						DrawSun(YELLOW, 3);
					}
					if (weather == 3) { //dazd
						DrawRain(DARKGREY, 3);
					}
					if (weather == 4) { //hmla
						DrawFog(DARKGREY, 3);
					}
					break;
				}

			}

			//dynamic
			if (screen == 0) {
				if (current_s == 0) {

					sprintf(time_string, "%02d:%02d", current_h, current_m);
					sprintf(seconds_string, ":%02d", current_s);

					DrawDataInBox(time_string, seconds_string, date_string,
					FONT4, 2, 1, 1, 0, 0, 1, 0, 0);
				}
				if (current_s != last_second_val) {

					sprintf(time_string, "%02d:%02d", current_h, current_m);
					sprintf(seconds_string, ":%02d", current_s);

					DrawDataInBox(time_string, seconds_string, date_string,
					FONT4, 2, 1, 1, 0, 0, 0, 1, 0);

					last_second_val = current_s;
				}

			}

			if (screen == 1) {
				if (current_s == 0) {

					sprintf(time_string, "%02d:%02d", current_h, current_m);
					sprintf(seconds_string, ":%02d", current_s);

					DrawDataCentered2(time_string, seconds_string, date_string,
					FONT4, 5, 3, 3, 1, 0, 0);
				}
				if (current_s != last_second_val) {

					sprintf(time_string, "%02d:%02d", current_h, current_m);
					sprintf(seconds_string, ":%02d", current_s);

					DrawDataCentered2(time_string, seconds_string, date_string,
					FONT4, 5, 3, 3, 0, 1, 0);

					last_second_val = current_s;
				}

			}

			if (sim_temp != last_temp) {

				sprintf(string_temp, "%.1f", sim_temp);

				if (screen == 2) {
					DrawDataCentered2(string_temp, "\177C", "", FONT4, 5, 3, 0,
							1, 0, 0);
				} else if (screen == 0) {
					DrawDataInBox(string_temp, "\177C", "",
					FONT4, 2, 1, 1, 160, 0, 1, 0, 0);
				}
				last_temp = sim_temp;
			}

			if (sim_humidity != last_humidity) {

				sprintf(string_humidity, "%d", sim_humidity);

				if (screen == 3) {
					DrawDataCentered2(string_humidity, "%", string_hum_lvl,
					FONT4, 5, 3, 3, 1, 0, 0);
				} else if (screen == 0) {
					DrawDataInBox(string_humidity, "%", string_hum_lvl,
					FONT4, 2, 1, 1, 0, 120, 1, 0, 0);
				}
				last_humidity = sim_humidity;
			}

			if (sim_pressure != last_pressure) {

				sprintf(string_pressure, "%d", sim_pressure);

				if (screen == 4) {
					DrawDataCentered2(string_pressure, "hPa", "", FONT4, 5, 3,
							0, 1, 0, 0);
				} else if (screen == 0) {
					DrawDataInBox(string_pressure, "hPa", "",
					FONT4, 2, 1, 1, 160, 120, 1, 0, 0);
				}
				last_pressure = sim_pressure;
			}

		}
}




if (mode == 1) {
	DrawDataCentered_WithOffset("*Press the button to switch screens",
			FONT4, 1, 240 - FONT4[2] - 1, GREEN);
	uint32_t first = 0;
	while (1) {
		uint32_t total_s = HAL_GetTick() / 1000;
		int current_s = (base_seconds + total_s) % 60;
		int current_m = (base_minutes + (base_seconds + total_s) / 60) % 60;
		int current_h = (base_hours
				+ (base_minutes + (base_seconds + total_s) / 60) / 60) % 24;
		if (btn_pressed) {
			screen++;
			if (screen > 5)
			screen = 0;

			ILI9341_FillScreen(BGCOLOR);
			last_temp = -1.0f;
			last_humidity = -1;
			last_pressure = -1;

			first = 1;
			btn_pressed = 0;
			switch (screen) {
				case 0:
				ILI9341_DrawHLine(0, 120, 320, FCOLOR);
				ILI9341_DrawVLine(160, 0, 240, FCOLOR);
				DrawDataInBox(time_string, seconds_string, date_string,
						FONT4, 2, 1, 1, 0, 0, 1, 1, 1);

				DrawDataInBox(string_temp, "\177C", "", FONT4, 2, 1, 1, 160,
						0, 1, 1, 0);

				DrawDataInBox(string_humidity, "%", string_hum_lvl, FONT4,
						2, 1, 1, 0, 120, 1, 1, 1);

				DrawDataInBox(string_pressure, "hPa", "", FONT4, 2, 1, 1,
						160, 120, 1, 1, 0);

				break;
				case 1:
				DrawDataCentered2(time_string, seconds_string, date_string,
						FONT4, 5, 3, 3, 1, 1, 1);
				break;
				case 2:
				DrawDataCentered2(string_temp, "\177C", "",
						FONT4, 5, 3, 0, 1, 1, 0);
				break;

				case 3:
				if (sim_humidity >= 40 && sim_humidity <= 60) {
					snprintf(string_hum_lvl, sizeof(string_hum_lvl),
							"COMFORT");
				}
				if (sim_humidity < 40) {
					snprintf(string_hum_lvl, sizeof(string_hum_lvl), "DRY");
				}
				if (sim_humidity > 60) {
					snprintf(string_hum_lvl, sizeof(string_hum_lvl),
							"HUMID");
				}
				DrawDataCentered2(string_humidity, "%", string_hum_lvl,
						FONT4, 5, 3, 3, 1, 1, 1);
				break;
				break;
				case 4:
				DrawDataCentered2(string_pressure, "hPa", "",
						FONT4, 5, 3, 0, 1, 1, 0);
				break;
				case 5:
				if (weather == 1) { //oblacno
					DrawCloud(DARKGREY, 3);
				}
				if (weather == 2) { //slnecno
					DrawSun(YELLOW, 3);
				}
				if (weather == 3) { //dazd
					DrawRain(DARKGREY, 3);
				}
				if (weather == 4) { //hmla
					DrawFog(DARKGREY, 3);
				}
				break;
			}

		}

		if (first == 1) {
			//dynamic
			if (screen == 0) {
				if (current_s == 0) {

					sprintf(time_string, "%02d:%02d", current_h, current_m);
					sprintf(seconds_string, ":%02d", current_s);

					DrawDataInBox(time_string, seconds_string, date_string,
							FONT4, 2, 1, 1, 0, 0, 1, 0, 0);
				}
				if (current_s != last_second_val) {

					sprintf(time_string, "%02d:%02d", current_h, current_m);
					sprintf(seconds_string, ":%02d", current_s);

					DrawDataInBox(time_string, seconds_string, date_string,
							FONT4, 2, 1, 1, 0, 0, 0, 1, 0);

					last_second_val = current_s;
				}

			}

			if (screen == 1) {
				if (current_s == 0) {

					sprintf(time_string, "%02d:%02d", current_h, current_m);
					sprintf(seconds_string, ":%02d", current_s);

					DrawDataCentered2(time_string, seconds_string,
							date_string,
							FONT4, 5, 3, 3, 1, 0, 0);
				}
				if (current_s != last_second_val) {

					sprintf(time_string, "%02d:%02d", current_h, current_m);
					sprintf(seconds_string, ":%02d", current_s);

					DrawDataCentered2(time_string, seconds_string,
							date_string,
							FONT4, 5, 3, 3, 0, 1, 0);

					last_second_val = current_s;
				}

			}

			if (sim_temp != last_temp) {

				sprintf(string_temp, "%.1f", sim_temp);

				if (screen == 2) {
					DrawDataCentered2(string_temp, "\177C", "", FONT4, 5, 3,
							0, 1, 0, 0);
				} else if (screen == 0) {
					DrawDataInBox(string_temp, "\177C", "",
							FONT4, 2, 1, 1, 160, 0, 1, 0, 0);
				}
				last_temp = sim_temp;
			}

			if (sim_humidity != last_humidity) {

				sprintf(string_humidity, "%d", sim_humidity);

				if (screen == 3) {
					DrawDataCentered2(string_humidity, "%", string_hum_lvl,
							FONT4, 5, 3, 3, 1, 0, 0);
				} else if (screen == 0) {
					DrawDataInBox(string_humidity, "%", string_hum_lvl,
							FONT4, 2, 1, 1, 0, 120, 1, 0, 0);
				}
				last_humidity = sim_humidity;
			}

			if (sim_pressure != last_pressure) {

				sprintf(string_pressure, "%d", sim_pressure);

				if (screen == 4) {
					DrawDataCentered2(string_pressure, "hPa", "", FONT4, 5,
							3, 0, 1, 0, 0);
				} else if (screen == 0) {
					DrawDataInBox(string_pressure, "hPa", "",
							FONT4, 2, 1, 1, 160, 120, 1, 0, 0);
				}
				last_pressure = sim_pressure;
			}

		}
	}
}

hts.hi2c = &hi2c1;
hts.I2C_Read = I2C_Read;
hts.I2C_Write = I2C_Write;

lps.hi2c = &hi2c1;
lps.I2C_Read = I2C_Read;
lps.I2C_Write = I2C_Write;

if (!HTS221_Init(&hts))
{
    const char *error = "HTS221 init failed\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)error, strlen(error), HAL_MAX_DELAY);
}

if (!LPS22_Init(&lps))
{
    const char *error = "LPS init failed\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)error, strlen(error), HAL_MAX_DELAY);
}

HAL_Delay(200);

// Referenčný tlak pri štarte (v hPa)
float referencePressure = readPressure_hPa();
printf("Referencny tlak pri starte: %.2f hPa\r\n", referencePressure);

lastSampleTick = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
while (1) {

	temperature = HTS221_ReadTemperature(&hts);
	      humidity    = HTS221_ReadHumidity(&hts);
	      pressure_hPa = readPressure_hPa();

	      // Log každú sekundu: T, RH, p
	      printf("%.1f, %.0f, %.1f\r\n", temperature, humidity, pressure_hPa);

	      // Raz za SAMPLE_PERIOD_S urob vzorku tlaku do buffra a (keď je buffer plný) pošli WX flag
	      if (HAL_GetTick() - lastSampleTick >= (SAMPLE_PERIOD_S * 1000UL))
	      {
	          lastSampleTick = HAL_GetTick();

	          pressBuf[pressIdx] = pressure_hPa;
	          pressIdx = (pressIdx + 1) % PRESS_BUF_SIZE;


	              float trend = pressureTrend_hPa_per_hr();
	              WeatherFlag wx = simpleForecast(pressure_hPa, humidity, trend, temperature);

	              // WX je flag predpovede
	              printf("WX=%d, trend=%.2f hPa/h\r\n", (int)wx, trend);

	      }

	      HAL_Delay(1000);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//if (mode == 0) {
//	  if (screen_status) {
//		  if (screen_index >= 3) {
//			  screen_index = 0;
//		  }
//		  if (screen_index == 0) {
//			  ILI9341_FillScreen(BLUE);
//		  }
//		  else if (screen_index == 1) {
//			  ILI9341_FillScreen(RED);
//		  }
//		  else if (screen_index == 2) {
//			  ILI9341_FillScreen(WHITE);
//		  }
//		  screen_status = 0;
//		  screen_index++;
//	  }
//}
//if (mode == 1) {
//	if (screen_tick) {
//		if (screen_index >= 3) {
//			screen_index = 0;
//		}
//		if (screen_index == 0) {
//		    ILI9341_FillScreen(BLUE);
//		}
//		else if (screen_index == 1) {
//		    ILI9341_FillScreen(RED);
//		}
//		else if (screen_index == 2) {
//		    ILI9341_FillScreen(WHITE);
//		}
//		screen_tick = 0;
//		screen_index++;
//	}
//}
	/*HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	 HAL_Delay(500);
	 HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	 HAL_Delay(500);*/

}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == BTN_Pin) {
		btn_pressed = !btn_pressed;
		HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
	}
}

// odošleme žiadosť na dektop apku pomocou USART portu a čakáme na odpoveď
int USART_ReadTime(){
  uint8_t msg[] = "get\0";

  HAL_UART_Transmit(&huart2, msg, sizeof(msg) - 1, HAL_MAX_DELAY);
  uint8_t rx;

  for(int i = 0; i < 6; i++){
    HAL_UART_Receive(&huart2, &rx, 1, HAL_MAX_DELAY);
    date_buff[i] = rx;
  }
  char buff[20];
  	uint16_t year = 1900;
  	year += date_buff[5];
	sprintf(buff, "%02d:%02d:%02d %02d.%02d.%04d", date_buff[0],date_buff[1],date_buff[2],date_buff[3],date_buff[4],year);
	HAL_UART_Transmit(&huart2, buff, sizeof(buff) - 1, HAL_MAX_DELAY);


  return 0;
}

// time increase
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim16){
		//HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);


		date_buff[2]++;
		date_changemask |= 0x8;
		if(date_buff[2] > 59){
			date_buff[2] = 0;
			date_buff[1]++;
			date_changemask |= 0x10;
		}

		if(date_buff[1] > 59){
			date_buff[1] = 0;
			date_buff[0]++;
			date_changemask |= 0x20;
		}

		char buff[20];
		uint16_t year = 1900;
		year += date_buff[5];

		sprintf(buff, "%02d:%02d:%02d %02d.%02d.%04d", date_buff[0],date_buff[1],date_buff[2],date_buff[3],date_buff[4],year);
		HAL_UART_Transmit(&huart2, buff, sizeof(buff) - 1, HAL_MAX_DELAY);
	}
}

// SEBO

int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

// Trend tlaku v hPa/h, počítaný cez celé okno buffra (napr. ~10 min)
static float pressureTrend_hPa_per_hr(void)
{
    if (!pressFull) return 0.0f;

    // newest = posledne zapisana vzorka
    uint8_t newest = (pressIdx + PRESS_BUF_SIZE - 1) % PRESS_BUF_SIZE;
    // oldest = pressIdx (keď je buffer full, pressIdx ukazuje na najstaršiu vzorku)
    uint8_t oldest = pressIdx;

    float p_new = pressBuf[newest];
    float p_old = pressBuf[oldest];

    float hours = (PRESS_BUF_SIZE * SAMPLE_PERIOD_S) / 3600.0f;
    if (hours <= 0.0f) return 0.0f;

    return (p_new - p_old) / hours;
}

static WeatherFlag simpleForecast(float p_hPa, float rh_percent, float trend_hPa_hr, float tempC)
{
    // 1) Búrka: veľmi rýchly pokles tlaku
    if (trend_hPa_hr < -6.0f) return WX_STORM;

    // 2) Dážď: tlak klesá + vlhkosť je vyššia
    if (trend_hPa_hr < -3.0f && rh_percent > 70.0f) return WX_RAIN;

    // 3) Hmla: veľmi vysoká vlhkosť + chladno + tlak takmer stabilný
    if (rh_percent > 92.0f && tempC < 8.0f && fabsf(trend_hPa_hr) < 1.0f) return WX_FOG;

    // 4) Slnečno: tlak rastie, alebo tlak je vysoký a podmienky sú stabilné
    if (trend_hPa_hr > 2.0f) return WX_SUNNY;
    if (p_hPa > 1018.0f && trend_hPa_hr > -1.0f && rh_percent < 75.0f) return WX_SUNNY;

    // 5) Inak zamračené
    return WX_CLOUDY;
}

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
while (1) {
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
