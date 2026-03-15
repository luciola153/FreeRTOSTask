#include "system.h"	  
#include "delay.h"	  

// 使用简单的软件循环延时，避免与 FreeRTOS 的 SysTick 冲突
// 假设系统时钟为 168MHz (STM32F407 默认 PLL 配置)
// 每个循环大约需要 4 个时钟周期
#define CPU_FREQUENCY   168000000UL
#define CYCLES_PER_US   (CPU_FREQUENCY / 1000000UL)

void delay_ms(u32 nms)
{
	HAL_Delay(nms);
}

void HAL_Delay_us(uint32_t us)
{       
    // 使用 DWT 计数器实现精确微秒延时（如果可用）
    // 或者使用简单的循环延时
    volatile uint32_t delay = us * (CYCLES_PER_US / 4);
    while(delay--);
}

void delay_us(u32 nus)
{
	HAL_Delay_us(nus);
}

