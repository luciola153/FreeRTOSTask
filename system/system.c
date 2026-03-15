#include "system.h"
#include <stdio.h>
/* ========== 关闭特定警告 ========== */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
/* ================================== */

// 在某个 .c 文件中定义
uint8_t RX_Data_1;
uint8_t RX_Data_2;
uint8_t RX_Data_3;
uint8_t RX_Data_4;
uint8_t RX_Data_5;
uint8_t RX_Data_6;

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


/* ========== 恢复警告 ========== */
#pragma GCC diagnostic pop
/* ============================== */
