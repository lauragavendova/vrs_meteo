#include "hts221.h"
#include "math.h"
#include "stdio.h"

static HAL_StatusTypeDef HTS221_ReadRegs(HTS221_t *dev, uint8_t reg, uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Read(dev->hi2c, HTS221_ADDR, reg | 0x80, I2C_MEMADD_SIZE_8BIT, data, len, 100);
}

static HAL_StatusTypeDef HTS221_WriteReg(HTS221_t *dev, uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(dev->hi2c, HTS221_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
}

// Inicializácia senzora
uint8_t HTS221_Init(HTS221_t *dev)
{
    uint8_t whoami = 0;
    HTS221_ReadRegs(dev, HTS221_WHO_AM_I_REG, &whoami, 1);
    printf("HTS221 WHO_AM_I register value: 0x%02X\r\n", whoami);

    if (whoami != HTS221_WHO_AM_I_VAL) return 0;

    // Nastavenie presnosti (AVG_T=256, AVG_H=512)
    uint8_t av_conf = 0x1B;
    HTS221_WriteReg(dev, 0x10, av_conf);

    // Zapnutie senzora (PD=1, BDU=1, ODR=1Hz)
    uint8_t ctrl = 0x85;
    HTS221_WriteReg(dev, HTS221_CTRL_REG1, ctrl);

    HAL_Delay(100);
    return 1;
}

// Čítanie kalibračných údajov a výpočet vlhkosti
float HTS221_ReadHumidity(HTS221_t *dev)
{
    uint8_t calib[16];
    uint8_t buf[2];
    HTS221_ReadRegs(dev, 0x30, calib, 16);

    float H0_rH = calib[0] / 2.0f;
    float H1_rH = calib[1] / 2.0f;

    int16_t H0_T0_OUT = (int16_t)(calib[6] | (calib[7] << 8));
    int16_t H1_T0_OUT = (int16_t)(calib[10] | (calib[11] << 8));

    HTS221_ReadRegs(dev, HTS221_HUMIDITY_OUT_L, buf, 2);
    int16_t H_OUT = (int16_t)(buf[0] | (buf[1] << 8));

    float hum = H0_rH + (H1_rH - H0_rH) * (H_OUT - H0_T0_OUT) / (float)(H1_T0_OUT - H0_T0_OUT);
    if (hum > 100.0f) hum = 100.0f;
    if (hum < 0.0f) hum = 0.0f;
    return hum;
}

// Čítanie kalibračných údajov a výpočet teploty
float HTS221_ReadTemperature(HTS221_t *dev)
{
    uint8_t calib[16];
    uint8_t buf[2];
    HTS221_ReadRegs(dev, 0x30, calib, 16);

    uint16_t T0_degC_x8 = calib[2];
    uint16_t T1_degC_x8 = calib[3];
    uint8_t msb = calib[5];

    T0_degC_x8 |= (msb & 0x03) << 8;
    T1_degC_x8 |= ((msb & 0x0C) >> 2) << 8;

    float T0_degC = T0_degC_x8 / 8.0f;
    float T1_degC = T1_degC_x8 / 8.0f;

    int16_t T0_OUT = (int16_t)(calib[12] | (calib[13] << 8));
    int16_t T1_OUT = (int16_t)(calib[14] | (calib[15] << 8));

    HTS221_ReadRegs(dev, HTS221_TEMP_OUT_L, buf, 2);
    int16_t T_OUT = (int16_t)(buf[0] | (buf[1] << 8));

    return T0_degC + (T1_degC - T0_degC) * (T_OUT - T0_OUT) / (float)(T1_OUT - T0_OUT);
}
