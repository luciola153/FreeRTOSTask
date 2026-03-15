#include "UART_NVIC.h"
#include "usart.h"
#include "cmsis_os2.h"

// 全局变量：每个串口的状态机
Serial_RxPacket RXdata[UART_NUM_MAX];

/**
 * @brief 获取串口对应的帧头
 */
static uint8_t UART_GetFrameHead(uint8_t uart_id)
{
    switch(uart_id)
    {
        case 1: return SENSOR1_HEAD;
        case 2: return SENSOR2_HEAD;
        case 3: return SENSOR3_HEAD;
        case 4: return SENSOR4_HEAD;
        case 5: return SENSOR5_HEAD;  // 返回 0，不使用帧头检测
        case 6: return SENSOR6_HEAD;
        default: return 0;
    }
}

/**
 * @brief 获取串口对应的帧尾
 */
static uint8_t UART_GetFrameTail(uint8_t uart_id)
{
    switch(uart_id)
    {
        case 1: return SENSOR1_TAIL;
        case 2: return SENSOR2_TAIL;
        case 3: return SENSOR3_TAIL;
        case 4: return SENSOR4_TAIL;
        case 5: return SENSOR5_TAIL;  // 只有帧尾
        case 6: return SENSOR6_TAIL;  // 返回 0，不使用帧尾检测
        default: return 0;
    }
}

/**
 * @brief 重置指定串口的状态机
 */
void UART_ResetStateMachine(uint8_t uart_id)
{
    if(uart_id >= UART_NUM_MAX) return;

    taskENTER_CRITICAL();
    memset(RXdata[uart_id].Rx, 0, MAX_FRAME_LENGTH);
    RXdata[uart_id].number = 0;
    RXdata[uart_id].number_final = 0;
    RXdata[uart_id].rx_final_flag = 0;
    RXdata[uart_id].last_byte_time = 0;
    taskEXIT_CRITICAL();
}

/**
 * @brief 初始化所有串口
 */
void UART_Init(void)
{
    // 初始化所有串口的状态机
    for(int i = 0; i < UART_NUM_MAX; i++)
    {
        UART_ResetStateMachine(i);
    }

    // 启动串口中断接收（每次接收一个字节）
    HAL_UART_Receive_IT(&huart1, &RX_Data_1, 1);
    HAL_UART_Receive_IT(&huart2, &RX_Data_2, 1);
    HAL_UART_Receive_IT(&huart3, &RX_Data_3, 1);
    HAL_UART_Receive_IT(&huart4, &RX_Data_4, 1);
    HAL_UART_Receive_IT(&huart5, &RX_Data_5, 1);  // 串口 5
    HAL_UART_Receive_IT(&huart6, &RX_Data_6, 1);  // 串口 6
}

/**
 * @brief 处理接收到的字节（核心状态机）
 * @param uart_id 串口号（1~6）
 * @param data    接收到的字节
 */
