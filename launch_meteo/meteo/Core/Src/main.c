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
#include "usart.h"
#include "gpio.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "text_function.h"
#include "stm32f3xx_hal.h"
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
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
int16_t temperature = 853;
int16_t pressure = 111;
int8_t screen_index = 0;
int8_t screen_status = 0;
char string_buf[7];

uint8_t screen = 0;
uint32_t change = 0;
volatile uint8_t btn_pressed = 0;

uint32_t mode = 0;

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

  /* USER CODE BEGIN 2 */

  char* big[4] = {"10:00", "22.1", "50", "1013"};
  char* small[4] = {":00", "C", "%", "hPa"};
  char* line2[4] = {"date", "", "OK", ""};

  ILI9341_Reset();
  HAL_Delay(10);
  ILI9341_Init();
  HAL_Delay(50);

  ILI9341_SetRotation(3);
  ILI9341_FillScreen(BGCOLOR);


  if(mode==0){
      DrawDataCentered_WithOffset("Welcome!", FONT4, 2, 75, FCOLOR);
      DrawDataCentered_WithOffset("Creators: Bodor, Gavendova,", FONT4, 1, 125, FCOLOR);
      DrawDataCentered_WithOffset("Kapina, Krajmer", FONT4, 1, 125 + FONT4[2] + 1, FCOLOR);
      DrawDataCentered_WithOffset("*Press the button to stop the screen", FONT4, 1, 240 - FONT4[2] - 1, RED);
      HAL_Delay(3000);

      ILI9341_FillScreen(BGCOLOR);

      uint32_t last_sim_update = 0;
      int16_t sim_pressure = 1010;
      int16_t last_displayed_pressure = -1;

      while (1) {
          if (!btn_pressed && (HAL_GetTick() - change >= 5000)) {
              screen++;
              if (screen > 5) screen = 0;
              change = HAL_GetTick();

              ILI9341_FillScreen(BGCOLOR);

              // Pri prepnutí obrazovky resetujeme "last" hodnotu, aby sa hneď vykreslila nová
              last_displayed_pressure = -1;

              switch (screen) {
                  case 0: DrawSummary(big, small, line2, FONT4);  break;
                  case 1: DrawDataCentered2("10:00", ":00", "date", FONT4, 5, 3, 3); break;
                  case 2: DrawDataCentered2("22.1", "C","", FONT4, 5, 3, 0);    break;
                  case 3: DrawDataCentered2("50", "%", "OK", FONT4, 5, 3, 3);      break;
                  case 4: break;
                  case 5: DrawSun(YELLOW); break;
              }
          }

          if (HAL_GetTick() - last_sim_update >= 500) {
              last_sim_update = HAL_GetTick();
              sim_pressure++;
              if (sim_pressure > 1030) sim_pressure = 990;
          }

          //dynamic

          if (sim_pressure != last_displayed_pressure) {

              sprintf(string_buf, "%d", sim_pressure);

              if (screen == 4) {
                  DrawDataCentered2(string_buf, "hPa", "", FONT4, 5, 3, 0);
              }
              else if (screen == 0) {
                  big[3] = string_buf;
                  DrawSummary(big, small, line2, FONT4);
              }
              last_displayed_pressure = sim_pressure;
          }
      }
    }

  if(mode==1){
    DrawDataCentered_WithOffset("Welcome!", FONT4, 2, 75, FCOLOR);
    DrawDataCentered_WithOffset("Creators: Bodor, Gavendova,", FONT4, 1, 125, FCOLOR);
    DrawDataCentered_WithOffset("Kapina, Krajmer", FONT4, 1, 125 + FONT4[2] + 1, FCOLOR);
    DrawDataCentered_WithOffset("*Press the button to switch screens", FONT4, 1, 240 - FONT4[2] - 1, RED);


    while (1) {
        if (btn_pressed) {
            screen++;
            if (screen > 5) screen = 0;

            ILI9341_FillScreen(BGCOLOR);
            switch (screen) {
                      	  case 0: DrawSummary(big, small, line2, FONT4);  break;
                          case 1: DrawDataCentered2("10:00", ":00", "date", FONT4, 5, 3, 3); break;
                          case 2: DrawDataCentered2("22.1", "C","", FONT4, 5, 3, 0);    break;
                          case 3: DrawDataCentered2("50", "%", "OK", FONT4, 5, 3, 3);      break;
                          case 4: DrawDataCentered2("1013", "hPa","", FONT4, 5, 3,0);  break;
                          case 5: DrawSun(YELLOW); break;
                      }
            }
            btn_pressed = 0;
        }
    }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (screen_status) {
		  pressure++;
		  digit3_to_ascii(pressure, string_buf);
		  ILI9341_DrawText(string_buf, FONT4, 190, 95, WHITE, BLACK);
		  screen_status = 0;
	  }

	  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	  //HAL_Delay(500);
	  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	  //HAL_Delay(500);

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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == BTN_Pin)
  {
	btn_pressed = !btn_pressed;
	HAL_GPIO_TogglePin(GPIOB, LD3_Pin);

  }
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
