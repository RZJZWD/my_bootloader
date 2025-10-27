// Header:
// File Name: uart_driver.h
// Author: DJS
// Date: 2025年7月5日

#ifndef _MY_UART_H_
#define _MY_UART_H_

#define USE_CUBEMX_UART 1 // 默认使用cubemx初始化uart

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "string.h"
#include "usart.h"

#define RX_BUFFER_SIZE 32

// 前置声明 防止函数指针参数类型未定义
typedef struct UART_Device_t UART_Device_t;

// 回调函数类型定义
typedef void (*UART_RxCallbackPointer)(void);
typedef void (*UART_ErrorCallbackPointer)(void);

// 函数指针类型定义
typedef void (*UART_TransmitFunc)(UART_Device_t *, uint8_t *, uint16_t);
typedef void (*UART_ReceiveFunc)(UART_Device_t *, uint8_t *, uint16_t);

// UART设备结构体
struct UART_Device_t {
    // 核心成员
    UART_HandleTypeDef *huart; // HAL库UART句柄

    // 接收相关
    uint8_t rx_byte;      // 单字节接收缓冲区
    uint16_t rx_buffSize; // 多字节接收缓冲区大小

    // 函数指针
    UART_RxCallbackPointer rx_callback;       // 接收回调函数指针
    UART_ErrorCallbackPointer error_callback; // 错误回调函数指针
    UART_TransmitFunc transmit;               // 发送函数指针
    UART_ReceiveFunc receive;                 // 接收函数指针
};
// 初始化函数
void UART_InitDev(UART_Device_t *dev, UART_HandleTypeDef *huart);
// 回调函数
void UART_Set_RxCallback(UART_Device_t *dev, UART_RxCallbackPointer rxcallback);
void UART_Set_ErrorCallback(UART_Device_t *dev,
                            UART_ErrorCallbackPointer errorcallback);

// 传输
void UART_Transmit(UART_Device_t *dev, uint8_t *data, uint16_t size);
void UART_Receive_IT(UART_Device_t *dev, uint8_t *data, uint16_t size);

// 中断处理
void UART_userIRQ_Handler(UART_Device_t *dev);

#ifdef __cplusplus
}
#endif

#endif
