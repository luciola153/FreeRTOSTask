#include "cmsis_os2.h"
#include "system.h"
#include "TlyTask.h"

#define TLY_DEVICE_ADDR            0x50
#define TLY_FUNC_CODE              0x03
#define TLY_EXPECT_MIN_FRAME_LEN   35

/* According to your data layout example:
 * 00 A3 FF CB 9B 30 -> RollH RollL PitchH PitchL YawH YawL
 */
#define TLY_ROLL_OFFSET_FROM_FRAME   29
#define TLY_PITCH_OFFSET_FROM_FRAME  31
#define TLY_YAW_OFFSET_FROM_FRAME    33

static const uint8_t g_tly_modbus_cmd[] = {0x50, 0x03, 0x00, 0x30, 0x00, 0x30, 0x48, 0x50};

volatile float g_tly_roll_deg = 0.0f;
volatile float g_tly_pitch_deg = 0.0f;
volatile float g_tly_yaw_deg = 0.0f;

static void TLY_RS485_SetTxMode(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
}

static void TLY_RS485_SetRxMode(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
}

static int16_t TLY_ReadInt16BE(const uint8_t *buf, uint16_t offset)
{
    return (int16_t)(((uint16_t)buf[offset] << 8) | buf[offset + 1]);
}

static float TLY_RawToDegree(int16_t raw)
{
    float deg = ((float)raw / 32768.0f) * 180.0f;
    if (deg > 180.0f) {
        deg -= 360.0f;
    }
    return deg;
}

static uint8_t TLY_ParseAngles(const TlyMessage *msg, float *roll, float *pitch, float *yaw)
{
    if (msg == NULL || msg->raw_len < TLY_EXPECT_MIN_FRAME_LEN) {
        return 0;
    }

    if (msg->raw[0] != TLY_DEVICE_ADDR || msg->raw[1] != TLY_FUNC_CODE) {
        return 0;
    }

    if ((uint16_t)(msg->raw[2] + 5U) != msg->raw_len) {
        return 0;
    }

    if ((TLY_YAW_OFFSET_FROM_FRAME + 1U) >= msg->raw_len) {
        return 0;
    }

    int16_t roll_raw = TLY_ReadInt16BE(msg->raw, TLY_ROLL_OFFSET_FROM_FRAME);
    int16_t pitch_raw = TLY_ReadInt16BE(msg->raw, TLY_PITCH_OFFSET_FROM_FRAME);
    int16_t yaw_raw = TLY_ReadInt16BE(msg->raw, TLY_YAW_OFFSET_FROM_FRAME);

    *roll = TLY_RawToDegree(roll_raw);
    *pitch = TLY_RawToDegree(pitch_raw);
    *yaw = TLY_RawToDegree(yaw_raw);
    return 1;
}

void StartTlyTask(void *argument)
{
    (void)argument;
    TlyMessage msg;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(20);  // 50Hz polling
    TLY_RS485_SetRxMode();
    xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        /* RS485 half-duplex: DE high before transmit. */
        TLY_RS485_SetTxMode();
        HAL_UART_Transmit(&huart2, (uint8_t *)g_tly_modbus_cmd, sizeof(g_tly_modbus_cmd), 100);
        while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) == RESET) {
        }
        /* Switch back to receive immediately after frame sent. */
        TLY_RS485_SetRxMode();

        if (osMessageQueueGet(TlyQueueHandle, &msg, NULL, 30) == osOK) {
            float roll, pitch, yaw;
            if (TLY_ParseAngles(&msg, &roll, &pitch, &yaw)) {
                g_tly_roll_deg = roll;
                g_tly_pitch_deg = pitch;
                g_tly_yaw_deg = yaw;
            }
        }

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}
