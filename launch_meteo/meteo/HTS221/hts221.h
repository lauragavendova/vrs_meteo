#ifndef HTS221_H_
#define HTS221_H_

#include "stdint.h"
#include "i2c.h"

#define HTS221_ADDR          (0x5F << 1)
#define HTS221_WHO_AM_I_REG  0x0F
#define HTS221_WHO_AM_I_VAL  0xBC

#define HTS221_CTRL_REG1     0x20
#define HTS221_AV_CONF       0x10
#define HTS221_HUMIDITY_OUT_L 0x28
#define HTS221_TEMP_OUT_L     0x2A

typedef struct {
    HAL_StatusTypeDef (*I2C_Read)(I2C_HandleTypeDef *, uint16_t, uint8_t, uint8_t*, uint16_t);
    HAL_StatusTypeDef (*I2C_Write)(I2C_HandleTypeDef *, uint16_t, uint8_t, uint8_t*, uint16_t);
    I2C_HandleTypeDef *hi2c;
} HTS221_t;

uint8_t HTS221_Init(HTS221_t *dev);
float HTS221_ReadTemperature(HTS221_t *dev);
float HTS221_ReadHumidity(HTS221_t *dev);

#endif /* HTS221_H_ */
