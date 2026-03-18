#ifndef UART_NVIC_H
#define UART_NVIC_H

#include "system.h"
#include "cmsis_os2.h"

// 串口数量配置（根据实际使用的串口数量调整）
#define UART_NUM_MAX  7  // 索引 0 不用，1~6对应串口 1~6

// 每个传感器的帧头帧尾定义（可自定义）
#define SENSOR1_HEAD    0x6C
#define SENSOR1_TAIL    0x7F
#define SENSOR2_HEAD    0xAA
#define SENSOR2_TAIL    0xBB
#define SENSOR3_HEAD    0x12
#define SENSOR3_TAIL    0x34
#define SENSOR4_HEAD    0xEF
#define SENSOR4_TAIL    0xCD
#define SENSOR5_HEAD    0x00  // 串口 5 无帧头（不使用）
#define SENSOR5_TAIL    0x88  // 串口 5 只有帧尾
#define SENSOR6_HEAD    0x99  // 串口 6 只有帧头
#define SENSOR6_TAIL    0x00  // 串口 6 无帧尾（不使用）

// 串口 6 固定帧长度（包含帧头）
#define SENSOR6_FIXED_LENGTH  10  // 例如：1 字节帧头 + 9 字节数据 = 10 字节

// 最大帧长度（防止缓冲区溢出）
#define MAX_FRAME_LENGTH 128

// 超时时间（ms）- 用于帧超时检测，建议根据实际波特率调整
#define FRAME_TIMEOUT_MS 50

// 每个串口的数据接收状态结构体
typedef struct {
    uint8_t Rx[MAX_FRAME_LENGTH];      // 接收缓冲区
    uint8_t number;                     // 当前接收位置（已接收字节数，不含帧头？实际是索引）
    uint8_t number_final;               // 完整帧长度（含帧头帧尾）
    uint8_t rx_final_flag;               // 帧完成标志
    uint32_t last_byte_time;             // 最后一个字节接收时间（系统 tick，单位 ms）
} Serial_RxPacket;

// 全局变量声明
extern Serial_RxPacket RXdata[UART_NUM_MAX];

// 函数声明
void UART_Init(void);
void UART_ProcessByte(uint8_t uart_id, uint8_t data);
void UART_ResetStateMachine(uint8_t uart_id);
UART_HandleTypeDef* GetUartHandle(uint8_t uart_id);
void UART_SendData(UART_HandleTypeDef *huart, uint8_t *data, uint16_t len);

#endif