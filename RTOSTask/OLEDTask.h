//
// Created by 32455 on 2026/2/12.
//

#ifndef LED01_OLEDTASK_H
#define LED01_OLEDTASK_H

#include <stdint.h>

// OLED 消息结构体
typedef struct {
    int16_t x;          // X 坐标
    int16_t y;          // Y 坐标
    char* text;         // 显示的文本
    uint8_t fontSize;   // 字体大小
} OLEDMessage;

void StartOLEDTask(void *argument);

#endif //LED01_OLEDTASK_H
