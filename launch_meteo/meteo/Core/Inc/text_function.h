/*
 * text_function.h
 *
 *  Created on: Apr 15, 2025
 *
 *  Header file for text_function.c
 */

#ifndef INC_TEXT_FUNCTION_H_
#define INC_TEXT_FUNCTION_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* USER CODE BEGIN EFP */
void digit3_to_ascii(uint16_t value, char *buffer);
void digit_to_ascii_X_XXX(uint16_t value, char *buffer);
void digit_to_ascii_XX_X(uint16_t value, char *buffer);
void digit_to_ascii_XX_XX(uint16_t value, char *buffer);
void digit_to_ascii_XXX_X(uint16_t value, char *buffer);
//static void digit3_to_ascii_int(int8_t value, char *buffer);
//static void digit5_to_ascii(uint16_t value, char *buffer);

/* USER CODE END EFP */

#endif /* INC_TEXT_FUNCTION_H_ */
