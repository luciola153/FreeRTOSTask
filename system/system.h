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
#include "TlyTask.h"
#include "DeepSensorTask.h"

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
extern osMessageQueueId_t TlyQueueHandle;
extern osMessageQueueId_t DeepSensorQueueHandle;
extern volatile float g_tly_roll_deg;
extern volatile float g_tly_pitch_deg;
extern volatile float g_tly_yaw_deg;
extern volatile uint8_t g_ds_ready;
extern volatile int32_t g_ds_temp_centi_c;
extern volatile int32_t g_ds_press_centi_mbar;
extern volatile int32_t g_ds_depth_mm;
extern volatile uint16_t g_ds_i2c_addr;
extern uint16_t g_ds_prom[8];
extern volatile float ms5837_pressure_mbar;
extern volatile float ms5837_temperature_c;
extern volatile float ms5837_depth_m;
extern volatile uint8_t ms5837_data_valid;

#define configUSE_COUNTING_SEMAPHORES 1           //使用计数型信号量必备宏定义
#define configUSE_MUTEXES 1                      //使用互斥锁必备宏定义


u32 myabs(long int a);

/* ========== 恢复警告 ========== */
#pragma GCC diagnostic pop

/* ============================== */

#endif 
