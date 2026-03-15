
#include "cmsis_os2.h"
#include "system.h"
#include "OLED.h"
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

void StartOLEDTask(void *argument) {
    // 初始化 OLED
    OLED_Init();
    
    // 显示欢迎信息
    OLED_ShowString(0, 0, "Water System", OLED_8X16);
    OLED_Update();
    
    uint32_t runTime = 0;  // 运行时间计数器
    char buffer[30];       // 字符串缓冲区
    static uint32_t counter = 0;  // 用于显示的计数器

    // 使用 vTaskDelayUntil 实现精确的 10ms 定时
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(10);  // 每 10ms 唤醒一次
    xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        // 方式 1: 从队列获取消息（推荐方式）
        // 其他任务可以通过队列发送显示命令
        OLEDMessage* message;
        if (osMessageQueueGet(OLEDQueueHandle, &message, NULL, 0) == osOK) {
            if (message != NULL) {
                // 在指定位置显示消息
                OLED_ClearArea(message->x, message->y, 128, 8);  // 清除对应行
                OLED_ShowString(message->x, message->y, message->text, message->fontSize);
                OLED_Update();
                
                // 释放动态分配的内存：先释放文本，再释放结构体
                vPortFree(message->text);  // 释放文本缓冲区
                vPortFree(message);        // 释放消息结构体
            }
        }
        
        // 方式 2: 直接在此处显示变量（适合快速测试）
        // 每 100ms 累加一次，每 1000ms (1 秒) 更新一次显示
        runTime++;
        if (runTime >= 100) {  // 100 * 10ms = 1000ms = 1 秒
            runTime = 0;
            counter++;  // 计数器加 1
            
            // 在 x=0, y=16 位置显示计数器值（第二行，更明显）
            sprintf(buffer, "Num:%d", counter);
            OLED_ClearArea(0, 32, 60, 8);  // 先清除该区域
            OLED_ShowString(0, 32, buffer, OLED_6X8);
            OLED_Update();
        }
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);  // 延时 10ms，避免占用过多 CPU
    }
}
