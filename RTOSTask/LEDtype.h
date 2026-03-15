//
// Created by 32455 on 2026/2/12.
//

#ifndef LED01_LEDTYPE_H
#define LED01_LEDTYPE_H

typedef enum {
    LEDState_Off = 0,
    LEDState_On = 1,
} LEDState;

typedef enum {
    LEDColor_Up = 0,
    LEDColor_Down = 1,
} LEDColor;

typedef struct {
    LEDColor color;
    LEDState state;
} LEDMessage;

#endif //LED01_LEDTYPE_H