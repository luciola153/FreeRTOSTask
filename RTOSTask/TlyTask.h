#ifndef LED01_TLYTASK_H
#define LED01_TLYTASK_H

#include <stdint.h>

#define TLY_RAW_FRAME_MAX_LEN 128

typedef struct {
    uint16_t raw_len;
    uint8_t raw[TLY_RAW_FRAME_MAX_LEN];
} TlyMessage;

void StartTlyTask(void *argument);

#endif
