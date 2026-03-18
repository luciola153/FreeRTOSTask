#include "uart_execute.h"
#include "usart.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int32_t abs_i32(int32_t v)
{
    return (v < 0) ? -v : v;
}

void UART1_SendBytes(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0U) {
        return;
    }
    HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 100);
}

void UART1_SendString(const char *str)
{
    if (str == NULL) {
        return;
    }
    HAL_UART_Transmit(&huart1, (uint8_t *)str, (uint16_t)strlen(str), 100);
}

void UART1_SendDeepSensorData(int32_t temp_centi_c, int32_t press_centi_mbar, int32_t depth_mm)
{
    char line[128];
    int32_t t_abs = abs_i32(temp_centi_c);
    int32_t p_abs = abs_i32(press_centi_mbar);
    int32_t d_abs = abs_i32(depth_mm);

    snprintf(line, sizeof(line),
             "MS5837 T=%s%ld.%02ldC P=%s%ld.%02ldmbar D=%s%ld.%03ldm\r\n",
             (temp_centi_c < 0) ? "-" : "",
             (long)(t_abs / 100), (long)(t_abs % 100),
             (press_centi_mbar < 0) ? "-" : "",
             (long)(p_abs / 100), (long)(p_abs % 100),
             (depth_mm < 0) ? "-" : "",
             (long)(d_abs / 1000), (long)(d_abs % 1000));

    UART1_SendString(line);
}

void UART1_SendDeepSensorDataFloat(float temperature_c, float pressure_mbar, float depth_m)
{
    char line[128];
    int32_t t_int = (int32_t)temperature_c;
    int32_t p_int = (int32_t)pressure_mbar;
    int32_t d_int = (int32_t)depth_m;
    int32_t t_dec = (int32_t)(fabsf(temperature_c - (float)t_int) * 100.0f);
    int32_t p_dec = (int32_t)(fabsf(pressure_mbar - (float)p_int) * 100.0f);
    int32_t d_dec = (int32_t)(fabsf(depth_m - (float)d_int) * 1000.0f);

    snprintf(line, sizeof(line),
             "MS5837 T=%ld.%02ldC P=%ld.%02ldmbar D=%ld.%03ldm\r\n",
             (long)t_int, (long)t_dec,
             (long)p_int, (long)p_dec,
             (long)d_int, (long)d_dec);
    UART1_SendString(line);
}
