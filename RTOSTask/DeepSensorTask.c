#include "system.h"
#include "DeepSensorTask.h"
#include "uart_execute.h"
#include "i2c.h"
#include "task.h"

typedef enum
{
    DS_STATE_IDLE = 0,
    DS_STATE_WAIT_D2,
    DS_STATE_WAIT_D1
} DeepSensorState_t;

static uint16_t s_prom_c[7] = {0}; /* index 1..6 */
static uint32_t s_d1_raw = 0;
static uint32_t s_d2_raw = 0;
static uint8_t s_inited = 0;
static uint8_t s_new_data_ready = 0;
static uint8_t s_comm_fail_count = 0;
static DeepSensorState_t s_state = DS_STATE_IDLE;
static uint32_t s_deadline_tick_10ms = 0;
static uint32_t s_last_cycle_tick_10ms = 0;
static uint32_t s_tick_10ms = 0;

static void ds_mark_comm_fail(void)
{
    if (s_comm_fail_count < 0xFFu) {
        s_comm_fail_count++;
    }
    if (s_comm_fail_count >= 3u) {
        s_inited = 0;
        g_ds_ready = 0;
        ms5837_data_valid = 0U;
        s_state = DS_STATE_IDLE;
    }
}

static HAL_StatusTypeDef ds_write_cmd(uint8_t cmd)
{
    return HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(MS5837_ADDR_7BIT << 1), &cmd, 1, 50);
}

static HAL_StatusTypeDef ds_read_bytes(uint8_t *buf, uint16_t len)
{
    return HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(MS5837_ADDR_7BIT << 1), buf, len, 50);
}

static HAL_StatusTypeDef ds_read_prom_coeff(uint8_t idx, uint16_t *out_coeff)
{
    uint8_t cmd = (uint8_t)(MS5837_CMD_PROM_READ_BASE + (idx << 1));
    uint8_t buf[2] = {0};
    HAL_StatusTypeDef ret = ds_write_cmd(cmd);
    if (ret != HAL_OK) {
        return ret;
    }
    ret = ds_read_bytes(buf, 2);
    if (ret != HAL_OK) {
        return ret;
    }
    *out_coeff = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
    return HAL_OK;
}

static HAL_StatusTypeDef ds_read_adc24(uint32_t *out_adc)
{
    uint8_t cmd = MS5837_CMD_ADC_READ;
    uint8_t buf[3] = {0};
    HAL_StatusTypeDef ret = ds_write_cmd(cmd);
    if (ret != HAL_OK) {
        return ret;
    }
    ret = ds_read_bytes(buf, 3);
    if (ret != HAL_OK) {
        return ret;
    }
    *out_adc = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    return HAL_OK;
}

static void ds_calculate(void)
{
    int64_t dT;
    int64_t TEMP;
    int64_t OFF;
    int64_t SENS;
    int64_t Ti = 0;
    int64_t OFFi = 0;
    int64_t SENSi = 0;

    dT = (int64_t)s_d2_raw - ((int64_t)s_prom_c[5] << 8);
    TEMP = 2000 + ((dT * s_prom_c[6]) / 8388608LL);

    OFF = ((int64_t)s_prom_c[2] << 17) + ((dT * s_prom_c[4]) >> 6);
    SENS = ((int64_t)s_prom_c[1] << 16) + ((dT * s_prom_c[3]) >> 7);

    if (TEMP < 2000) {
        int64_t t = TEMP - 2000;
        Ti = (11 * dT * dT) >> 35;
        OFFi = (31 * t * t) >> 3;
        SENSi = (63 * t * t) >> 5;

        if (TEMP < -1500) {
            int64_t t2 = TEMP + 1500;
            OFFi += 17 * t2 * t2;
            SENSi += 9 * t2 * t2;
        }
    } else {
        int64_t t = TEMP - 2000;
        Ti = (3 * dT * dT) >> 33;
        OFFi = (3 * t * t) >> 1;
        SENSi = (5 * t * t) >> 3;
    }

    TEMP -= Ti;
    OFF -= OFFi;
    SENS -= SENSi;

    {
        int64_t p_0p01mbar = ((((int64_t)s_d1_raw * SENS) >> 21) - OFF) >> 15;
        g_ds_press_centi_mbar = (int32_t)p_0p01mbar;
    }

    g_ds_temp_centi_c = (int32_t)TEMP;
    ms5837_pressure_mbar = (float)g_ds_press_centi_mbar / 100.0f;
    ms5837_temperature_c = (float)g_ds_temp_centi_c / 100.0f;
    ms5837_depth_m = (ms5837_pressure_mbar - 1013.25f) * 100.0f / (1029.0f * 9.80665f);
    ms5837_data_valid = 1U;

    /* Keep fixed-point globals for existing modules. */
    g_ds_depth_mm = (int32_t)(ms5837_depth_m * 1000.0f);
}

