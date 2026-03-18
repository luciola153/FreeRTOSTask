
#include "cmsis_os2.h"
#include "system.h"
#include "OLEDTask.h"
#include "OLED.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

static void OLED_FormatAngle(char *out, char axis, int16_t value_tenths)
{
    uint16_t abs_val;
    uint16_t int_part;
    uint8_t frac_part;
    char digits[6];
    uint8_t len = 0;
    uint8_t i;
    char *p = out;

    *p++ = axis;
    *p++ = ':';

    if (value_tenths < 0) {
        *p++ = '-';
        abs_val = (uint16_t)(-value_tenths);
    } else {
        abs_val = (uint16_t)value_tenths;
    }

    int_part = abs_val / 10U;
    frac_part = (uint8_t)(abs_val % 10U);

    if (int_part == 0U) {
        *p++ = '0';
    } else {
        while (int_part > 0U && len < sizeof(digits)) {
            digits[len++] = (char)('0' + (int_part % 10U));
            int_part /= 10U;
        }
        for (i = 0; i < len; i++) {
            *p++ = digits[len - 1U - i];
        }
    }

    *p++ = '.';
    *p++ = (char)('0' + frac_part);
    *p = '\0';
}

void StartOLEDTask(void *argument) {
    (void)argument;
    // 初始化 OLED
    OLED_Init();
    OLED_Clear();
    OLED_Update();
    
    uint32_t runTime = 0;     // 运行时间计数器
    char xyzBuffer[20];

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
                uint8_t clear_h = (message->fontSize == OLED_8X16) ? 16 : 8;
                OLED_ClearArea(message->x, message->y, 128, clear_h);
                OLED_ShowString(message->x, message->y, message->text, message->fontSize);
                OLED_Update();
                
                // 释放动态分配的内存：先释放文本，再释放结构体
                vPortFree(message->text);  // 释放文本缓冲区
                vPortFree(message);        // 释放消息结构体
            }
        }
        
        // 方式 2: 直接在此处显示变量（适合快速测试）
        runTime++;
        if (runTime >= 10) {  // 10 * 10ms = 100ms，刷新角度显示
            runTime = 0;

            int16_t roll10 = (int16_t)(g_tly_roll_deg * 10.0f);
            int16_t pitch10 = (int16_t)(g_tly_pitch_deg * 10.0f);
            int16_t yaw10 = (int16_t)(g_tly_yaw_deg * 10.0f);

            OLED_ClearArea(0, 0, 128, 16);
            OLED_FormatAngle(xyzBuffer, 'X', roll10);
            OLED_ShowString(0, 0, xyzBuffer, OLED_8X16);

            OLED_ClearArea(0, 16, 128, 16);
            OLED_FormatAngle(xyzBuffer, 'Y', pitch10);
            OLED_ShowString(0, 16, xyzBuffer, OLED_8X16);

            OLED_ClearArea(0, 32, 128, 16);
            OLED_FormatAngle(xyzBuffer, 'Z', yaw10);
            OLED_ShowString(0, 32, xyzBuffer, OLED_8X16);

            OLED_Update();
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);  // 延时 10ms，避免占用过多 CPU
    }
}
