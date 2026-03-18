#include "cmsis_os2.h"
#include "system.h"
#include "LEDTask.h"

void StartLEDTask(void *argument) {
    for (;;) {
        LEDMessage* message;
        osMessageQueueGet(LEDQueueHandle, &message, 0, osWaitForever);
        switch (message -> color) {
            case LEDColor_Up:
                PCout(13) = message -> state;
                break;
            case LEDColor_Down:
                PEout(1) = message -> state;
                break;
        }
        vPortFree(message);
    }
}
