#ifndef LED01_DEEPSENSORTASK_H
#define LED01_DEEPSENSORTASK_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define MS5837_ADDR_7BIT            0x76u
#define MS5837_CMD_RESET            0x1Eu
#define MS5837_CMD_ADC_READ         0x00u
#define MS5837_CMD_PROM_READ_BASE   0xA0u
#define MS5837_CMD_CONV_D1_OSR4096  0x48u
#define MS5837_CMD_CONV_D2_OSR4096  0x58u

typedef struct {
    uint16_t len;
    uint8_t payload[32];
} DeepSensorMessage;

HAL_StatusTypeDef DeepSensor_Init(void);
void DeepSensor_Task10ms(void);
uint8_t DeepSensor_IsReady(void);
void StartDeepSensorTask(void *argument);

#endif // LED01_DEEPSENSORTASK_H
