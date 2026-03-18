#ifndef LED01_UART_EXECUTE_H
#define LED01_UART_EXECUTE_H

#include <stdint.h>

void UART1_SendBytes(const uint8_t *data, uint16_t len);
void UART1_SendString(const char *str);
void UART1_SendDeepSensorData(int32_t temp_centi_c, int32_t press_centi_mbar, int32_t depth_mm);
void UART1_SendDeepSensorDataFloat(float temperature_c, float pressure_mbar, float depth_m);

#endif // LED01_UART_EXECUTE_H
