#include "system.h"
#include <stdio.h>

// 在某个 .c 文件中定义
uint8_t RX_Data_1;
uint8_t RX_Data_2;
uint8_t RX_Data_3;
uint8_t RX_Data_4;
uint8_t RX_Data_5;
uint8_t RX_Data_6;

volatile uint8_t g_ds_ready = 0;
volatile int32_t g_ds_temp_centi_c = 0;
volatile int32_t g_ds_press_centi_mbar = 0;
volatile int32_t g_ds_depth_mm = 0;
volatile uint16_t g_ds_i2c_addr = 0;
uint16_t g_ds_prom[8] = {0};
volatile float ms5837_pressure_mbar = 0.0f;
volatile float ms5837_temperature_c = 0.0f;
volatile float ms5837_depth_m = 0.0f;
volatile uint8_t ms5837_data_valid = 0U;

u32 myabs(const long int a)
{ 		   
	  u32 temp;
	  if(a<0)  temp=-a;
	  else temp=a;
	  return temp;
}

// 重定向 printf 到 USART1
int _write(int file, char *ptr, int len)
{
	(void)file;
	// 使用 HAL 库的阻塞发送函数
	HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
	return len;
}

// 重定向输入（可选）
int fgetc(FILE *f __attribute__((unused)))
{
	while ((USART3->SR & USART_SR_RXNE) == 0);
	return (uint8_t)(USART3->DR & 0xFF);
}
