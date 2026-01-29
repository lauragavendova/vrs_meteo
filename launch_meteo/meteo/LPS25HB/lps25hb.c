#include "lps25hb.h"
#include "math.h"
#include <stdio.h>

uint8_t LPS22_Init(LPS22HB_t *dev)
{
    uint8_t whoami = 0;
    HAL_I2C_Mem_Read(dev->hi2c, LPS22_ADDR, LPS22_WHO_AM_I_REG, I2C_MEMADD_SIZE_8BIT, &whoami, 1, 100);

    if (whoami != LPS22_WHO_AM_I_VAL) return 0;

    // Power on + ODR = 1Hz (CTRL_REG1)
    uint8_t ctrl = 0x90; // PD = 1, ODR = 1Hz
    dev->I2C_Write(dev->hi2c, LPS22_ADDR, LPS22_CTRL_REG1, &ctrl, 1);

    HAL_Delay(100);
    return 1;
}

float LPS22_ReadPressure(LPS22HB_t *dev)
{
    uint8_t buf[3];
    dev->I2C_Read(dev->hi2c, LPS22_ADDR, LPS22_PRESS_OUT_XL | 0x80, buf, 3);

    // Poradie bajtov: XL, L, H
    int32_t raw = (int32_t)(buf[0] | (buf[1] << 8) | (buf[2] << 16));

    // Rozšírenie znamienka pre 24-bit signed
    if (raw & 0x00800000) raw |= 0xFF000000;

    // Prevod na hPa
    float pressure_hPa = (float)raw / 4096.0f;
    return pressure_hPa * 1.33322f; // * 1.33322f -> prevod na hPa
}
