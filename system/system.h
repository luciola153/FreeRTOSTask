#ifndef SYSTEM_H
#define SYSTEM_H

/* ========== 关闭特定警告 ========== */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
/* ================================== */

#define subdivision 16
#define PI 3.14159265358979323846f
#define Log 1

#include "gpio.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include "main.h"
#include "tim.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "semphr.h"

#include "UART_NVIC.h"
#include "sys.h"
#include "EXTI.h"
#include "delay.h"
#include "LEDtype.h"
#include "OLED.h"
#include "OLEDTask.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdarg.h"
#include "stdbool.h"

extern uint8_t RX_Data_1;
extern uint8_t RX_Data_2;
extern uint8_t RX_Data_3;
extern uint8_t RX_Data_4;
extern uint8_t RX_Data_5;
extern uint8_t RX_Data_6;

extern osMessageQueueId_t LEDQueueHandle;
extern osMessageQueueId_t CommandQueueHandle;
extern osSemaphoreId_t KeySemaphoreHandle;
extern osMessageQueueId_t OLEDQueueHandle;

#define configUSE_COUNTING_SEMAPHORES 1           //使用计数型信号量必备宏定义
#define configUSE_MUTEXES 1                      //使用互斥锁必备宏定义


u32 myabs(long int a);

/* ========== 恢复警告 ========== */
#pragma GCC diagnostic pop

/* ============================== */

#endif 
