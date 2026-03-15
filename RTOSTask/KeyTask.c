
#include "cmsis_os2.h"
#include "system.h"
#include "OLEDTask.h"  // 包含 OLED 消息结构定义


#define IS_KEY_PRESSED()  (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_0) == GPIO_PIN_RESET)
#define KEY_CHECK_INTERVAL 10
#define KEY_DEBOUNCE_TIME  30
#define KEY_DEBOUNCE_COUNT (KEY_DEBOUNCE_TIME / KEY_CHECK_INTERVAL)

uint8_t isKey1Clicked(void) {
    if (IS_KEY_PRESSED())
    {
     osDelay(KEY_DEBOUNCE_COUNT);
        if (IS_KEY_PRESSED())
        {
            return 1;
        }
    }
    return 0;
}



void StartKeyTask(void *argument) {
    LEDState state = LEDState_Off;
    char message1[50] = "获取到信号量";
    uint8_t keyPressCount = 0;  // 按键次数计数器
    
    for (;;) {
        osSemaphoreAcquire(KeySemaphoreHandle, osWaitForever);
        HAL_UART_Transmit(&huart1, (uint8_t *)message1, strlen(message1), 1000);
        
        if (isKey1Clicked()) {
            state = !state;
            
            // 发送 LED 消息
            LEDMessage* message = pvPortMalloc(sizeof(LEDMessage));
            message -> color = LEDColor_Up;
            message -> state = state;
            osMessageQueuePut(LEDQueueHandle, &message, 0, osWaitForever);
            
            // 示例：通过 OLED 队列显示按键次数
            keyPressCount++;
            OLEDMessage* oledMsg = pvPortMalloc(sizeof(OLEDMessage));
            oledMsg->x = 0;
            oledMsg->y = 16;
            oledMsg->fontSize = OLED_6X8;
            
            // 动态分配文本内存（每个消息独立）
            char* textBuffer = pvPortMalloc(30);
            sprintf(textBuffer, "Key: %d", keyPressCount);
            oledMsg->text = textBuffer;  // 指向独立的缓冲区
            
            osMessageQueuePut(OLEDQueueHandle, &oledMsg, 0, osWaitForever);
        }
        osDelay(10);
    }
}

