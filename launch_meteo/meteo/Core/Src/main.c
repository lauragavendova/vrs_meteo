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

// Nastav na 1 ak máš desktop aplikáciu, 0 ak testuješ bez nej
#define USE_DESKTOP_APP  0

// Nastav na 1 ak chceš inicializovať SENZORY PRED DISPLEJOM (odporúčané)
// Nastav na 0 ak chceš DISPLEJ PRED SENZORMI s I2C reinicializáciou
#define SENSORS_BEFORE_DISPLAY  1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int8_t screen_index = 0;
int8_t screen_status = 0;
int8_t screen_tick = 0;
int32_t butonCounter = 0;

char string_pressure[7];
char string_humidity[7];
char string_hum_lvl[10];
char string_temp[7];

int8_t mode = 1;
int8_t redraw = 0;
uint8_t screen = 0;
uint32_t change = 0;
volatile uint8_t btn_pressed = 0;

char string_buf[7];
uint8_t date_buff[6];
uint8_t date_changemask = 0;

// SEBO
HTS221_t hts;
LPS22HB_t lps;

float temperature = 0.0f, humidity = 0.0f, pressure_hPa = 0.0f;

static float   pressBuf[PRESS_BUF_SIZE];
static uint8_t pressIdx = 0;
static bool    pressFull = false;
static uint32_t lastSampleTick = 0;
static uint32_t lastSensorRead = 0;

