/*
 * text_function.c
 *
 *  Created on: Apr 15, 2025
 *
 *  File for function to convert digit to ascii
 */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "text_function.h"

/* USER CODE END Includes */


/* INT to CHAR functions--------------------------------------------------------*/
/* USER CODE BEGIN 1 */
void digit3_to_ascii(uint16_t value, char *buffer)
{
	buffer[0] = (value / 100) + '0';
	buffer[1] = ((value % 100) / 10) + '0';
	buffer[2] = (value % 10) + '0';
	buffer[3] = '\0';
}

void digit_to_ascii_X_XXX(uint16_t value, char *buffer)
{
	buffer[0] = (value / 1000) + '0';
	buffer[1] = '.';
	buffer[2] = ((value % 1000) / 100) + '0';
	buffer[3] = ((value % 100) / 10) + '0';
	buffer[4] = (value % 10) + '0';
	buffer[5] = '\0';
}

void digit_to_ascii_XX_X(uint16_t value, char *buffer)
{
	buffer[0] = (value / 100) + '0';
	buffer[1] = ((value % 100) / 10) + '0';
	buffer[2] = '.';
	buffer[3] = (value % 10) + '0';
	buffer[4] = '\0';
}

void digit_to_ascii_XX_XX(uint16_t value, char *buffer) {
	buffer[0] = (value / 1000) + '0';
	buffer[1] = ((value % 1000) / 100) + '0';
	buffer[2] = '.';
	buffer[3] = ((value % 100) / 10) + '0';
	buffer[4] = (value % 10) + '0';
	buffer[5] = '\0';
}

void digit_to_ascii_XXX_X(uint16_t value, char *buffer)
{
	buffer[0] = (value / 1000) + '0';
	buffer[1] = ((value % 1000) / 100) + '0';
	buffer[2] = ((value % 100) / 10) + '0';
	buffer[3] = '.';
	buffer[4] = (value % 10) + '0';
	buffer[5] = '\0';
}
/*
void digit5_to_ascii(uint16_t value, char *buffer)
{
	buffer[0] = (value / 10000) + '0';
	buffer[1] = ((value % 10000) / 1000) + '0';
	buffer[2] = ((value % 1000) / 100) + '0';
	buffer[3] = ((value % 100) / 10) + '0';
	buffer[4] = (value % 10) + '0';
	buffer[5] = '\0';
}
*/

/*
void digit3_to_ascii_int(int8_t value, char *buffer)
{
	buffer[0] = (value >= 0) ? '+' : '-';
	value = value >= 0 ? value : -value;
	buffer[1] = (value / 100) + '0';
	buffer[2] = ((value % 100) / 10) + '0';
	buffer[3] = (value % 10) + '0';
	buffer[4] = '\0';
}
*/
/* USER CODE END 1 */
