#ifndef LPS25HB_H_
#define LPS25HB_H_

#include "stdint.h"
#include "i2c.h"

#define LPS22_ADDR          (0x5D << 1)
#define LPS22_WHO_AM_I_REG  0x0F
#define LPS22_WHO_AM_I_VAL  0xBD
#define LPS22_CTRL_REG1     0x10
#define LPS22_PRESS_OUT_XL  0x28
#define LPS22_ADDR_DEFAULT   (0x5D << 1)

typedef struct {
    HAL_StatusTypeDef (*I2C_Read)(I2C_HandleTypeDef *, uint16_t, uint8_t, uint8_t*, uint16_t);
    HAL_StatusTypeDef (*I2C_Write)(I2C_HandleTypeDef *, uint16_t, uint8_t, uint8_t*, uint16_t);
    I2C_HandleTypeDef *hi2c;
} LPS22HB_t;

uint8_t LPS22_Init(LPS22HB_t *dev);
float LPS22_ReadPressure(LPS22HB_t *dev);
float LPS22Altitude(float pressure_hPa, float seaLevel_hPa);

#endif /* LPS25HB_H_ */