void UART_ProcessByte(uint8_t uart_id, uint8_t data)
{
   if(uart_id >= UART_NUM_MAX) return;

    uint8_t frame_head = UART_GetFrameHead(uart_id);
    uint8_t frame_tail = UART_GetFrameTail(uart_id);

    // 更新最后接收时间（使用 FreeRTOS tick，需确保 configTICK_RATE_HZ = 1000）
    RXdata[uart_id].last_byte_time = osKernelGetTickCount();

    // 如果已经有一帧完成但还未处理，暂时不接收新数据（直到任务处理完）
    if(RXdata[uart_id].rx_final_flag == 1)
    {
        return;
    }

    // 检查是否超时（如果正在接收中但间隔太久，重置状态机）
    if(RXdata[uart_id].number > 0)
    {
        uint32_t now = osKernelGetTickCount();
        if((now - RXdata[uart_id].last_byte_time) > FRAME_TIMEOUT_MS)
        {
            UART_ResetStateMachine(uart_id);
            // 重置后重新处理当前字节？理论上应该丢弃，但为了简单，直接返回
            // 注意：重置后 number=0，所以下面的状态机会重新开始
        }
    }

    // 特殊处理：串口 5（只有帧尾，无帧头）
   if(uart_id == 5)
    {
        // 直接接收数据，等待帧尾
       if(RXdata[uart_id].number >= MAX_FRAME_LENGTH - 1)
        {
            UART_ResetStateMachine(uart_id);
            return;
        }
        
        RXdata[uart_id].Rx[RXdata[uart_id].number] = data;
        RXdata[uart_id].number++;
        
        // 检测帧尾
       if(data == frame_tail)
        {
            // 检查最小帧长度（至少 1 个数据 + 帧尾 = 2 字节）
           if(RXdata[uart_id].number >= 2)
            {
                RXdata[uart_id].number_final = RXdata[uart_id].number;
                RXdata[uart_id].number = 0;
                RXdata[uart_id].rx_final_flag = 1;
            }
            else
            {
                // 帧太短，重置
                UART_ResetStateMachine(uart_id);
            }
        }
        return;
    }
    
    // 特殊处理：串口 6（只有帧头 + 固定长度，无帧尾）
   if(uart_id == 6)
    {
       if(RXdata[uart_id].number == 0)  // 等待帧头
        {
           if(data == frame_head)
            {
                RXdata[uart_id].Rx[0] = data;
                RXdata[uart_id].number = 1;
            }
            // 否则丢弃
        }
        else  // 已经收到帧头，继续接收固定长度的数据
        {
           if(RXdata[uart_id].number >= SENSOR6_FIXED_LENGTH - 1)
            {
                // 达到固定长度，完成一帧
                RXdata[uart_id].number_final = RXdata[uart_id].number;
                RXdata[uart_id].number = 0;
                RXdata[uart_id].rx_final_flag = 1;
            }
            else
            {
                RXdata[uart_id].Rx[RXdata[uart_id].number] = data;
                RXdata[uart_id].number++;
            }
        }
        return;
    }

    // 状态机
    if(RXdata[uart_id].number == 0)  // 等待帧头
    {
        if(data == frame_head)
        {
            RXdata[uart_id].Rx[0] = data;
            RXdata[uart_id].number = 1;
        }
        // 否则丢弃
    }
    else if(data != frame_tail)  // 接收数据
    {
        // 防止缓冲区溢出
        if(RXdata[uart_id].number >= MAX_FRAME_LENGTH - 1)
        {
            UART_ResetStateMachine(uart_id);
            return;
        }

        RXdata[uart_id].Rx[RXdata[uart_id].number] = data;
        RXdata[uart_id].number++;
    }
    else if(data == frame_tail)  // 收到帧尾
    {
        // 检查最小帧长度（至少帧头+1个数据+帧尾 = 3字节）
        if(RXdata[uart_id].number >= 2)
        {
            RXdata[uart_id].Rx[RXdata[uart_id].number] = data;
            RXdata[uart_id].number_final = RXdata[uart_id].number + 1;
            RXdata[uart_id].number = 0;
            RXdata[uart_id].rx_final_flag = 1;

            // 不需要再发送通知，任务循环会检测到标志并处理
        }
        else
        {
            // 帧太短，重置
            UART_ResetStateMachine(uart_id);
        }
    }
}

/**
 * @brief 串口接收完成回调（在中断中调用）
 *        将接收到的字节打包成 (uart_id<<8)|data 发送到队列
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t data;
    uint8_t uart_id = 0;

    // 判断是哪个串口并获取数据，同时重新启动中断接收
    if(huart->Instance == USART1)
    {
        data = RX_Data_1;
        uart_id = 1;
        HAL_UART_Receive_IT(&huart1, &RX_Data_1, 1);
    }
    else if(huart->Instance == USART2)
    {
        data = RX_Data_2;
        uart_id = 2;
        HAL_UART_Receive_IT(&huart2, &RX_Data_2, 1);
    }
    else if(huart->Instance == USART3)
    {
        data = RX_Data_3;
        uart_id = 3;
        HAL_UART_Receive_IT(&huart3, &RX_Data_3, 1);
    }
    else if(huart->Instance == UART4)
    {
        data = RX_Data_4;
        uart_id = 4;
        HAL_UART_Receive_IT(&huart4, &RX_Data_4, 1);
    }
   else if(huart->Instance == UART5)  // 串口 5
    {
        data = RX_Data_5;
        uart_id = 5;
        HAL_UART_Receive_IT(&huart5, &RX_Data_5, 1);
    }
   else if(huart->Instance == USART6)  // 串口 6
    {
        data = RX_Data_6;
        uart_id = 6;
        HAL_UART_Receive_IT(&huart6, &RX_Data_6, 1);
    }
    else
    {
        return;
    }

    // 将数据打包发送到队列（队列元素大小为 sizeof(uint16_t)）
    uint16_t packet = (uart_id << 8) | data;
    osMessageQueuePut(CommandQueueHandle, &packet, 0, 0);
}

/**
 * @brief 命令处理任务（接收队列数据，组帧并处理）
 */