HAL_StatusTypeDef DeepSensor_Init(void)
{
    HAL_StatusTypeDef ret;
    uint8_t i;

    ret = ds_write_cmd(MS5837_CMD_RESET);
    if (ret != HAL_OK) {
        s_inited = 0;
        g_ds_ready = 0;
        ms5837_data_valid = 0U;
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(3));

    for (i = 1; i <= 6; i++) {
        ret = ds_read_prom_coeff(i, &s_prom_c[i]);
        if (ret != HAL_OK) {
            s_inited = 0;
            g_ds_ready = 0;
            ms5837_data_valid = 0U;
            return ret;
        }
    }

    s_state = DS_STATE_IDLE;
    s_tick_10ms = 0;
    s_deadline_tick_10ms = 0;
    s_last_cycle_tick_10ms = 0;
    s_new_data_ready = 0;
    s_comm_fail_count = 0;
    s_inited = 1;
    g_ds_ready = 1;
    ms5837_data_valid = 0U;
    g_ds_i2c_addr = (uint16_t)(MS5837_ADDR_7BIT << 1);
    for (i = 1; i <= 6; i++) {
        g_ds_prom[i] = s_prom_c[i];
    }
    return HAL_OK;
}

void DeepSensor_Task10ms(void)
{
    HAL_StatusTypeDef ret;

    if (!s_inited) {
        return;
    }

    switch (s_state) {
        case DS_STATE_IDLE:
            if ((uint32_t)(s_tick_10ms - s_last_cycle_tick_10ms) >= 10u) {
                ret = ds_write_cmd(MS5837_CMD_CONV_D2_OSR4096);
                if (ret == HAL_OK) {
                    s_deadline_tick_10ms = s_tick_10ms + 1u;
                    s_state = DS_STATE_WAIT_D2;
                } else {
                    ds_mark_comm_fail();
                }
            }
            break;

        case DS_STATE_WAIT_D2:
            if ((int32_t)(s_tick_10ms - s_deadline_tick_10ms) >= 0) {
                ret = ds_read_adc24(&s_d2_raw);
                if (ret == HAL_OK) {
                    ret = ds_write_cmd(MS5837_CMD_CONV_D1_OSR4096);
                    if (ret == HAL_OK) {
                        s_deadline_tick_10ms = s_tick_10ms + 1u;
                        s_state = DS_STATE_WAIT_D1;
                    } else {
                        ds_mark_comm_fail();
                        s_state = DS_STATE_IDLE;
                    }
                } else {
                    ds_mark_comm_fail();
                    s_state = DS_STATE_IDLE;
                }
            }
            break;

        case DS_STATE_WAIT_D1:
            if ((int32_t)(s_tick_10ms - s_deadline_tick_10ms) >= 0) {
                ret = ds_read_adc24(&s_d1_raw);
                if (ret == HAL_OK) {
                    ds_calculate();
                    s_new_data_ready = 1;
                    s_comm_fail_count = 0;
                } else {
                    ds_mark_comm_fail();
                }
                s_last_cycle_tick_10ms = s_tick_10ms;
                s_state = DS_STATE_IDLE;
            }
            break;

        default:
            s_state = DS_STATE_IDLE;
            break;
    }
}

uint8_t DeepSensor_IsReady(void)
{
    return s_inited;
}

void StartDeepSensorTask(void *argument)
{
    (void)argument;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(10);
    uint8_t init_log_div = 0;
    uint8_t comm_lost_log = 0;

    UART1_SendString("DeepSensorTask start\r\n");
    vTaskDelay(pdMS_TO_TICKS(200));

    if (DeepSensor_Init() == HAL_OK) {
        UART1_SendString("MS5837 init ok\r\n");
    } else {
        UART1_SendString("MS5837 init failed, retrying...\r\n");
    }

    for (;;) {
        if (!DeepSensor_IsReady()) {
            init_log_div++;
            if (DeepSensor_Init() == HAL_OK) {
                UART1_SendString("MS5837 init ok\r\n");
                init_log_div = 0;
                comm_lost_log = 0;
            } else if (init_log_div >= 50U) {
                init_log_div = 0;
                UART1_SendString("MS5837 init failed, retrying...\r\n");
            }
        } else {
            s_tick_10ms++;
            DeepSensor_Task10ms();
            if (!DeepSensor_IsReady() && comm_lost_log == 0U) {
                comm_lost_log = 1U;
                UART1_SendString("MS5837 comm lost, reinit...\r\n");
            }
            if (s_new_data_ready) {
                s_new_data_ready = 0;
                UART1_SendDeepSensorData(g_ds_temp_centi_c, g_ds_press_centi_mbar, g_ds_depth_mm);
            }
        }
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}