// Pre display
float display_temp = 0.0f;
float display_humidity = 0.0f;
float display_pressure = 0.0f;
WeatherFlag current_weather = WX_SUNNY;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int USART_ReadTime();
static float readPressure_hPa(void);
static float pressureTrend_hPa_per_hr(void);
static WeatherFlag simpleForecast(float p_hPa, float rh_percent, float trend_hPa_hr, float tempC);
static void updateSensorData(void);
static void updateWeatherForecast(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static float readPressure_hPa(void)
{
    float pressure = LPS22_ReadPressure(&lps);
    return pressure;
}

static void updateSensorData(void)
{
    temperature = HTS221_ReadTemperature(&hts);
    humidity = HTS221_ReadHumidity(&hts);
    pressure_hPa = readPressure_hPa();

    // Aktualizuj display hodnoty
    display_temp = temperature;
    display_humidity = humidity;
    display_pressure = pressure_hPa;

    // Log každú sekundu: T, RH, p
    printf("%.1f, %.0f, %.1f\r\n", temperature, humidity, pressure_hPa);
}

static void updateWeatherForecast(void)
{
    // Raz za SAMPLE_PERIOD_S urob vzorku tlaku do buffra
    if (HAL_GetTick() - lastSampleTick >= (SAMPLE_PERIOD_S * 1000UL))
    {
        lastSampleTick = HAL_GetTick();

        pressBuf[pressIdx] = pressure_hPa;
        pressIdx = (pressIdx + 1) % PRESS_BUF_SIZE;

        // Po naplnení buffra nastav flag
        if (pressIdx == 0 && !pressFull) {
            pressFull = true;
        }

        // Keď je buffer plný, vypočítaj trend a predpoveď
        if (pressFull)
        {
            float trend = pressureTrend_hPa_per_hr();
            current_weather = simpleForecast(pressure_hPa, humidity, trend, temperature);

            // WX je flag predpovede
            printf("WX=%d, trend=%.2f hPa/h\r\n", (int)current_weather, trend);
        }
    }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
	MX_I2C1_Init();        // ← I2C MUSÍ BYŤ PRED SPI!
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_SPI1_Init();        // ← SPI až PO I2C
	MX_TIM7_Init();
	MX_TIM6_Init();
	MX_TIM16_Init();
	/* USER CODE BEGIN 2 */

	// ***** 1. ČASOVANIE *****
	HAL_Delay(1000);

#if USE_DESKTOP_APP
	USART_ReadTime();
#else
	// Nastav default čas ak nemáš desktop aplikáciu
	date_buff[0] = 12; // hodiny
	date_buff[1] = 0;  // minúty
	date_buff[2] = 0;  // sekundy
	date_buff[3] = 28;  // deň
	date_buff[4] = 1;  // mesiac (január)
	date_buff[5] = 126; // rok (2026 - 1900)
	const char *msg = "Using default time (no desktop app)\r\n";
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
#endif

#if SENSORS_BEFORE_DISPLAY
	// ========== VARIANT A: SENZORY PRED DISPLEJOM (odporúčané) ==========
	const char *sensor_init_msg = "Init: SENSORS first, then display\r\n";
	HAL_UART_Transmit(&huart2, (uint8_t*)sensor_init_msg, strlen(sensor_init_msg), HAL_MAX_DELAY);

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
	else
	{
	    const char *ok = "HTS221 OK\r\n";
	    HAL_UART_Transmit(&huart2, (uint8_t*)ok, strlen(ok), HAL_MAX_DELAY);
	}

	if (!LPS22_Init(&lps))
	{
	    const char *error = "LPS init failed\r\n";
	    HAL_UART_Transmit(&huart2, (uint8_t*)error, strlen(error), HAL_MAX_DELAY);
	}
	else
	{
	    const char *ok = "LPS22 OK\r\n";
	    HAL_UART_Transmit(&huart2, (uint8_t*)ok, strlen(ok), HAL_MAX_DELAY);
	}

	HAL_Delay(200);

	// Prvé čítanie senzorov
	updateSensorData();

	printf("Reference pressure: %.2f hPa\r\n", pressure_hPa);
	printf("Temperature: %.1f C\r\n", temperature);
	printf("Humidity: %.0f %%\r\n\r\n", humidity);

	lastSampleTick = HAL_GetTick();
	lastSensorRead = HAL_GetTick();

	// TERAZ DISPLEJ
	const char *display_msg = "Initializing display...\r\n";
	HAL_UART_Transmit(&huart2, (uint8_t*)display_msg, strlen(display_msg), HAL_MAX_DELAY);

	ILI9341_Reset();
	HAL_Delay(10);
	ILI9341_Init();
	HAL_Delay(50);

	ILI9341_SetRotation(3);
	ILI9341_FillScreen(BGCOLOR);

#else
	// ========== VARIANT B: DISPLEJ PRED SENZORMI + I2C REINIT ==========
	const char *display_init_msg = "Init: DISPLAY first, then sensors\r\n";
	HAL_UART_Transmit(&huart2, (uint8_t*)display_init_msg, strlen(display_init_msg), HAL_MAX_DELAY);

	ILI9341_Reset();
	HAL_Delay(10);
	ILI9341_Init();
	HAL_Delay(50);

	ILI9341_SetRotation(3);
	ILI9341_FillScreen(BGCOLOR);


	// TERAZ SENZORY
	hts.hi2c = &hi2c1;
	hts.I2C_Read = I2C_Read;
	hts.I2C_Write = I2C_Write;

	lps.hi2c = &hi2c1;
	lps.I2C_Read = I2C_Read;
	lps.I2C_Write = I2C_Write;

	HAL_Delay(200);

	if (!HTS221_Init(&hts))
	{
	    const char *error = "HTS221 init failed\r\n";
	    HAL_UART_Transmit(&huart2, (uint8_t*)error, strlen(error), HAL_MAX_DELAY);
	}
	else
	{
	    const char *ok = "HTS221 OK\r\n";
	    HAL_UART_Transmit(&huart2, (uint8_t*)ok, strlen(ok), HAL_MAX_DELAY);
	}

	if (!LPS22_Init(&lps))
	{
	    const char *error = "LPS init failed\r\n";
	    HAL_UART_Transmit(&huart2, (uint8_t*)error, strlen(error), HAL_MAX_DELAY);
	}
	else
	{
	    const char *ok = "LPS22 OK\r\n";
	    HAL_UART_Transmit(&huart2, (uint8_t*)ok, strlen(ok), HAL_MAX_DELAY);
	}

	HAL_Delay(200);

	// Prvé čítanie senzorov
	updateSensorData();

	printf("Reference pressure: %.2f hPa\r\n", pressure_hPa);
	printf("Temperature: %.1f C\r\n", temperature);
	printf("Humidity: %.0f %%\r\n\r\n", humidity);

	lastSampleTick = HAL_GetTick();
	lastSensorRead = HAL_GetTick();
#endif

	// ***** SPOLOČNÉ PRE OBE VARIANTY *****
#if USE_DESKTOP_APP
	HAL_TIM_Base_Start_IT(&htim16);
#endif

	const char *ready_msg = "System ready!\r\n\r\n";
	HAL_UART_Transmit(&huart2, (uint8_t*)ready_msg, strlen(ready_msg), HAL_MAX_DELAY);

	// ***** WELCOME SCREEN *****
	int last_second_val = -1;
	char time_string[10];
	char seconds_string[5];
	char date_string[12];

	float last_temp = -1.0f;
	int16_t last_humidity = -1;
	int16_t last_pressure = -1;

	DrawDataCentered_WithOffset("Welcome!", FONT4, 2, 65, FCOLOR);
	DrawDataCentered_WithOffset("Weather station created by:", FONT4, 1, 115, FCOLOR);
	DrawDataCentered_WithOffset("Bodor, Gavendova,", FONT4, 1, 115 + FONT4[2] + 1, FCOLOR);
	DrawDataCentered_WithOffset("Kapina, Krajmer", FONT4, 1, 115 + (FONT4[2]) * 2 + 1, FCOLOR);

	if (mode == 1) {
		DrawDataCentered_WithOffset("*Press the button to switch screens",
		FONT4, 1, 240 - FONT4[2] - 1, GREEN);
	} else {
		DrawDataCentered_WithOffset("*Press the button to stop the screen",
		FONT4, 1, 240 - FONT4[2] - 1, GREEN);
	}

	ILI9341_FillScreen(BGCOLOR);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	if (mode == 0) {
		while (1) {
			if (HAL_GetTick() - lastSensorRead >= 1000) {
				lastSensorRead = HAL_GetTick();
				updateSensorData();
				updateWeatherForecast();

#if !USE_DESKTOP_APP
				// Ak nemáme desktop app, inkrementuj čas manuálne
				date_buff[2]++;
				if (date_buff[2] > 59) {
					date_buff[2] = 0;
					date_buff[1]++;
				}
				if (date_buff[1] > 59) {
					date_buff[1] = 0;
					date_buff[0]++;
				}
				if (date_buff[0] > 23) {
					date_buff[0] = 0;
				}
#endif
			}

			int current_s = date_buff[2];
			int year = 1900 + date_buff[5];
			sprintf(date_string, "%02d.%02d.%04d", date_buff[3], date_buff[4], year);

			if ((int)display_humidity >= 40 && (int)display_humidity <= 60) {
				snprintf(string_hum_lvl, sizeof(string_hum_lvl), "COMFORT");
			} else if ((int)display_humidity < 40) {
				snprintf(string_hum_lvl, sizeof(string_hum_lvl), "DRY");
			} else {
				snprintf(string_hum_lvl, sizeof(string_hum_lvl), "HUMID");
			}

			if (!btn_pressed && (HAL_GetTick() - change >= 5000)) {
				screen++;
				if (screen > 5) screen = 0;
				change = HAL_GetTick();

				ILI9341_FillScreen(BGCOLOR);
				last_temp = -1.0f;
				last_humidity = -1;
				last_pressure = -1;

				switch (screen) {
				case 0:
					sprintf(time_string, "%02d:%02d", date_buff[0], date_buff[1]);
					sprintf(seconds_string, ":%02d", date_buff[2]);
					sprintf(string_temp, "%.1f", display_temp);
					sprintf(string_humidity, "%.0f", display_humidity);
					sprintf(string_pressure, "%.0f", display_pressure);

					ILI9341_DrawHLine(0, 120, 320, FCOLOR);
					ILI9341_DrawVLine(160, 0, 240, FCOLOR);
					DrawDataInBox(time_string, seconds_string, date_string, FONT4, 2, 1, 1, 0, 0, 1, 1, 1);
					DrawDataInBox(string_temp, "\177C", "", FONT4, 2, 1, 1, 160, 0, 1, 1, 0);
					DrawDataInBox(string_humidity, "%", string_hum_lvl, FONT4, 2, 1, 1, 0, 120, 1, 1, 1);
					DrawDataInBox(string_pressure, "hPa", "", FONT4, 2, 1, 1, 160, 120, 1, 1, 0);
					break;

				case 1:
					sprintf(time_string, "%02d:%02d", date_buff[0], date_buff[1]);
					sprintf(seconds_string, ":%02d", date_buff[2]);
					DrawDataCentered2(time_string, seconds_string, date_string, FONT4, 5, 3, 3, 1, 1, 1);
					break;

				case 2:
					sprintf(string_temp, "%.1f", display_temp);
					DrawDataCentered2(string_temp, "\177C", "", FONT4, 5, 3, 0, 1, 1, 0);
					break;

				case 3:
					sprintf(string_humidity, "%.0f", display_humidity);
					DrawDataCentered2(string_humidity, "%", string_hum_lvl, FONT4, 5, 3, 3, 1, 1, 1);
					break;

				case 4:
					sprintf(string_pressure, "%.0f", display_pressure);
					DrawDataCentered2(string_pressure, "hPa", "", FONT4, 5, 3, 0, 1, 1, 0);
					break;

				case 5:
					if (current_weather == WX_CLOUDY) {
						DrawCloud(LIGHTGREY, 3);
					} else if (current_weather == WX_SUNNY) {
						DrawSun(YELLOW, 3);
					} else if (current_weather == WX_RAIN) {
						DrawRain(LIGHTGREY, 3);
					} else if (current_weather == WX_FOG) {
						DrawFog(LIGHTGREY, 3);
					}
					break;
				}
			}

			// dynamic
			if (screen == 0) {
				if (current_s != last_second_val) {
					sprintf(time_string, "%02d:%02d", date_buff[0], date_buff[1]);
					sprintf(seconds_string, ":%02d", date_buff[2]);
					redraw = (current_s == 0) ? 1 : 0;
					DrawDataInBox(time_string, seconds_string, date_string, FONT4, 2, 1, 1, 0, 0, redraw, 1, 0);
					last_second_val = current_s;
				}
			}

			if (screen == 1) {
				if (current_s != last_second_val) {
					sprintf(time_string, "%02d:%02d", date_buff[0], date_buff[1]);
					sprintf(seconds_string, ":%02d", date_buff[2]);
					redraw = (current_s == 0) ? 1 : 0;
					DrawDataCentered2(time_string, seconds_string, date_string, FONT4, 5, 3, 3, redraw, 1, 0);
					last_second_val = current_s;
				}
			}

			if (display_temp != last_temp) {
				sprintf(string_temp, "%.1f", display_temp);
				if (screen == 2) {
					DrawDataCentered2(string_temp, "\177C", "", FONT4, 5, 3, 0, 1, 0, 0);
				} else if (screen == 0) {
					DrawDataInBox(string_temp, "\177C", "", FONT4, 2, 1, 1, 160, 0, 1, 0, 0);
				}
				last_temp = display_temp;
			}

			if ((int)display_humidity != last_humidity) {
				sprintf(string_humidity, "%.0f", display_humidity);
				if (screen == 3) {
					DrawDataCentered2(string_humidity, "%", string_hum_lvl, FONT4, 5, 3, 3, 1, 0, 0);
				} else if (screen == 0) {
					DrawDataInBox(string_humidity, "%", string_hum_lvl, FONT4, 2, 1, 1, 0, 120, 1, 0, 0);
				}
				last_humidity = (int)display_humidity;
			}

			if ((int)display_pressure != last_pressure) {
				sprintf(string_pressure, "%.0f", display_pressure);
				if (screen == 4) {
					DrawDataCentered2(string_pressure, "hPa", "", FONT4, 5, 3, 0, 1, 0, 0);
				} else if (screen == 0) {
					DrawDataInBox(string_pressure, "hPa", "", FONT4, 2, 1, 1, 160, 120, 1, 0, 0);
				}
				last_pressure = (int)display_pressure;
			}
		}
	}

	if (mode == 1) {
		DrawDataCentered_WithOffset("*Press the button to switch screens",
		FONT4, 1, 240 - FONT4[2] - 1, GREEN);

		uint32_t first = 0;
		while (1) {
			// Čítaj senzory každú sekundu
			if (HAL_GetTick() - lastSensorRead >= 1000) {
				lastSensorRead = HAL_GetTick();
				updateSensorData();
				updateWeatherForecast();
			}

			int current_s = date_buff[2];
			int year = 1900 + date_buff[5];
			sprintf(date_string, "%02d.%02d.%04d", date_buff[3], date_buff[4], year);


			if ((int)display_humidity >= 40 && (int)display_humidity <= 60) {
				snprintf(string_hum_lvl, sizeof(string_hum_lvl), "COMFORT");
			} else if ((int)display_humidity < 40) {
				snprintf(string_hum_lvl, sizeof(string_hum_lvl), "DRY");
			} else {
				snprintf(string_hum_lvl, sizeof(string_hum_lvl), "HUMID");
			}

			if (btn_pressed) {
				screen++;
				if (screen > 5) screen = 0;

				ILI9341_FillScreen(BGCOLOR);
				last_temp = -1.0f;
				last_humidity = -1;
				last_pressure = -1;

				first = 1;
				btn_pressed = 0;

				switch (screen) {
				case 0:
					sprintf(time_string, "%02d:%02d", date_buff[0], date_buff[1]);
					sprintf(seconds_string, ":%02d", date_buff[2]);
					sprintf(string_temp, "%.1f", display_temp);
					sprintf(string_humidity, "%.0f", display_humidity);
					sprintf(string_pressure, "%.0f", display_pressure);

					ILI9341_DrawHLine(0, 120, 320, FCOLOR);
					ILI9341_DrawVLine(160, 0, 240, FCOLOR);
					DrawDataInBox(time_string, seconds_string, date_string, FONT4, 2, 1, 1, 0, 0, 1, 1, 1);
					DrawDataInBox(string_temp, "\177C", "", FONT4, 2, 1, 1, 160, 0, 1, 1, 0);
					DrawDataInBox(string_humidity, "%", string_hum_lvl, FONT4, 2, 1, 1, 0, 120, 1, 1, 1);
					DrawDataInBox(string_pressure, "hPa", "", FONT4, 2, 1, 1, 160, 120, 1, 1, 0);
					break;

				case 1:
					sprintf(time_string, "%02d:%02d", date_buff[0], date_buff[1]);
					sprintf(seconds_string, ":%02d", date_buff[2]);
					DrawDataCentered2(time_string, seconds_string, date_string, FONT4, 5, 3, 3, 1, 1, 1);
					break;

				case 2:
					sprintf(string_temp, "%.1f", display_temp);
					DrawDataCentered2(string_temp, "\177C", "", FONT4, 5, 3, 0, 1, 1, 0);
					break;

				case 3:
					sprintf(string_humidity, "%.0f", display_humidity);
					DrawDataCentered2(string_humidity, "%", string_hum_lvl, FONT4, 5, 3, 3, 1, 1, 1);
					break;

				case 4:
					sprintf(string_pressure, "%.0f", display_pressure);
					DrawDataCentered2(string_pressure, "hPa", "", FONT4, 5, 3, 0, 1, 1, 0);
					break;

				case 5:
					if (current_weather == WX_CLOUDY) {
						DrawCloud(LIGHTGREY, 3);
					} else if (current_weather == WX_SUNNY) {
						DrawSun(YELLOW, 3);
					} else if (current_weather == WX_RAIN) {
						DrawRain(LIGHTGREY, 3);
					}
					else if (current_weather == WX_FOG) {
						DrawFog(LIGHTGREY, 3);
					}
					break;
				}
			}

			if (first == 1) {
				// dynamic
				if (screen == 0) {
					if (current_s != last_second_val) {
						sprintf(time_string, "%02d:%02d", date_buff[0], date_buff[1]);
						sprintf(seconds_string, ":%02d", date_buff[2]);
						redraw = (current_s == 0) ? 1 : 0;
						DrawDataInBox(time_string, seconds_string, date_string, FONT4, 2, 1, 1, 0, 0, redraw, 1, 0);
						last_second_val = current_s;
					}
				}

				if (screen == 1) {
					if (current_s != last_second_val) {
						sprintf(time_string, "%02d:%02d", date_buff[0], date_buff[1]);
						sprintf(seconds_string, ":%02d", date_buff[2]);
						redraw = (current_s == 0) ? 1 : 0;
						DrawDataCentered2(time_string, seconds_string, date_string, FONT4, 5, 3, 3, redraw, 1, 0);
						last_second_val = current_s;
					}
				}

				if (display_temp != last_temp) {
					sprintf(string_temp, "%.1f", display_temp);
					if (screen == 2) {
						DrawDataCentered2(string_temp, "\177C", "", FONT4, 5, 3, 0, 1, 0, 0);
					} else if (screen == 0) {
						DrawDataInBox(string_temp, "\177C", "", FONT4, 2, 1, 1, 160, 0, 1, 0, 0);
					}
					last_temp = display_temp;
				}

				if ((int)display_humidity != last_humidity) {
					sprintf(string_humidity, "%.0f", display_humidity);
					if (screen == 3) {
						DrawDataCentered2(string_humidity, "%", string_hum_lvl, FONT4, 5, 3, 3, 1, 0, 0);
					} else if (screen == 0) {
						DrawDataInBox(string_humidity, "%", string_hum_lvl, FONT4, 2, 1, 1, 0, 120, 1, 0, 0);
					}
					last_humidity = (int)display_humidity;
				}

				if ((int)display_pressure != last_pressure) {
					sprintf(string_pressure, "%.0f", display_pressure);
					if (screen == 4) {
						DrawDataCentered2(string_pressure, "hPa", "", FONT4, 5, 3, 0, 1, 0, 0);
					} else if (screen == 0) {
						DrawDataInBox(string_pressure, "hPa", "", FONT4, 2, 1, 1, 160, 120, 1, 0, 0);
					}
					last_pressure = (int)display_pressure;
				}
			}
		}
	}

	/* USER CODE END WHILE */

	/* USER CODE BEGIN 3 */
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
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
int USART_ReadTime() {
	uint8_t msg[] = "get\0";

	HAL_UART_Transmit(&huart2, msg, sizeof(msg) - 1, HAL_MAX_DELAY);
	uint8_t rx;

	// Použijeme timeout 1000ms namiesto HAL_MAX_DELAY
	// Ak desktop app nebeží, nebudeme čakať navždy
	HAL_StatusTypeDef status;
	for (int i = 0; i < 6; i++) {
		status = HAL_UART_Receive(&huart2, &rx, 1, 1000); // 1 sekunda timeout
		if (status == HAL_OK) {
			date_buff[i] = rx;
		} else {
			date_buff[0] = 12; // hodiny
			date_buff[1] = 0;  // minúty
			date_buff[2] = 0;  // sekundy
			date_buff[3] = 1;  // deň
			date_buff[4] = 1;  // mesiac
			date_buff[5] = 125; // rok (2025 - 1900)

			const char *timeout_msg = "USART timeout - using default time\r\n";
			HAL_UART_Transmit(&huart2, (uint8_t*)timeout_msg, strlen(timeout_msg), HAL_MAX_DELAY);
			return -1;
		}
	}

	// Ak sme úspešne dostali dáta, pošli potvrdenie
	char buff[20];
	uint16_t year = 1900;
	year += date_buff[5];
	sprintf(buff, "%02d:%02d:%02d %02d.%02d.%04d", date_buff[0], date_buff[1],
			date_buff[2], date_buff[3], date_buff[4], year);
	HAL_UART_Transmit(&huart2, buff, sizeof(buff) - 1, HAL_MAX_DELAY);

	return 0;
}

// time increase
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim16) {
		date_buff[2]++;
		date_changemask |= 0x8;
		if (date_buff[2] > 59) {
			date_buff[2] = 0;
			date_buff[1]++;
			date_changemask |= 0x10;
		}

		if (date_buff[1] > 59) {
			date_buff[1] = 0;
			date_buff[0]++;
			date_changemask |= 0x20;
		}

		char buff[20];
		uint16_t year = 1900;
		year += date_buff[5];

		sprintf(buff, "%02d:%02d:%02d %02d.%02d.%04d", date_buff[0],
				date_buff[1], date_buff[2], date_buff[3], date_buff[4], year);
		HAL_UART_Transmit(&huart2, buff, sizeof(buff) - 1, HAL_MAX_DELAY);
	}
}

// SEBO

int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

// Trend tlaku v hPa/h, počítaný cez celé okno buffra
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
void Error_Handler(void) {
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