void StartCommandTask(void *argument)
{
    // 初始化所有串口
    UART_Init();

    uint16_t packet;
    uint8_t uart_id, data;
    uint8_t temp_frame[MAX_FRAME_LENGTH];
    uint16_t temp_len;

    for(;;)
    {
        // 等待队列数据（无限等待）
        if(osMessageQueueGet(CommandQueueHandle, &packet, 0, osWaitForever) == osOK)
        {
            // 解析包：高8位是串口号，低8位是数据
            uart_id = (packet >> 8) & 0xFF;
            data = packet & 0xFF;

            // 将字节送入对应串口的状态机
            UART_ProcessByte(uart_id, data);

            // 检查是否有完成的帧
            if(RXdata[uart_id].rx_final_flag == 1)
            {
                // 临界区保护，复制数据并清除标志
                taskENTER_CRITICAL();
                temp_len = RXdata[uart_id].number_final;
                memcpy(temp_frame, RXdata[uart_id].Rx, temp_len);
                RXdata[uart_id].rx_final_flag = 0;  // 允许接收新帧
                taskEXIT_CRITICAL();

                // 根据串口号处理不同的传感器数据
                switch(uart_id)
                {
                    case 1:  // 串口 1 - LED 控制 + 数据回显
                      if(temp_len >= 3)
                        {
                            // 发送 LED 控制消息到队列
                            LEDMessage *message = pvPortMalloc(sizeof(LEDMessage));
                          if(message != NULL)
                            {
                                message->color = temp_frame[1] - 1;
                                message->state = temp_frame[2];
                              if(osMessageQueuePut(LEDQueueHandle, &message, 0, 100) != osOK)
                                {
                                    vPortFree(message);
                                }
                            }
                            
                            // 将原始数据通过串口 1 回显
                            UART_SendData(&huart1, temp_frame, temp_len);
                        }
                        break;

                    case 2:  // 串口 2 - 数据回显
                        UART_SendData(&huart2, temp_frame, temp_len);
                        break;

                    case 3:  // 串口 3 - 数据回显
                        UART_SendData(&huart3, temp_frame, temp_len);
                        break;

                    case 4:  // 串口 4 - 数据回显
                        UART_SendData(&huart4, temp_frame, temp_len);
                        break;

                    case 5:  // 串口 5 - 数据回显（只有帧尾）
                        UART_SendData(&huart5, temp_frame, temp_len);
                        break;

                    case 6:  // 串口 6 - 数据回显（固定长度）
                        UART_SendData(&huart6, temp_frame, temp_len);
                        break;

                    default:
                        break;
                }

                // 可选：将原始数据回显，用于调试
                // 已在各 case 中单独处理，此处无需重复发送
            }
        }
    }
}

/**
 * @brief 获取串口句柄的辅助函数
 */
UART_HandleTypeDef* GetUartHandle(uint8_t uart_id)
{
  switch(uart_id)
    {
        case 1: return &huart1;
        case 2: return &huart2;
        case 3: return &huart3;
        case 4: return &huart4;
        case 5: return &huart5;
        case 6: return &huart6;
        default: return NULL;
    }
}

/**
 * @brief 串口发送函数（阻塞方式，简单可靠）
 */
void UART_SendData(UART_HandleTypeDef *huart, uint8_t *data, uint16_t len)
{
    if(huart != NULL && data != NULL && len > 0)
    {
        HAL_UART_Transmit(huart, data, len, 100);
    }
}